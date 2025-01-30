#pragma once

#ifndef _SKIN_TOOL_TOP_INC_
#define _SKIN_TOOL_TOP_INC_

#include "SkinTypes.h"
#include "SkinFontProperty.h"

class CSkinWnd;


class CSkinToolTip {
public:
    CSkinToolTip(CSkinWnd *skinWnd, int timerId);
    virtual ~CSkinToolTip();

    void delToolTip(uint32_t idTooltip);
    void addToolTip(const StringView &text, CRect &rc, uint32_t idTooltip);

    bool setProperty(cstr_t property, cstr_t value);

    void onCreate();
    void onDestroy();

    void onTimer(uint32_t idEvent);

    void onMouseMove(CPoint point);
    void onPaint(CRawGraph *canvas);

    void onKeyDown(uint32_t nChar, uint32_t nFlags) { stopDisplayTooltip(); }
    void onLButtonDown(uint32_t nFlags, CPoint point) { stopDisplayTooltip(); }
    void onLButtonDblClk(uint32_t nFlags, CPoint point) { stopDisplayTooltip(); }
    void onRButtonDown(uint32_t nFlags, CPoint point) { stopDisplayTooltip(); }
    void onMouseDrag(uint32_t nFlags, CPoint point) { stopDisplayTooltip(); }

protected:
    void stopDisplayTooltip();
    void invalidate();

    struct Item {
        int                     x;

    };

    struct Tooltip {
        uint32_t                id;
        CRect                   rc;
        string                  text;
    };
    using VecTooltips = std::vector<Tooltip>;

protected:
    int                         TIMER_ID_TOOLTIP;

    CSkinWnd                    *_skinWnd = nullptr;

    CSkinFontProperty           _fontProperty;     // Default font property.
    CColor                      _clrBg;

    bool                        _isTimerSet = false;
    bool                        _isDisplayTooltip = false;
    bool                        _isDrawAbove = false;
    int64_t                     _timeHideTooltip = 0;
    CRect                       _rcCurTooltip;
    CRect                       _rcDisplayArea;
    string                      _txtCurTooltip;
    VecTooltips                 _tooltips;

    int                         _heightHandle = 10;
    int                         _margin = 5;

};

#endif // !defined(Window_mac_SkinToolTip_h)
