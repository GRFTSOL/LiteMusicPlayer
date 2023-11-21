let colorDialog = Vue.reactive({
    show: false,
    color: '#FFFFFF',
    callback: (clr) => {},
});

let histColors = Vue.reactive(getPref('hist-colors'));
if (!(histColors instanceof Array)) {
    histColors = histColors = [];
}
let colors = loadColors('colors', 0);
let bgColors = loadColors('bgColors', colors.length);
let fgColors = loadColors('fgColors', colors.length);

let tabChangedCallbacks = {};
function addTabChangedCallback(name, cb) {
    tabChangedCallbacks[name] = cb;
}

function loadColors(name, length) {
    let colors = getPref(name);
    if (!(colors instanceof Array)) {
        g_prefs[name] = colors = [];
    }

    for (let i = 0; i < length; i++) {
        if (colors[i] == null) {
            colors[i] = '#FFFFFF';
        } else {
            addHistoryColor(colors[i]);
        }
    }

    return Vue.reactive(colors);
}

function addHistoryColor(color) {
    if (!color.startsWith('#')) return;
    for (const c of histColors) {
        if (c === color) {
            return;
        }
    }
    histColors.push(color);
    if (histColors.length > 20) {
        histColors.splice(0, histColors.length - 20);
    }
}

function countOfChoosedColors(flags) {
    let count = 0;
    for (let i in colors) {
        if (flags[i]) {
            count++;
        } else if (flags[i] == undefined) {
            flags[i] = false;
        }
    }
    return count;
}

function toIntProps(obj) {
    for (const name in obj) {
        let v = obj[name];
        if (typeof v == 'string' && parseFloat(v) == v) {
            obj[name] = parseFloat(v);
        } else if (v instanceof Array) {
            toIntProps(v);
        } else if (v instanceof Object) {
            toIntProps(v);
        }
    }
}

function scaleIntProps(obj, scale) {
    for (const name in obj) {
        let v = obj[name];
        if (typeof v == 'number' && !name.startsWith('alpha')) {
            obj[name] = v * scale;
        } else if (v instanceof Array) {
            scaleIntProps(v, scale);
        } else if (v instanceof Object) {
            scaleIntProps(v, scale);
        }
    }
}

function clearCanvas(id) {
    const canvas = document.getElementById(id);
    canvas.width = canvas.height = 0;
}

function saveCanvas(name, nameVert) {
    function _save(id, name) {
        let canvas = document.getElementById(id);
        canvas.toBlob(blob => {
            saveAs(blob, name + ".png");
        }, "image/png");    
    }

    _save("canvasCtrls", name);
    _save("canvasCtrls2x", name + "@2x");

    if (nameVert) {
        _save("canvasCtrlsVert", nameVert);
        _save("canvasCtrlsVert2x", nameVert + "@2x");
    }
}

function initAppCtrl(name, ctrlDefValue, drawCtrl, drawCtrlVert) {
    const ctrl = Vue.reactive(getPref(name, ctrlDefValue));
    copyPropertiesIfNotExist(ctrl, ctrlDefValue);

    Vue.watch(ctrl, updateCanvas);
    tabChangedCallbacks[name] = updateCanvas;

    function updateCanvas() {
        toIntProps(ctrl);

        drawCtrl("canvasCtrls", ctrl);
        if (drawCtrlVert) drawCtrlVert("canvasCtrlsVert", ctrl); else clearCanvas("canvasCtrlsVert");

        const ctrl2x = deepCopy(ctrl);
        scaleIntProps(ctrl2x, 2);
        drawCtrl("canvasCtrls2x", ctrl2x);
        if (drawCtrlVert) drawCtrlVert("canvasCtrlsVert2x", ctrl2x); else clearCanvas("canvasCtrlsVert2x");
    }

    return ctrl;
}

