#pragma once

#ifndef _ML_WINDOW_H_INC_
#define _ML_WINDOW_H_INC_

#include "../IWindow.h"
#include "Cursor.h"


const int ML_WM_LANGUAGE_CHANGED = (WM_USER + 1212);
const int ML_LANGUAGE_CHANGED_PARAM = 172172;
#define ML_WM_USER                (WM_USER + 1213)

LRESULT CALLBACK BaseWndProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam);

class Window : public IWindow {
public:
    Window();
    virtual ~Window();

public:
    virtual bool create(cstr_t szCaption, int x, int y, int nWidth, int nHeight, Window *pWndParent, uint32_t dwStyle = DS_NOIDLEMSG | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, int nID = ID_UNDEFINE);
    virtual bool createEx(cstr_t szClassName, cstr_t szCaption, int x, int y, int nWidth, int nHeight, Window *pWndParent, uint32_t dwStyle = DS_NOIDLEMSG | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, uint32_t dwExStyle = 0, int nID = ID_UNDEFINE);

    virtual bool createForSkin(cstr_t szClassName, cstr_t szCaption, int x, int y, int nWidth, int nHeight, Window *pWndParent, bool bToolWindow = true, bool bTopmost = false, bool bVisible = true);

    virtual bool destroy();

    void postDestroy();

public:
    //
    // Messages
    //
    virtual void onDropFiles(HDROP hDrop) { }


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

    uint32_t getDlgItemText(int nIDItem, char * szString, int nMaxCount) { return ::getDlgItemText(m_hWnd, nIDItem, szString, nMaxCount); }
    uint32_t getDlgItemText(int nIDItem, string &str);
    bool setDlgItemText(int nIDItem, cstr_t szString) { return ::setDlgItemText(m_hWnd, nIDItem, szString); }
    uint32_t getDlgItemInt(int nIDItem, bool *pTranslated, bool bSigned) { return ::getDlgItemInt(m_hWnd, nIDItem, pTranslated, bSigned); }
    bool setDlgItemInt(int nIDItem, uint32_t uValue, bool bSigned) { return ::setDlgItemInt(m_hWnd, nIDItem, uValue, bSigned); }
    bool enableDlgItem(int nIDItem, bool bEnable) { return tobool(::enableWindow(::getDlgItem(m_hWnd, nIDItem), bEnable)); }
    bool enableWindow(bool bEnable) { return tobool(::enableWindow(m_hWnd, bEnable)); }
    bool showDlgItem(int nIDItem, int nCmdShow) { return tobool(::showWindow(::getDlgItem(m_hWnd, nIDItem), nCmdShow)); }
    Window *getDlgItem(int nIDItem) { return fromHandle(::getDlgItem(m_hWnd, nIDItem)); }
    bool getDlgItemRect(int nIDItem, CRect* lpRect);

    bool setTimer(uint32_t nTimerId, uint32_t nElapse);
    void killTimer(uint32_t nTimerId);

    int getWindowText(char * szString, int nMaxCount) { return ::getWindowText(m_hWnd, szString, nMaxCount); }
    int getWindowText(string &str);
    bool setWindowText(cstr_t szText) { return tobool(::setWindowText(m_hWnd, szText)); }

    bool setWndCursor(Cursor *pCursor);

    bool setFocus();

    void setUseWindowsAppearance(bool isUseWindowAppearance);

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

    CGraphics *getGraphics();
    void releaseGraphics(CGraphics *canvas);

    virtual bool invalidateRect(const CRect* lpRect = nullptr, bool bErase = false);

    void checkButton(int nIDButton, bool bCheck) { CheckDlgButton(m_hWnd, nIDButton, bCheck ? BST_CHECKED : BST_UNCHECKED); }
    bool isButtonChecked(int nIDButton) { return IsDlgButtonChecked(m_hWnd, nIDButton) == BST_CHECKED; }

    bool isSameWnd(Window *pWnd) { return m_hWnd == pWnd->m_hWnd; }

    bool isChild();

    bool isMouseCaptured();

    bool isIconic();

    bool isZoomed() { return tobool(::isZoomed(m_hWnd)); }

    bool isWindow();

    bool isValid() { return m_hWnd != nullptr; }

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
    void sendUserMessage(int nMessageID, LPARAM param);

    virtual void onUserMessage(int nMessageID, LPARAM param) { }

public:
    // Translucency related APIs
    bool                        m_bTranslucencyLayered; // Is following alpha setting enabled?

    // Alpha
    int                         m_nAlpha;
    bool                        m_bClickThrough;

    virtual void setTransparent(uint8_t nAlpha, bool bClickThrough);
    virtual bool isClickThrough() { return m_bClickThrough; }

    bool updateLayeredWindowUsingMemGraph(CRawGraph *canvas);

public:
    static Window *fromHandle(HWND hWnd);
    void attach(HWND hWnd);
    void detach();

    HWND getHandle() {
        return m_hWnd;
    }
    // void SetWndHandle(HWND hWnd);
    // bool sendMessage(uint32_t Msg, WPARAM wParam, LPARAM lParam);
    // bool PostMessage(uint32_t Msg, WPARAM wParam, LPARAM lParam);

    virtual LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

protected:
    HWND                        m_hWnd;
    WndSizeMode                 m_WndSizeMode;

};

#endif // !defined(Window_win32_Window_h)
