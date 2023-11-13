
function currentTime() {
    return new Date().getTime();
}

function stringFormat(text) {
    var formatArgs = arguments;
    return text.replace(/\{(.+?)\}/g, function(str, p){
        return formatArgs[parseInt(p) + 1];
    });
}

function composeUrl(url, parameters) {
    var a = [];
    for (var name in parameters) {
        a.push(encodeURIComponent(name) + '=' + encodeURIComponent(parameters[name]));
    }
    if (a.length > 0) {
        return url + '?' + a.join('&');
    }
    return url;
}

function formatSize(size) {
    var UINTS = ["Bytes", "KB", "M", "G"]
    var unitIndex = 0
    while (size > 1024 && unitIndex < UINTS.length - 1) {
        size /= 1024.0
        unitIndex += 1
    }

    return size.toFixed(1).replace(/0+$/, '').replace(/\.+$/, '') + " " + UINTS[unitIndex];
}

function toFixed(n, precision) {
    if (n == Math.round(n)) {
        return n.toString();
    }

    if (!precision) {
        precision = 3;
    }
    return n.toFixed(precision);
}

function formatDuration(duration) {
    if (duration === -1) {
        return -1;
    }

    duration /= 1000.0;
    if (duration < 60) {
        text = toFixed(duration) + ' 秒';
    } else {
        text = stringFormat('{0} 分 {1} 秒', Math.round(duration / 60), toFixed(duration % 60));
    }
    return text;
}

function funcFormatTextEllisis(maxLength) {
    return function (text) {
        if (!text) {
            return text;
        }
    
        if (text.length > maxLength) {
            text = text.substr(0, maxLength / 2) + ' ... ' + text.substr(text.length - maxLength / 2 - 5);
        }
        return text;
    }    
}

function copyProperties(dst, src) {
    for (let name in src) {
        dst[name] = src[name];
    }
}

function strPartition(s, withStr) {
    var pos = s.indexOf(withStr);
    if (pos === -1) {
        return [s, null];
    }

    return [s.substr(0, pos), s.substr(pos + 1)];
}

function strPartition3(s, withStr) {
    var pos1 = s.indexOf(withStr);
    if (pos1 === -1) {
        return [s, null, null];
    }

    var pos2 = s.indexOf(withStr, pos1 + 1);
    if (pos2 === -1) {
        return [s.substring(0, pos1), s.substring(pos1 + 1), null];
    }

    return [s.substring(0, pos1), s.substring(pos1 + 1, pos2), s.substring(pos2 + 1)];
}

function assert(condition) {
    if (!condition) {
        throw "assert failed with condition: " + condition;
    }
}

function deepCopy(o) {
    var output, v, key;
    output = Array.isArray(o) ? [] : {};
    for (key in o) {
        v = o[key];
        if (v == null) {
            output[key] = v;
        } else {
            output[key] = (typeof v === "object") ? deepCopy(v) : v;
        }
    }
    return output;
}

function copyTextToClipboard(text) {
    function fallbackCopyTextToClipboard(text) {
        var textArea = document.createElement("textarea");
        textArea.value = text;
        
        // Avoid scrolling to bottom
        textArea.style.top = "0";
        textArea.style.left = "0";
        textArea.style.position = "fixed";
      
        document.body.appendChild(textArea);
        textArea.focus();
        textArea.select();
      
        try {
            var successful = document.execCommand('copy');
            var msg = successful ? 'successful' : 'unsuccessful';
            console.log('Fallback: Copying text command was ' + msg);
        } catch (err) {
            console.error('Fallback: Oops, unable to copy', err);
        }
      
        document.body.removeChild(textArea);
    }

    if (!navigator.clipboard) {
        fallbackCopyTextToClipboard(text);
        return;
    }
    navigator.clipboard.writeText(text).then(function() {
        console.log('Async: Copying to clipboard was successful!');
    }, function(err) {
        console.error('Async: Could not copy text: ', err);
    });
}


const RC_OK                                       = 0; // 成功

// 下面的变量是在 Navigation 中设置的，用于其他 App 设置值
window.displayErrorMessage = console.error;
window.setAjaxRunning = function () { }

/**
 * 
 * @param {string} url 
 * @param {function name(data) {
 }} callback 
 * @param {string} method 
 * @param {dict} query 
 * @param {dict} postBody 
 */