function appIcons() {
    const ctrl = initAppCtrl('icons', {
        tabs: [],
    }, drawCtrl);

    const iconDialog = Vue.reactive({
        show: false,
        tabName: 'codicon',
        replaceColor: true,
        callback: (icon, type, replaceColor) => {},
    });

    let codiconText = [];
    // 字体的信息使用 https://fontdrop.info/ 分析自 glyphIndexMap
    for (let i = 60000; i < 60447; i++) {
        codiconText.push(String.fromCodePoint(i));
    }
    codiconText = codiconText.join('');

    const fonts = Vue.reactive([
        { name: 'codicon', icon: 'ea60', text: codiconText },
        { name: 'Material Icons', icon: 'e8b6', text: '', url: 'https://fonts.google.com/icons?icon.set=Material+Icons&icon.platform=web&icon.query=delete' },
    ]);

    const images = {};
    for (let tab of ctrl.tabs) {
        for (let item of tab.icons) {
            addImage(item);
        }
    }

    function removeChoosenIconText(icons, index) {
        if (confirm("Click 'OK' to remove this icon.")) {
            icons.splice(index, 1);
        }
    }

    function addImage(item, onload) {
        if (item.type == '_image_' && !images[item.icon]) {
            const image = new Image();
            image.src = 'images/' + item.icon;
            image.onload = onload;
            images[item.icon] = image;
        }
    }

    function saveIcons() {
        const tab = ctrl.tabs.find(tab => tab.name == ctrl.tabName);
        if (tab) {
            saveCanvas(tab.name);
        }
    }

    function drawCtrl(canvasId, ctrl) {
        const tab = ctrl.tabs.find(tab => tab.name == ctrl.tabName);
        if (!tab) return;

        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(tab.colors);
        const widthItem = tab.fontSize + tab.marginX * 2;
        const heightItem = tab.fontSize + tab.marginY * 2;

        canvas.width = widthItem * tab.icons.length;
        canvas.height = heightItem * countColors;

        for (let item of tab.icons) {
            addImage(item, () => drawCtrl(canvasId, ctrl));
        }

        let y = 0;
        for (let i in colors) {
            if (tab.colors[i]) {
                let x = 0;
                for (let item of tab.icons) {
                    if (item.type == '_image_') {
                        const image = images[item.icon];
                        if (image) {
                            drawImage(ctx, image, x, y, widthItem, heightItem, item.replaceColor, colors[i]);
                        }
                    } else {
                        drawTextCenter(ctx, x, y, widthItem, heightItem, item.icon, tab.fontSize, colors[i], 0, item.type);
                    }
                    x += widthItem;
                }

                y += heightItem;
            }
        }
    }

    function onDropIcon(ev, icons, curIndex) {
        ev.preventDefault();
        var index = ev.dataTransfer.getData("index");
        if (parseInt(index) == index) {
            const item = icons[index];
            icons.splice(index, 1);
            icons.splice(curIndex, 0, item);
        }
    }

    function showAddIconsDialog(tab) {
        iconDialog.show = true;
        iconDialog.callback = (icon, type, replaceColor) => {
            for (let i = 0; i < tab.icons.length; i++) {
                const item = tab.icons[i];
                if (item.icon == icon && item.type == type) {
                    return;
                }
            }

            tab.icons.push({ icon, type, replaceColor });
        };
    }

    function addIconByHexCode(font) {
        const n = parseInt(font.icon, 16);
        if (!n) {
            return;
        }

        iconDialog.callback(String.fromCodePoint(n), font.name);
    }

    function addIconTab() {
        const name = 'Untitled';
        ctrl.tabs.push({
            name,
            marginX: 0,
            marginY: 0,
            fontSize: 40,
            fontWeight: 400,
            icons: [],
            colors: [],
        });
        ctrl.tabName = name;
    }

    function deleteIconTab(tabIdx) {
        if (confirm("Click 'OK' to remove this tab of icons.")) {
            ctrl.tabs.splice(tabIdx, 1);
        }
    }

    return {
        icons: ctrl,
        addIconTab,
        deleteIconTab,
        showAddIconsDialog,

        fonts,
        iconDialog,
        addIconByHexCode,

        removeChoosenIconText,
        saveIcons,
        onDropIcon,
    };
}

function roundRect(ctx, x, y, width, height, radius, color, lineWidth) {
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = color;
    ctx.beginPath();
    ctx.roundRect(x + lineWidth / 2, y + lineWidth / 2, width - lineWidth, height - lineWidth, [radius]);
    ctx.stroke();
}

function fillRoundRect(ctx, x, y, width, height, radius, color) {
    if (color) {
        ctx.fillStyle = color;
        ctx.beginPath();
        ctx.roundRect(x, y, width, height, [radius]);
        ctx.fill();
    }
}

function fillEdgeRoundRect(ctx, x, y, width, height, radius, colorFill, colorEdge, lineWidth) {
    fillRoundRect(ctx, x, y, width, height, radius + (lineWidth > 0) * 2, colorFill);
    if (lineWidth > 0) {
        roundRect(ctx, x, y, width, height, radius, colorEdge, lineWidth);
    }
}

