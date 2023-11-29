#include "SkinToolTip.h"
#include "Skin.h"
#include "SkinWnd.h"


#define TIMER_SPAN_TOOLTIP    (1 * 1000)

CSkinToolTip::CSkinToolTip(CSkinWnd *skinWnd, int timerId) : _skinWnd(skinWnd), TIMER_ID_TOOLTIP(timerId) {

    _fontProperty.setProperty("TextColor", "#FFFFFF");
    _clrBg.set(0, 0, 0);
}

CSkinToolTip::~CSkinToolTip() {
}

void CSkinToolTip::addToolTip(const StringView &text, CRect &rc, uint32_t idTooltip) {
    for (auto &item : _tooltips) {
        assert(item.id != idTooltip);
    }

    _tooltips.push_back({idTooltip, rc, text.toString()});
}

void CSkinToolTip::delToolTip(uint32_t idTooltip) {
    for (auto it = _tooltips.begin(); it != _tooltips.end(); ++it) {
        if ((*it).id == idTooltip) {
            _tooltips.erase(it);
            return;
        }
    }
}

bool CSkinToolTip::setProperty(cstr_t property, cstr_t value) {
    if (strIsSame(property, "BgColor")) {
        _clrBg = parseColorString(value);
    } else {
        return _fontProperty.setProperty(property, value);
    }

    return true;
}

void CSkinToolTip::onCreate() {
    _fontProperty.setParent(_skinWnd);
}

void CSkinToolTip::onDestroy() {
    _tooltips.clear();
    _fontProperty.clear();
}

void CSkinToolTip::onTimer(uint32_t idEvent) {
    _skinWnd->killTimer(TIMER_ID_TOOLTIP);
    _isTimerSet = false;
    _isDisplayTooltip = true;
    invalidate();
}

void CSkinToolTip::onMouseMove(CPoint point) {
    if (_isTimerSet) {
        _skinWnd->killTimer(TIMER_ID_TOOLTIP);
        _isTimerSet = false;
    }

    for (auto &tip : _tooltips) {
        if (tip.rc.ptInRect(point)) {
            if (_rcCurTooltip.equal(tip.rc) && _txtCurTooltip == tip.text &&
                (_isTimerSet || _isDisplayTooltip)) {
                // In/Already displaying tooltip.
                return;
            }

            _rcCurTooltip = tip.rc;
            _txtCurTooltip = tip.text;

            auto now = getTickCount();
            if (now - _timeHideTooltip < 500 || _isDisplayTooltip) {
                // 连续显示 tooltip.
                _isDisplayTooltip = true;
                _skinWnd->invalidateRect(&_rcDisplayArea);
                invalidate();
            } else {
                // 等待一段时间显示 tooltip
                _isTimerSet = true;
                _skinWnd->setTimer(TIMER_ID_TOOLTIP, TIMER_SPAN_TOOLTIP);
            }
            return;
        }
    }

    if (_isDisplayTooltip) {
        // 隐藏 tooltip
        _isDisplayTooltip = false;
        _skinWnd->invalidateRect(&_rcDisplayArea);
        _timeHideTooltip = getTickCount();
    }
}

void CSkinToolTip::onPaint(CRawGraph *canvas) {
    if (!_isDisplayTooltip || _rcDisplayArea.empty()) {
        return;
    }

    CRect rc;
    _skinWnd->getClientRect(&rc);

    CRect rcFill;
    VecPoints triangle;

    int x = (_rcCurTooltip.left + _rcCurTooltip.right) / 2;

    if (_isDrawAbove) {
        // 在上方显示
        rcFill = makeRectLTWH(_rcDisplayArea.left, _rcDisplayArea.top, _rcDisplayArea.width(), _rcDisplayArea.height() - _heightHandle);

        triangle = {
            {x - _heightHandle / 2, _rcDisplayArea.bottom - _heightHandle},
            {x + _heightHandle / 2, _rcDisplayArea.bottom - _heightHandle},
            {x, _rcDisplayArea.bottom},
        };
    } else {
        // 在下方显示
        rcFill = makeRectLTWH(_rcDisplayArea.left, _rcDisplayArea.top + _heightHandle, _rcDisplayArea.width(), _rcDisplayArea.height() - _heightHandle);

        triangle = {
            {x - _heightHandle / 2, _rcDisplayArea.top + _heightHandle},
            {x + _heightHandle / 2, _rcDisplayArea.top + _heightHandle},
            {x, _rcDisplayArea.top},
        };
    }

    canvas->fillRoundedRect(rcFill.left, rcFill.top, rcFill.width(), rcFill.height(), _margin, _clrBg);
    canvas->fillPath(triangle, _clrBg);

    canvas->setTextColor(_fontProperty.getTextColor());
    canvas->setFont(_fontProperty.getFont());

    canvas->drawText(_txtCurTooltip, rcFill, DT_CENTER | DT_VCENTER);
}

void CSkinToolTip::stopDisplayTooltip() {
    if (_isTimerSet) {
        _skinWnd->killTimer(TIMER_ID_TOOLTIP);
        _isTimerSet = false;
    }

    if (_isDisplayTooltip) {
        _isDisplayTooltip = false;
        _skinWnd->invalidateRect(&_rcDisplayArea);
    }
}

void CSkinToolTip::invalidate() {
    CRect rc;
    _skinWnd->getClientRect(&rc);

    CSize size = {0, 0};
    auto font = _fontProperty.getFont();
    font->getTextExtentPoint32(_txtCurTooltip.c_str(), _txtCurTooltip.size(), &size);
    size.cx += _margin;
    size.cy += _margin + _heightHandle;

    _rcDisplayArea.left = (_rcCurTooltip.left + _rcCurTooltip.right - size.cx) / 2;
    if (_rcDisplayArea.left + size.cx > rc.right) {
        _rcDisplayArea.left = rc.right - size.cx;
    }
    if (_rcDisplayArea.left < rc.left) {
        _rcDisplayArea.left = rc.left;
    }
    _rcDisplayArea.right = _rcDisplayArea.left + size.cx;
    if (_rcDisplayArea.right > rc.right) {
        _rcDisplayArea.right = rc.right;
    }

    // 优先在下方显示
    if (_rcCurTooltip.bottom + size.cy >= rc.bottom) {
        // 只能在上方显示?
        _isDrawAbove = true;
        if (size.cy >= _rcCurTooltip.top) {
            // 上方也显示不了，则不显示 tooltip 了
            _rcDisplayArea.setEmpty();
            return;
        }
        _rcDisplayArea.top = _rcCurTooltip.top - size.cy;
    } else {
        _isDrawAbove = false;
        _rcDisplayArea.top = _rcCurTooltip.bottom;
    }
    _rcDisplayArea.bottom = _rcDisplayArea.top + size.cy;

    _skinWnd->invalidateRect(&_rcDisplayArea);
}
