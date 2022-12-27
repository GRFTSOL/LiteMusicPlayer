#pragma once

#ifndef _HEADER_WIDGETIF_H_
#define _HEADER_WIDGETIF_H_

#include "../Utils/Utils.h"

//
//
//
#ifdef _MAC_OS
#include "mac/Constants.h"


#endif

#define ID_UNDEFINE        (0)

#ifdef _WIN32
#ifndef MK_ALT
#define MK_ALT              0x0080
#endif
#endif

//
// Scroll info define
//
class IScrollNotify;
class IScrollBar {
public:
    virtual void setScrollInfo(int nMin, int nMax, int nPage, int nPos = 0, int nLine = 1, bool bRedraw = true) = 0;
    virtual int setScrollPos(int nPos, bool bRedraw = true) = 0;

    virtual int getScrollPos() const = 0;
    virtual int getPage() const = 0;
    virtual int getMin() const = 0;
    virtual int getMax() const = 0;
    virtual int getID() const = 0;

    // For CSkinVScrollBar, it will send the scroll notify event to pNotify, or it will send to parent.
    virtual void setScrollNotify(IScrollNotify *pNofity) = 0;

    virtual bool isEnabled() const = 0;
    virtual void disableScrollBar() = 0;

    // For windows scroll bar, IScrollBar need to handle scroll codes and set scroll pos of the scroll bar.
    // This was done in WM_VSCROLL or WM_HSCROLL message handler.
    // If pos is changed, return true.
    virtual bool handleScrollCode(uint32_t nSBCode, int nPos) = 0;

};

class IScrollNotify {
public:
    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) = 0;
    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) = 0;

};


#endif // _HEADER_WIDGETIF_H_