function drawLine(ctx, x1, y1, x2, y2, color, width) {
    ctx.lineWidth = width;
    ctx.strokeStyle = color;

    ctx.beginPath();
    ctx.moveTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.stroke();
}

function drawTextCenter(ctx, x, y, width, height, text, fontSize, textColor, rotate, fontName) {
    if (!text || fontSize == 0) {
        return;
    }

    if (!fontName) fontName = 'Material Icons';

    ctx.save()
    ctx.font = `${ fontSize }px ${fontName}`;
    ctx.fillStyle = textColor;

    if (rotate) {
        const size = ctx.measureText(text);
        rotate %= 360;
        let x1 = x + width / 2, y1 = y + height / 2;
        if (rotate == 90) {
            x1 -= size.width / 2; y1 -= size.width / 2;
        } else if (rotate == 180) {
            x1 += size.width / 2; y1 -= size.width / 2;
        } else if (rotate == 270) {
            x1 += size.width / 2; y1 += size.width / 2;
        }
        ctx.textAlign = 'left';
        ctx.translate(x1, y1);
        ctx.rotate(Math.PI * rotate / 180)
        ctx.fillText(text, 0, 0);
    } else {
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';    
        ctx.fillText(text, x + width / 2, y + height / 2);
    }

    ctx.restore();
}

function drawImage(ctx, image, x, y, width, height, replaceColor, color) {
    ctx.drawImage(image, x, y, width, height);
    if (replaceColor) {
        const r = parseInt(color.substr(1, 2), 16);
        const g = parseInt(color.substr(3, 2), 16);
        const b = parseInt(color.substr(5, 2), 16);
        const imageData = ctx.getImageData(x, y, width, height);
        const data = imageData.data;
        const length = width * height * 4;
        for (let i = 0; i < length; i += 4) {
            if (data[i + 3]) {
                data[i] = r; data[i + 1] = g; data[i + 2] = b;
            }
        }

        ctx.putImageData(imageData, x, y);
    }
}

function appButton() {
    const ctrl = initAppCtrl('button', {
        width: 40,
        height: 26,
        cornerRadius: 3,
        edge: 2,
        margin: 2,
        colors: [],
    }, drawCtrl);

    function drawCtrl(canvasId, ctrl) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(ctrl.colors);
        const width = ctrl.width - ctrl.margin * 2, height = ctrl.height - ctrl.margin * 2;

        canvas.width = ctrl.width;
        canvas.height = ctrl.height * countColors;

        let x = ctrl.margin, y = ctrl.margin;
        for (let i in colors) {
            if (ctrl.colors[i]) {
                drawItem(x, y, colors[i], bgColors[i]);

                y += ctrl.height;
            }
        }

        function drawItem(x, y, color, bgColor) {
            fillEdgeRoundRect(ctx, x, y, width, height, ctrl.cornerRadius, bgColor, color, ctrl.edge);
        }
    }

    return {
        bt: ctrl,
    };
}

function appButtonGroup() {
    const ctrl = initAppCtrl('button-group', {
        width: 60,
        height: 24,
        cornerRadius: 10,
        edge: 2,
        margin: 0,
        colors: [],
    }, drawCtrl);

    function drawCtrl(canvasId, ctrl) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(ctrl.colors);
        const width = ctrl.width - ctrl.margin * 2, height = ctrl.height - ctrl.margin * 2;
        canvas.width = ctrl.width;
        canvas.height = ctrl.height * countColors;

        let x = ctrl.margin, y = ctrl.margin;
        for (let i in colors) {
            if (ctrl.colors[i]) {
                drawItem(x, y, bgColors[i], colors[i], ctrl.edge);

                y += ctrl.height;
            }
        }

        function drawItem(x, y, fillColor, edgeColor, edge) {
            fillEdgeRoundRect(ctx, x, y, width, height, ctrl.cornerRadius, fillColor, edgeColor, edge);
            if (edge > 0) {
                drawLine(ctx, x + width / 2, y, x + width / 2, y + height, edgeColor, edge);
            }
        }
    }

    return {
        btg: ctrl,
    };
}

