//
//  Window.h
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/15/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "../IWindow.h"
#include "Constants.h"


typedef int LPARAM;

class Cursor;

class Window : public IWindow {
public:
    Window();
    virtual ~Window();

    virtual bool createForSkin(cstr_t szClassName, cstr_t szCaption, int x, int y, int nWidth, int nHeight, Window *pWndParent, bool bToolWindow = true, bool bTopmost = false, bool bVisible = true);

    virtual void destroy();

    void postDestroy();

public:
    //
    // Operations
    //
    void activateWindow();


    void showNoActivate();
    void show();
    void hide();
    void minimize();
    void minimizeNoActivate();
    void maximize();
    void restore();

    bool setTimer(uint32_t nTimerId, uint32_t nElapse);
    void killTimer(uint32_t nTimerId);
    
    string getTitle();
    void setTitle(cstr_t szText);

    bool setWndCursor(Cursor *pCursor);

    bool setFocus();

    bool setCapture();
    void releaseCapture();

    void screenToClient(CRect &rc);
    void clientToScreen(CRect &rc);

    void screenToClient(CPoint &pt);
    void clientToScreen(CPoint &pt);

    bool getWindowRect(CRect* lpRect);
    bool getClientRect(CRect* lpRect);

    void setParent(Window *pWndParent);
    Window *getParent();

    virtual void onPaint(CRawGraph *surface, CRect *rcClip);

    virtual bool invalidateRect(const CRect* lpRect = nullptr, bool bErase = false);

    void checkButton(int nIDButton, bool bCheck);
    bool isButtonChecked(int nIDButton);
    
    bool isSameWnd(Window *pWnd);

    bool isChild();

    bool isMouseCaptured();

    bool isIconic();
    bool isZoomed();

    bool isWindow();

    bool isValid();

    bool isVisible();

    bool isTopmost();
    void setTopmost(bool bTopmost);

    bool isToolWindow();
    void setToolWindow(bool bToolWindow);

    bool setForeground();

    void setWindowPos(int x, int y);
    void setWindowPosSafely(int x, int y);

    bool moveWindow(int X, int Y, int nWidth, int nHeight, bool bRepaint = true);

    bool moveWindowSafely(int X, int Y, int nWidth, int nHeight, bool bRepaint = true);

    int messageOut(cstr_t lpText, uint32_t uType = MB_ICONINFORMATION | MB_OK, cstr_t lpCaption = nullptr);

    bool replaceChildPos(int nIDChildSrcPos, Window *pChildNew);

    void postUserMessage(int nMessageID, LPARAM param);

    virtual void onUserMessage(int nMessageID, LPARAM param) { }

public:
    // Translucency related APIs
    bool                    m_bTranslucencyLayered;                // Is following alpha setting enabled?

    // Alpha
    int                     m_nAlpha;
    bool                    m_bClickThrough;

    virtual void setTransparent(uint8_t nAlpha, bool bClickThrough);
    virtual bool isClickThrough() { return m_bClickThrough; }
    
//    bool updateLayeredWindowUsingMemGraph(CRawGraph *canvas);

    bool                    m_bMouseCaptured;
    
    // For Mac
    // void *getHandle();
    void setHasShadow(bool hasShadow);

protected:
    WndSizeMode             m_WndSizeMode;
    Window                  *m_parent;

};
