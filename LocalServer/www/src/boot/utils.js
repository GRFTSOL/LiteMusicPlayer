
export function randArray(length, max) {
    return Array.from({length: length}, () => Math.floor(Math.random() * max));
}

export function encodeUtf8Bytes(s) {
    return unescape(encodeURIComponent(s));
}

export function decodeUtf8Bytes2(s) {
    // 遇到不支持的编码就会抛出异常，无法正确解码.
    return decodeURIComponent(escape(s));
}

export function decodeUtf8Bytes(s) {
    let a = [], n, len = s.length, Q = '?'.charCodeAt(0);

    for (let i = 0; i < len;) {
        let c = s.charCodeAt(i);
        if (c < 0x80) {
            n = c;
            i++;
        } else if (c < 0xc0) {
            n = Q;
            i++;
        } else if (c < 0xe0) {
            n = ((c & 0x3F) << 6) | (s.charCodeAt(i + 1) & 0x3F);
            i += 2;
        } else if (c < 0xf0) {
            n = ((c & 0x0F) << 12) | ((s.charCodeAt(i + 1) & 0x3F) << 6) | (s.charCodeAt(i + 2) & 0x3F);
            i += 3;
        } else if (c < 0xf8) {
            n = ((c & 0x07) << 18) | ((s.charCodeAt(i + 1) & 0x3F) << 12) |
                ((s.charCodeAt(i + 2) & 0x3F) << 6) | (s.charCodeAt(i + 3) & 0x3F);
            i += 4;
            if (n > 0xFFFF) {
                // https://mathiasbynens.be/notes/javascript-encoding#surrogate-formulae
                n -= 0x10000;
                a.push(String.fromCharCode((n >> 10) + 0xD800), String.fromCharCode((n % 0x400) + 0xDC00));
            } else {
                a.push(String.fromCharCode(n));
            }
            continue;
        } else if (c < 0xfc) {
            n = Q;
            i += 5;
        } else if (c < 0xfe) {
            n = Q;
            i += 6;
        } else {
            n = Q;
            i++;
        }

        a.push(String.fromCharCode(n));
    }

    return a.join('');
}

export function bytesToUtf8Array(s) {
    let a = [];
    for (let i = 0; i < s.length; i++) {
        a.push(s.charCodeAt(i));
    }
    return a;
}

export function assert(c) {
    if (!c) {
        throw 'Failed on condition';
    }
}

export function stringFormat(text) {
    var formatArgs = arguments;
    return text.replace(/\{(.+?)\}/g, function (_str, p) {
        return formatArgs[parseInt(p)];
    });
};

export function deepCopy(dst, src) {
    for (let k in src) {
        let vs = src[k];
        if (typeof vs === 'object') {
            let vd = dst[k];
            if (typeof vd === 'object') {
                deepCopy(vd, vs);
                continue;
            }
        }
        dst[k] = vs;
    }
}

export function appendUint32(arr, n) {
    assert(typeof n === 'number' && n >= -0x80000000 && n <= 0xFFFFFFFF);

    if (typeof n !== 'number') {
        n = 0;
    }

    arr.push((n >> 24) & 0xFF);
    arr.push((n >> 16) & 0xFF);
    arr.push((n >> 8) & 0xFF);
    arr.push(n & 0xFF);
}

export function readUint8(arr) {
    assert(arr._offset < arr.length);
    return arr[arr._offset++];
}

export function readUint32(arr) {
    assert(arr._offset + 4 <= arr.length);

    // ' >>> 0 ' 是为了将负数转换为正数
    return ((arr[arr._offset++] << 24) | (arr[arr._offset++] << 16) |
        (arr[arr._offset++] << 8) | arr[arr._offset++]) >>> 0;
}

export function encodeUint32ToBytes(length) {
    let a = [];
    appendUint32(a, length);
    return utf8ArrayToBytes(a);
}

export function decodeUint32FromBytes(str) {
    assert(str.length >= 4);

    let a = [
        str.charCodeAt(0),
        str.charCodeAt(1),
        str.charCodeAt(2),
        str.charCodeAt(3)];

    a._offset = 0;
    return readUint32(a);
}

export function jsonToUtf8Array(json) {
    let str = JSON.stringify(json);
    return new TextEncoder().encode(str);
}

export function jsonToUtf8Bytes(json) {
    return encodeUtf8Bytes(JSON.stringify(json));
}

export function utf8ArrayToString(arr) {
    return new TextDecoder().decode(arr);
}

export function utf8ArrayToBytes(arr) {
    let a = [];
    for (let i = 0; i < arr.length; i++) {
        a.push(String.fromCharCode(arr[i]));
    }
    return a.join('');
}

export function utf8ArrayToJson(arr) {
    let str = new TextDecoder().decode(arr);
    return JSON.parse(str);
}

function makeCRCTable() {
    let crcTable = [];
    for (let n = 0; n < 256; n++) {
        let c = n;
        for (let k = 0; k < 8; k++) {
            c = ((c&1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
        }
        crcTable[n] = c;
    }

    return crcTable;
}

let crcTable;

export function crc32(bytes) {
    if (!crcTable) {
        crcTable  = makeCRCTable();
    }

    let crc = 0 ^ (-1);

    for (let i = 0; i < bytes.length; i++) {
        crc = (crc >>> 8) ^ crcTable[(crc ^ bytes.charCodeAt(i)) & 0xFF];
    }

    return (crc ^ (-1)) >>> 0;
}