function appComboBox() {
    const ctrl = initAppCtrl('combo-box', {
        width: 60,
        height: 26,
        cornerRadius: 3,
        edge: 2,
        margin: 2,
        fontSize: 20,
        icon: 'e5c5',
        colors: [],
    }, drawCtrl);

    function drawCtrl(canvasId, ctrl) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(ctrl.colors);
        const width = ctrl.width - ctrl.margin * 2, height = ctrl.height - ctrl.margin * 2;

        canvas.width = ctrl.width;
        canvas.height = ctrl.height * countColors;

        const iconText = String.fromCodePoint(parseInt(ctrl.icon, 16) || '?'.charCodeAt(0));

        let x = ctrl.margin, y = ctrl.margin;
        for (let i in colors) {
            if (ctrl.colors[i]) {
                drawItem(x, y, colors[i], bgColors[i], fgColors[i]);

                y += ctrl.height;
            }
        }

        function drawItem(x, y, color, bgColor, fgColor) {
            fillEdgeRoundRect(ctx, x, y, width, height, ctrl.cornerRadius, bgColor, color, ctrl.edge);
            drawTextCenter(ctx, x + width - height - ctrl.edge, y, height, height, iconText, ctrl.fontSize, fgColor);
        }
    }

    return {
        cb: ctrl,
    };
}

function appFrame(name, title) {
    const ctrl = initAppCtrl(name, {
        name,
        title,
        width: 40,
        height: 26,
        cornerRadius: 3,
        edge: 2,
        margin: 0,
        colors: [],
    }, drawCtrl);

    function drawCtrl(canvasId, ctrl) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(ctrl.colors);
        const width = ctrl.width - ctrl.margin * 2, height = ctrl.height - ctrl.margin * 2;

        canvas.width = ctrl.width;
        canvas.height = ctrl.height * countColors;

        let x = ctrl.margin, y = ctrl.margin;
        for (let i in colors) {
            if (ctrl.colors[i]) {
                fillEdgeRoundRect(ctx, x, y, width, height, ctrl.cornerRadius, bgColors[i], colors[i], ctrl.edge);

                y += ctrl.height;
            }
        }
    }

    return ctrl;
}

function appChecks() {
    const ctrl = initAppCtrl('checks', {
        width: 40,
        height: 26,
        fontSize: 22,
        colors: [],
        icons: 'e834,e835,e836,e837',
    }, drawCtrl);

    function drawCtrl(canvasId, ctrl) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(ctrl.colors);
        const width = ctrl.width, height = ctrl.height;
        const icons = ctrl.icons.split(',');

        canvas.width = ctrl.width * icons.length;
        canvas.height = ctrl.height * countColors;

        ctx.font = `${ ctrl.fontSize }px Material Icons`;
        ctx.textAlign    = 'center';
        ctx.textBaseline = 'middle';

        let y = 0;
        for (let i in colors) {
            let x = 0;
            if (ctrl.colors[i]) {
                for (let icon of icons) {
                    ctx.fillStyle = bgColors[i];
                    const n = parseInt(icon, 16) || '?'.charCodeAt(0);
                    ctx.fillText(String.fromCodePoint(n), x + width / 2, y + height / 2);

                    x += ctrl.width;
                }

                y += ctrl.height;
            }
        }
    }

    return {
        checks: ctrl,
    };
}

function appProgress() {
    const ctrl = initAppCtrl('progress', {
        width: 40,
        height: 10,
        marginX: 2,
        marginY: 2,
        cornerRadius: 3,
        colors: [],
    }, drawCtrl);

    function drawCtrl(canvasId, ctrl) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(ctrl.colors);
        const width = ctrl.width - ctrl.marginX * 2, height = ctrl.height - ctrl.marginY * 2;

        canvas.width = ctrl.width;
        canvas.height = ctrl.height * countColors;

        let x = ctrl.marginX, y = ctrl.marginY;
        for (let i in colors) {
            if (ctrl.colors[i]) {
                fillEdgeRoundRect(ctx, x, y, width, height, ctrl.cornerRadius, bgColors[i], colors[i], 0);

                y += ctrl.height;
            }
        }
    }

    return {
        progress: ctrl,
    };
}

