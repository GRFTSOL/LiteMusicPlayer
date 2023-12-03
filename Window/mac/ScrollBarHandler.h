#ifndef Window_mac_ScrollBarHandler_h
#define Window_mac_ScrollBarHandler_h

#pragma once

#include "IScrollBar.h"


class Window;

#define SB_VERT             1

class CScrollBarHandler : public IScrollBar {
public:
    CScrollBarHandler();
    virtual ~CScrollBarHandler();

    virtual void setScrollInfo(int nMin, int nMax, int nPage, int nPos, int nLine = 1, bool bRedraw = true);
    virtual int setScrollPos(int nPos, bool bRedraw = true);

    virtual int getScrollPos() const;
    virtual int getPage() const{ return m_nPage; }
    virtual int getMin() const{ return m_nMin; }
    virtual int getMax() const;
    virtual int getID() const;

    // For CSkinVScrollBar, it will send the scroll notify event to pNotify, or it will send to parent.
    virtual void setScrollNotify(IScrollNotify *pNofity) { }

    virtual bool isEnabled() const { return !m_bDisabled; }
    virtual void disableScrollBar();

    // For windows scroll bar, IScrollBar need to handle scroll codes and set scroll pos of the scroll bar.
    // This was done in WM_VSCROLL or WM_HSCROLL message handler.
    // If pos is changed, return true.
    bool handleScrollCode(uint32_t nSBCode, int nPos);

    void init(Window *pWnd, int nBar = SB_VERT);

    bool isVertBar() { return m_nScrollBar == SB_VERT; }

protected:
    int                         m_nScrollBar;
    bool                        m_bDisabled;
    int                         m_nOneline;

    int                         m_nPage, m_nMin;

};

#endif // !defined(Window_mac_ScrollBarHandler_h)
