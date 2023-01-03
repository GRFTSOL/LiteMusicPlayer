//
//  IWindow.h
//  Mp3Player
//
//  Created by HongyongXiao on 2021/11/13.
//

#pragma once

#ifndef IWindow_hpp
#define IWindow_hpp

#include "WindowTypes.h"
#include "IScrollBar.h"


//
// The Window size state definition
//
enum WndSizeMode {
    WndSizeMode_Minimized,
    WndSizeMode_Maximized,
    WndSizeMode_Normal,
};

struct WindowHandleHolder;

class IWindow {
public:
    //
    // Messages
    //
    IWindow();
    virtual ~IWindow();

    virtual void onActivate(bool bActived) { }
    virtual bool onClose() { return true; }
    virtual void onCommand(uint32_t uID, uint32_t nNotifyCode) { }
    virtual void onContexMenu(int xPos, int yPos) { }
    virtual void onCreate();
    virtual void onDestroy() { }

    virtual void onKeyDown(uint32_t nChar, uint32_t nFlags) { }
    virtual void onKeyUp(uint32_t nChar, uint32_t nFlags) { }
    virtual void onChar(uint32_t nChar) { }
    virtual void onLButtonUp(uint32_t nFlags, CPoint point) { }
    virtual void onLButtonDown(uint32_t nFlags, CPoint point) { }
    virtual void onLButtonDblClk(uint32_t nFlags, CPoint point) { }
    virtual void onRButtonUp( uint32_t nFlags, CPoint point ) { }
    virtual void onRButtonDown( uint32_t nFlags, CPoint point ) { }
    virtual void onMouseMove(CPoint point) { }
    virtual void onMouseDrag(uint32_t nFlags, CPoint point) { }
    virtual void onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) { }

    virtual void onMagnify(float magnification) { }

    virtual void onMove(int x, int y) { }

    virtual void onPaint(CRawGraph *surface, CRect *rc) { }

    virtual void onKillFocus() { }
    virtual void onSetFocus() { }

    virtual void onSize(int cx, int cy) { }
    virtual void onSizeModeChanged(WndSizeMode sizeMode) { }

    virtual void onTimer(uint32_t nIDEvent) { }

    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) { }
    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) { }

    // Language Changed Notification message
    virtual void onLanguageChanged() { }

public:
    CRawGraph *getMemGraphics();

    bool recreateMemGraphics();

    void onResized(int width, int height);

    WindowHandleHolder *getHandleHolder() { return m_handleHolder; }

protected:
    WindowHandleHolder          *m_handleHolder;
    CRawGraph                   *m_pmemGraph;
    CSize                       m_wndSize;

};

#endif // IWindow_hpp