function appThumb() {
    const ctrl = initAppCtrl('thumb', {
        width: 20,
        height: 20,
        width1: 10,
        height1: 10,
        width2: 15,
        height2: 15,
        cornerRadius1: 5,
        cornerRadius2: 7,
        alpha2: 0.3,
        colors: [],
    }, drawCtrl);

    function drawCtrl(canvasId, ctrl) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(ctrl.colors);
        const width = ctrl.width, height = ctrl.height;

        canvas.width = ctrl.width * 3;
        canvas.height = ctrl.height * countColors;

        const width1 = ctrl.width1, height1 = ctrl.height1, width2 = ctrl.width2, height2 = ctrl.height2;
        const alpha2 = ctrl.alpha2;
        let y = 0;
        for (let i in colors) {
            if (ctrl.colors[i]) {
                let x = 0;
                const color = bgColors[i];
                fillRoundRect(ctx, x + (width - width1) / 2, y + (height - height1) / 2, width1, height1, ctrl.cornerRadius1, color);

                // two rect
                x += ctrl.width;
                ctx.globalAlpha = alpha2;
                fillRoundRect(ctx, x + (width - width2) / 2, y + (height - height2) / 2, width2, height2, ctrl.cornerRadius2, colors[i]);
                ctx.globalAlpha = 1;
                fillRoundRect(ctx, x + (width - width1) / 2, y + (height - height1) / 2, width1, height1, ctrl.cornerRadius1, color);
    
                // larger one
                x += ctrl.width;
                fillRoundRect(ctx, x + (width - width2) / 2, y + (height - height2) / 2, width2, height2, ctrl.cornerRadius2, color);
    
                y += ctrl.height;
            }
        }
    }

    return {
        thumb: ctrl,
    };
}

function appScrollBar() {
    // * Vertical/Horizontal

    const ctrl = initAppCtrl('scrollbar', {
        width: 100,
        height: 20,
        edge: 2,
        btnSize: 20,
        thumbSize: 30,
        cornerRadius: 4,
        alpha: 1.0,
        fontSize: 20,
        icon: 'e5c5',
        colors: [],
    }, drawCtrlHorz, drawCtrlVert);

    function drawCtrlHorz(canvasId, ctrl) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(ctrl.colors);
        const width = ctrl.width, height = ctrl.height;

        canvas.width = ctrl.width;
        canvas.height = ctrl.height * countColors;

        ctx.globalAlpha = ctrl.alpha;

        const trackSize = width - ctrl.btnSize * 2 - ctrl.thumbSize;
        const iconText = parseInt(ctrl.icon, 16) ? String.fromCodePoint(parseInt(ctrl.icon, 16)) : null;
        let y = 0;
        for (let i in colors) {
            if (ctrl.colors[i]) {
                // draw with color, no edge
                drawItem(0, y, colors[i], bgColors[i], fgColors[i]);

                y += height;
            }
        }

        function drawItem(x, y, thumbColor, fillColor, fgColor) {
            if (ctrl.btnSize > 0) {
                // button
                fillEdgeRoundRect(ctx, x, y, ctrl.btnSize, height, 0, fillColor, null, 0);
                if (fgColor)
                    drawTextCenter(ctx, x, y, ctrl.btnSize, height, iconText, ctrl.fontSize, fgColor, 90);
                x += ctrl.btnSize;
            }

            // Thumb
            fillEdgeRoundRect(ctx, x, y, ctrl.thumbSize, height, ctrl.cornerRadius, thumbColor, null, 0);
            x += ctrl.thumbSize;

            // Track
            fillEdgeRoundRect(ctx, x, y, trackSize, height, 0, fillColor, null, 0);
            x += trackSize;

            if (ctrl.btnSize > 0) {
                // button
                fillEdgeRoundRect(ctx, x, y, ctrl.btnSize, height, 0, fillColor, null, 0);
                if (fgColor)
                    drawTextCenter(ctx, x, y, ctrl.btnSize, height, iconText, ctrl.fontSize, fgColor, 270);
            }
        }
    }

    function drawCtrlVert(canvasId, ctrl) {
        const canvas = document.getElementById(canvasId);
        const ctx = canvas.getContext("2d");
        const countColors = countOfChoosedColors(ctrl.colors);
        const height = ctrl.width, width = ctrl.height;

        canvas.width = width * countColors;
        canvas.height = height;

        ctx.globalAlpha = ctrl.alpha;

        const trackSize = height - ctrl.btnSize * 2 - ctrl.thumbSize;
        const iconText = parseInt(ctrl.icon, 16) ? String.fromCodePoint(parseInt(ctrl.icon, 16)) : null;
        let x = 0;
        for (let i in colors) {
            if (ctrl.colors[i]) {
                // draw with color, no edge
                drawItem(x, 0, colors[i], bgColors[i], fgColors[i]);

                x += width;
            }
        }

        function drawItem(x, y, thumbColor, fillColor, fgColor) {
            if (ctrl.btnSize > 0) {
                // button
                fillEdgeRoundRect(ctx, x, y, width, ctrl.btnSize, 0, fillColor, null, 0);
                if (fgColor)
                    drawTextCenter(ctx, x, y, width, ctrl.btnSize, iconText, ctrl.fontSize, fgColor, 180);
                y += ctrl.btnSize;
            }

            // Thumb
            fillEdgeRoundRect(ctx, x, y, width, ctrl.thumbSize, ctrl.cornerRadius, thumbColor, null, 0);
            y += ctrl.thumbSize;

            // Track
            fillEdgeRoundRect(ctx, x, y, width, trackSize, 0, fillColor, null, 0);
            y += trackSize;

            if (ctrl.btnSize > 0) {
                // button
                fillEdgeRoundRect(ctx, x, y, width, ctrl.btnSize, 0, fillColor, null, 0);
                if (fgColor)
                    drawTextCenter(ctx, x, y, width, ctrl.btnSize, iconText, ctrl.fontSize, fgColor, 0);
            }
        }
    }

    return {
        sb: ctrl,
    };
}

