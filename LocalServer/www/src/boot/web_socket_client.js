import { assert, readUint8, readUint32, appendUint32, stringFormat, encodeUtf8Bytes, decodeUtf8Bytes, encodeUint32ToBytes,
    decodeUint32FromBytes, utf8ArrayToJson, utf8ArrayToBytes, jsonToUtf8Bytes, jsonToUtf8Array, bytesToUtf8Array, crc32 } from './utils';
import { reactive } from 'vue'

console.log('### in websocket client.');

const CUR_VERSION = 1;

const MT_ERROR = 0,
    MT_GET_PUBLIC_KEY = 1,
    MT_PUBLIC_KEY = 2,
    MT_PUB_KEY_ENC = 3,
    MT_CLIENT_KEY_ENC = 4;

export const STATUS_CONNECTING = 0,
    STATUS_DISCONNECTED = 1,
    STATUS_INITED = 3;

export const TYPE_PUBLIC_KEY = 'PublicKey',
    TYPE_INIT_CONNECTION = 'InitConnection',
    TYPE_PLAYER_REMOTE_CTRL = 'PlayerRemoteCtrl',
    TYPE_PLAYER_NOTIFICATION = 'PlayerNotification',
    TYPE_PLAYER_STATES = 'PlayerStates',
    TYPE_MEDIA_LIB_NOTIFICATION = 'MediaLibNotification',
    TYPE_MEDIA_LIB_ALL = 'MediaLibAll';

let socket = null, g_clientId = 0, g_clientKey = [], g_pubKey;
let wsServer = 'ws://localhost:1213';
let lastConnectTime = getTimeInSeconds();
let handlers = {
    [TYPE_INIT_CONNECTION] : handleInitConnection,
};

export let network = reactive({
    status: STATUS_CONNECTING,
    description: '',
});

function getTimeInSeconds() {
    return new Date().getTime() / 1000;
}

function packDataPacket(type, clientId, data) {
    assert(data instanceof Array || data instanceof Uint8Array);

    let arr = [CUR_VERSION, type];
    appendUint32(arr, clientId);

    let r = new Uint8Array(arr.length + data.length);
    r.set(arr, 0);
    r.set(data, arr.length);
    return r;
}

function parseDataPacket(data) {
    assert(data instanceof ArrayBuffer);
    let packet = new Uint8Array(data);

    packet._offset = 0;
    let version = readUint8(packet);
    if (version != CUR_VERSION) {
        throw 'Invalid version number: ' + version;
    }

    let type = readUint8(packet);
    let clientId = readUint32(packet);
    return {
        type: type,
        clientId: clientId,
        data: packet.slice(packet._offset),
    }
}

function noPadding(input) {
    return input;
}

function randomPadding(length) {
    length = length % 16;
    if (length != 0) {
        return forge.random.getBytesSync(16 - length);
    }

    return '';
}

function getBrowserName() {
    let parser = new UAParser("user-agent");
    let a = [];
    if (parser.device) a.push(parser.device);
    if (parser.os) a.push(parser.os);
    if (parser.browser) a.push(parser.browser);
    return a.join(' ');
}

/**
 * 使用 clientKey 加密数据，前 4 个字节是原始数据的长度.
 * @param {String} data 
 * @returns 加密后的数据（包括了 IV）
 */
function clientKeyEncrypt(data) {
    var iv = forge.random.getBytesSync(16);
    var cipher = forge.cipher.createCipher('AES-CBC', g_clientKey);

    // 转换为 utf-8 bytes
    if (typeof data == 'object') {
        data = JSON.stringify(data);
    }
    data = encodeUtf8Bytes(data);

    // 增加长度 和 padding.
    data = encodeUint32ToBytes(crc32(data)) + encodeUint32ToBytes(data.length) +
        data + randomPadding(data.length + 8);

    cipher.start({iv: iv});
    cipher.update(forge.util.createBuffer(data));
    cipher.finish(noPadding);

    return iv + cipher.output.getBytes();
}

/**
 * 使用 clientKey 解密数据，前 4 个字节是原始数据的长度.
 * @param {String} data 
 * @returns 加密后的数据（包括了 IV）
 */
function clientKeyDecrypt(data) {
    if (data instanceof Array || data instanceof Uint8Array) {
        data = utf8ArrayToBytes(data);
    } else {
        assert(typeof data === 'string');
    }

    let iv = data.slice(0, 16);
    let encrypted = data.slice(16);

    var cipher = forge.cipher.createDecipher('AES-CBC', g_clientKey);
    cipher.start({iv: iv});
    cipher.update(forge.util.createBuffer(encrypted));
    cipher.finish(noPadding);

    let decrypted = cipher.output.getBytes();

    let crc = decodeUint32FromBytes(decrypted); decrypted = decrypted.slice(4);
    let length = decodeUint32FromBytes(decrypted); decrypted = decrypted.slice(4);
    if (length <= decrypted.length) {
        decrypted = decrypted.slice(0, length);
        if (crc == crc32(decrypted)) {
            return decodeUtf8Bytes(decrypted);
        }
    }

    throw 'Failed to decrypt data with client key.';
}