function apiRequest(url, onDataCallback, method, query, postBody, onFailedCallback) {

    method = method || 'GET';
    query = query || {};
    setAjaxRunning(true);

    if (typeof postBody != 'string') {
        postBody = JSON.stringify(postBody);
    }

    if (!onFailedCallback) {
        onFailedCallback = displayErrorMessage;
    }

    fetch(composeUrl(url, query), {
        method: method,
        cache: 'no-cache',
        headers: {
            "Content-Type": "application/json",
        },
        body: postBody,
    })
    .then(function (stream) {
        if (stream.status == 404) {
            onFailedCallback('找不到 API: ' + url + '. \n');
            return;
        }

        if (stream.headers.get('content-type').indexOf('application/json') == -1) {
            stream.text().then(function (text) {
                onFailedCallback(text);
            });
            return;
        }

        stream.json()
        .then(function (r) {
            if (stream.status == 200) {
                onDataCallback(r);
            } else {
                assert(r.code != RC_OK);
                if (r.message) {
                    onFailedCallback(r.message);
                } else {
                    if (r.code != null) {
                        onFailedCallback('错误代码: ' + r.code);
                    } else {
                        onFailedCallback('响应格式错误: ' + JSON.stringify(r));
                    }
                }
            }
        })
        .catch(function (error) {
            onFailedCallback(error);
        });
    })
    .catch(function (error) {
        onFailedCallback(error);
    })
    .finally(function () {
        setAjaxRunning(false);
    });
}

/**
 * 异步执行 apiRequest
 * 
 * @param {string} url 
 * @param {function name(data) {
 }} callback 
 * @param {string} method 
 * @param {dict} query 
 * @param {dict} postBody 
 */
 async function apiRequestAsync(url, method, query, postBody) {

    method = method || 'GET';
    query = query || {};
    setAjaxRunning(true);

    if (typeof postBody != 'string') {
        postBody = JSON.stringify(postBody);
    }

    return await fetch(composeUrl(url, query), {
        method: method,
        cache: 'no-cache',
        headers: {
            "Content-Type": "application/json",
        },
        body: postBody,
    })
    .then(function (stream) {
        if (stream.status == 404) {
            throw new Error('找不到 API: ' + url + '. \n');
        }

        if (stream.headers.get('content-type').indexOf('application/json') == -1) {
            stream.text().then(function (text) {
                throw new Error(text);
            });
        }

        return stream.json()
        .then(function (r) {
            if (stream.status == 200) {
                return r;
            } else {
                assert(r.code != RC_OK);
                if (r.message) {
                    throw new Error(r.message);
                } else {
                    if (r.code != null) {
                        throw new Error('错误代码: ' + r.code);
                    } else {
                        throw new Error('响应格式错误: ' + JSON.stringify(r));
                    }
                }
            }
        });
    })
    .finally(function () {
        setAjaxRunning(false);
    });
}

var g_prefs = {
};

function getPref(name, defVal) {
    let obj = g_prefs[name];
    if (obj == null) {
        if (defVal == null) {
            defVal = {};
        }
        g_prefs[name] = obj = defVal;
    }

    return obj;
}

function savePrefs() {
    localStorage.setItem('prefs', JSON.stringify(g_prefs));
}

function loadPrefs() {
    // 确保 g_prefs 的 object 数据结构是正确的
    try {
        let prefs = JSON.parse(localStorage.getItem('prefs')) || {};
        for (let k in g_prefs) {
            if (typeof(prefs[k]) != 'object') {
                prefs[k] = g_prefs[k];
            }
        }
        g_prefs = prefs;
    } catch (error) {
        console.error('Failed to parse prefs: ', error);
    }
}

function initPrefs() {
    addEventListener('beforeunload', () => {
        savePrefs();
    });

    loadPrefs();
}

function toTransparentCanvas(canvas) {
    const ctx = canvas.getContext("2d");
    const image = ctx.getImageData(0, 0, canvas.width, canvas.height);
    const width = image.width, height = image.height;
    const data = image.data; // Uint8ClampedArray
    const size = width * height * 4;

    let maxValue = 0;
    for (let i = 0; i < size; i += 4) {
        const c = 255 * 3 - (data[i] + data[i + 1] + data[i + 1]);
        if (c > maxValue) {
            maxValue = c;
        }
    }

    for (let i = 0; i < size; i += 4) {
        let c = 255 * 3 - (data[i] + data[i + 1] + data[i + 1]);
        c = Math.round(255 * c / maxValue);
        data[i] = c;
        data[i + 1] = c;
        data[i + 2] = c;
        data[i + 3] = c;
    }

    ctx.putImageData(image, 0, 0);
}

function saveElementEx(selector, fileTitle, scale) {
    if (scale > 1) {
        fileTitle += `@${ scale }x`;
    }

    html2canvas(document.querySelector(selector), {
        scale: scale,
        // width: 400,
        // width: (iconFontSize.value + iconMarginX.value * 2) * choosenIconText.length,
    }).then(canvas => {
        // document.body.appendChild(canvas);
        toTransparentCanvas(canvas);
        canvas.toBlob(blob => {
            saveAs(blob, fileTitle + ".png");
        }, "image/png");
    });
}

function saveElement(selector, fileTitle) {
    saveElementEx(selector, fileTitle, 1);
    saveElementEx(selector, fileTitle, 2);
}

function init() {
    initPrefs();
}

init();

