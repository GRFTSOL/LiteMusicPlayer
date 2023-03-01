
export interface ArrayStream extends Uint8Array {
    _offset: number;
}

export function randArray(length: number, max: number) {
    return Array.from({ length: length }, () => Math.floor(Math.random() * max));
}

export function encodeUtf8Bytes(s: string) {
    return unescape(encodeURIComponent(s));
}

export function decodeUtf8Bytes2(s: string) {
    // 遇到不支持的编码就会抛出异常，无法正确解码.
    return decodeURIComponent(escape(s));
}

export function decodeUtf8Bytes(s: string) {
    const a = [], len = s.length, Q = '?'.charCodeAt(0);

    for (let i = 0; i < len;) {
        const c = s.charCodeAt(i);
        let n;
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

export function bytesToUtf8Array(s: string) {
    const a = [];
    for (let i = 0; i < s.length; i++) {
        a.push(s.charCodeAt(i));
    }
    return a;
}

export function assert(c: any) {
    if (!c) {
        throw 'Failed on condition';
    }
}

export function stringFormat(text: string, ...otherArgs: Array<any>) {
    return text.replace(/\{(.+?)\}/g, function (_str, p) {
        return otherArgs[parseInt(p)];
    });
};

export function deepCopy(dst: any, src: any) {
    if (src instanceof Array && dst instanceof Array) {
        for (let i = 0; i < src.length; i++) {
            const vs = dst[i], vd = src[i];
            if (typeof vs === 'object' && typeof vd === 'object') {
                deepCopy(vd, vs);
                continue;
            }
            dst[i] = vs;
        }
    }

    for (const k in src) {
        const vs = src[k];
        if (typeof vs === 'object') {
            const vd = dst[k];
            if (typeof vd === 'object') {
                deepCopy(vd, vs);
                continue;
            }
        }
        dst[k] = vs;
    }

    return dst;
}

export function appendUint32(arr: Array<number>, n: number) {
    assert(typeof n === 'number' && n >= -0x80000000 && n <= 0xFFFFFFFF);

    if (typeof n !== 'number') {
        n = 0;
    }

    arr.push((n >> 24) & 0xFF);
    arr.push((n >> 16) & 0xFF);
    arr.push((n >> 8) & 0xFF);
    arr.push(n & 0xFF);
}

export function readUint8(arr: ArrayStream) {
    assert(arr._offset < arr.length);
    return arr[arr._offset++];
}

export function readUint32(arr: ArrayStream) {
    assert(arr._offset + 4 <= arr.length);

    // ' >>> 0 ' 是为了将负数转换为正数
    return ((arr[arr._offset++] << 24) | (arr[arr._offset++] << 16) |
        (arr[arr._offset++] << 8) | arr[arr._offset++]) >>> 0;
}

export function encodeUint32ToBytes(length: number) {
    const a: Array<number> = [];
    appendUint32(a, length);
    return utf8ArrayToBytes(a);
}

export function decodeUint32FromBytes(str: string) {
    assert(str.length >= 4);

    const a = [
        str.charCodeAt(0),
        str.charCodeAt(1),
        str.charCodeAt(2),
        str.charCodeAt(3)];

    const as = (a as any) as ArrayStream;
    as._offset = 0;
    return readUint32(as);
}

export function jsonToUtf8Array(json: object) {
    const str = JSON.stringify(json);
    return new TextEncoder().encode(str);
}

export function jsonToUtf8Bytes(json: object) {
    return encodeUtf8Bytes(JSON.stringify(json));
}

export function utf8ArrayToString(arr: Uint8Array) {
    return new TextDecoder().decode(arr);
}

export function utf8ArrayToBytes(arr: ArrayLike<number>) {
    const a = [];
    for (let i = 0; i < arr.length; i++) {
        a.push(String.fromCharCode(arr[i]));
    }
    return a.join('');
}

export function utf8ArrayToJson(arr: BufferSource) {
    const str = new TextDecoder().decode(arr);
    return JSON.parse(str);
}

function makeCRCTable() {
    const crcTable = [];
    for (let n = 0; n < 256; n++) {
        let c = n;
        for (let k = 0; k < 8; k++) {
            c = ((c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
        }
        crcTable[n] = c;
    }

    return crcTable;
}

let crcTable: Array<number>;

export function crc32(bytes: string) {
    if (!crcTable) {
        crcTable = makeCRCTable();
    }

    let crc = 0 ^ (-1);

    for (let i = 0; i < bytes.length; i++) {
        crc = (crc >>> 8) ^ crcTable[(crc ^ bytes.charCodeAt(i)) & 0xFF];
    }

    return (crc ^ (-1)) >>> 0;
}

export function strPartition(s: string, sep: string) {
    const index = s.indexOf(sep);
    if (index == -1) {
        return [s, ''];
    } else {
        return [s.substring(0, index), s.substring(index + 1)];
    }
}

function paddingZero(n: number, width: number) {
    let str = String(Math.floor(n));
    while (str.length < width) {
        str = '0' + str;
    }
    return str;
}

export function formatDuration(n: number) {
    n /= 1000;
    let str = ':' + paddingZero(n % 60, 2); // Seconds
    n /= 60;
    if (n >= 1) {
        str = paddingZero(n % 60, 2) + str; // Minutes
        n /= 60;
        if (n >= 1) {
            str = paddingZero(n, 0) + ':' + str; // Hours
        } else {
            if (str[0] == '0') {
                str = str.slice(1);
            }
        }
    } else {
        // Minutes: 0
        str = '0' + str;
    }
    return str;
}

export function formatRecentDateTime(n: number) {
    const now = new Date().getTime() / 1000;
    let duration = Math.abs(now - n);

    if (duration < 60) {
        return 'Less than one minutes'
    }

    duration /= 60;
    if (duration <= 60) {
        return stringFormat('{0} minutes ago', Math.floor(duration));
    }

    duration /= 60;
    if (duration <= 24) {
        return stringFormat('{0} hours ago', Math.floor(duration));
    }

    duration /= 24;
    if (duration <= 30) {
        return stringFormat('{0} days ago', Math.floor(duration));
    }

    duration /= 30;
    if (duration <= 12) {
        return stringFormat('{0} months ago', Math.floor(duration));
    }

    duration /= 12;
    return stringFormat('{0} years ago', Math.floor(duration));
}

export function formatBitRate(n: number) {
    if (n <= 0) {
        return '';
    }

    return (Math.floor(n / 1000)).toString() + 'k';
}

export function formatDurationML(n: number) {
    if (n == 0) {
        return '';
    }
    return formatDuration(n);
}

export function formatTimePlayed(n: number) {
    return n == 0 ? '' : formatRecentDateTime(n);
}

export function loadConfig(name: string, version: number, def?: any): any {
    const confText = localStorage.getItem(name);
    if (confText) {
        try {
            const conf = JSON.parse(confText);
            if (conf && conf._version === version) {
                return conf;
            }
        } catch (error) {
            console.warn(stringFormat('Failed to load config {0} from localStorage', name), error);
        }
    }

    return def;
}

export function saveConfig(name: string, version: number, conf: any): any {
    conf._version = version;

    try {
        localStorage.setItem(name, JSON.stringify(conf));
    } catch (error) {
        console.warn(stringFormat('Failed to save config {0}', name), error);
    }
}