function init() {
    lastConnectTime = getTimeInSeconds();

    network.status = STATUS_CONNECTING;
    network.description = 'Connecting...';

    socket = new WebSocket(wsServer);

    // Connection opened
    socket.addEventListener('open', (event) => {
        let data = { type: TYPE_PUBLIC_KEY };
        let buf = packDataPacket(MT_GET_PUBLIC_KEY, 0, jsonToUtf8Array(data));
        socket.send(buf);
    });

    // Listen for messages
    socket.addEventListener('message', onWsMessage);

    socket.addEventListener('error', (event) => {
        network.status = STATUS_DISCONNECTED;
        network.description = stringFormat('WebSocket connection to server {0} failed. Will try again in 10 seconds.', wsServer);
        setTimeout(init, 10 * 1000);
    });
}

function onWsMessage(event) {
    // console.log('Message from server ', event.data);

    event.data.arrayBuffer()
    .then((data) => {
        let packet = parseDataPacket(data);
        if (packet.type == MT_PUBLIC_KEY) {
            let json = utf8ArrayToJson(packet.data);
            assert(json.type === TYPE_PUBLIC_KEY);
            if (json.type === TYPE_PUBLIC_KEY) {
                handlePublicKeyMessage(json, packet.clientId);
            }
        } else if (packet.type == MT_CLIENT_KEY_ENC) {
            let json = clientKeyDecrypt(packet.data);
            json = JSON.parse(json);
            handleMessage(json.type, json);
        } else if (packet.type == MT_ERROR) {
            console.error("Response From WS Server: ", utf8ArrayToString(packet.data));
        }
    });
}

/**
 * 从 server 下发 public key，重新初始化连接的 id 和 AES key.
 * 
 * @param {*} json 
 * @param {*} client_id 
 */
function handlePublicKeyMessage(json, client_id) {
    g_clientId = client_id;
    g_clientKey = forge.random.getBytesSync(32);
    let keyStr = bytesToUtf8Array(g_clientKey).join(',');

    g_pubKey = forge.pki.publicKeyFromPem(json.pub_key);

    // 保存在 localStorage 中.
    localStorage.setItem('client_id', client_id);
    localStorage.setItem('key', keyStr);

    let bytes = jsonToUtf8Bytes({
        type: TYPE_INIT_CONNECTION,
        key: keyStr,
        clientName: getBrowserName(),
    });

    // 使用 public key 加密
    let data = bytesToUtf8Array(g_pubKey.encrypt(bytes));

    // 发送初始化连接的请求
    socket.send(packDataPacket(MT_PUB_KEY_ENC, g_clientId, data));
}

function handleInitConnection(json) {
    assert(json.result == 'OK');
    if (json.result == 'OK') {
        network.status = STATUS_INITED;
        network.description = 'Connected successfully.';
    }
}

function handleMessage(type, json) {
    let handler = handlers[type];
    if (handler) {
        handler(json);
    } else {
        console.warn('No handler associated:', type);
    }
}

const RC_PLAY = 'play',
      RC_PAUSE = 'plause',
      RC_PLAY_PAUSE = 'play_plause',
      RC_STOP = 'stop',
      RC_STATUS = 'status',
      RC_VOLUME = 'volume',
      RC_POSITION = 'position',
      RC_MEDIA_INFO = 'info',
      RC_SHUFFLE = 'shuffle',
      RC_LOOP = 'shuffle',
      RC_SET_MEDIA_INFO = 'set_info';

export function registerHandler(name, handler) {
    assert(handlers[name] == null);
    handlers[name] = handler;
}

export function sendPlayerCommand(cmd, parameter) {
    let json = {
        type: TYPE_PLAYER_REMOTE_CTRL,
        cmd: cmd,
    };
    if (parameter != undefined) {
        json.parameter = parameter;
    }

    let data = bytesToUtf8Array(clientKeyEncrypt(json));
    data = packDataPacket(MT_CLIENT_KEY_ENC, g_clientId, data);
    if (socket.readyState == WebSocket.OPEN) {
        socket.send(data);
    } else if (socket.readyState != WebSocket.CONNECTING) {
        let now = getTimeInSeconds();
        if (now - lastConnectTime >= 10) {
            init();
        } else {
            network.status = STATUS_DISCONNECTED;
            network.description = 'Disconnected. Will try again in 10 seconds.';
            setTimeout(init, 10 * 1000);
        }
    }
}

function test() {
    g_clientKey = forge.random.getBytesSync(32);
    let data = clientKeyEncrypt('abc, 中defg');
    console.log(data.length);
    data = clientKeyDecrypt(data);
    console.log(data.length, data);
}

test();
init();