const app = Vue.createApp({
    setup () {
        let tab = Vue.reactive(getPref('tab', {
            name: 'button',
        }));

        const colorNames = Vue.reactive(getPref('colorNames', ['Disabled', 'Normal', 'Hover/Focus', 'Pressed', 'Checked', 'C-Hover/Foucs', 'C-Pressed', 'Positive', 'Negative', 'Info', 'Warning', 'Dark']));

        Vue.watch(tab, () => {
            callUpdateCanvas();
        });

        setTimeout(() => {
            callUpdateCanvas();
        }, 0);

        // Need to call twice to draw the icon for the first time.
        setTimeout(() => {
            callUpdateCanvas();
        }, 100);

        const frames = [
            appFrame('button', 'Button'),
            appFrame('button-flat', 'Flat Button'),
            appFrame('edit', 'Edit'),
            appFrame('listctrl-frame', 'ListCtrl'),
            appFrame('frame-ctrl', 'Frame'),
        ];

        function callUpdateCanvas() {
            const canvas = document.getElementById('canvasCtrls');
            if (!canvas) {
                return;
            }

            const cb = tabChangedCallbacks[tab.name];
            if (cb) {
                cb();
            } else {
                clearCanvas("canvasCtrls");
                clearCanvas("canvasCtrls2x");
                clearCanvas("canvasCtrlsVert");
                clearCanvas("canvasCtrlsVert2x");
            }
        }

        function removePresetColor(index) {
            colors.splice(index, 1);
            bgColors.splice(index, 1);
        }

        function showColorDialog(color, callback) {
            colorDialog.show = true;
            colorDialog.color = color;
            colorDialog.callback = callback;
        }

        function onColorChoosed(color) {
            addHistoryColor(color);
            colorDialog.callback(color);
        }

        let confFileToLoad = Vue.ref('');
        Vue.watch(confFileToLoad, (file) => {
            if (file) {
                const reader = new FileReader();
                reader.addEventListener( "load", () => reloadConf(reader.result), false);
                reader.readAsText(file);
            }
        });

        function downloadConf() {
            const blob = new Blob([JSON.stringify(g_prefs, null, 2)], {
            type: "application/json",
            });
            saveAs(blob, 'ctrls-config.json');
        }

        function reloadConf(text) {
            try {
                const conf = JSON.parse(text);
                g_prefs = conf;
                location.reload();
            } catch (error) {
                console.log(text);
                alert("Failed to parse configuration, please make sure it's JSON format.");
            }
        }

        return {
            tab,
            colors,
            bgColors,
            fgColors,
            histColors,
            colorNames,
            removePresetColor,

            colorDialog,
            showColorDialog,
            onColorChoosed,

            saveCanvas,

            downloadConf,
            confFileToLoad,

            frames,
            ...appIcons(),
            ...appButton(),
            ...appButtonGroup(),
            ...appComboBox(),
            ...appChecks(),
            ...appProgress(),
            ...appThumb(),
            ...appScrollBar(),
        };
    }
});

app.use(Quasar);
app.mount('#q-app');
