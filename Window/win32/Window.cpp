

#include "BaseWnd.h"


#define BASEWNDCLASSNAME    "Window"

#define WM_MOUSEWHEEL       0x020A

CMLHandleMap<HWND, Window> g_mapWnd;


LRESULT CALLBACK BaseWndProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
    // fetch out the Window *
    if (uMsg == WM_CREATE || uMsg == WM_NCCREATE) {
        CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
        assert(cs->lpCreateParams != nullptr);

        // stash pointer to self
        Window *pWnd = (Window *)cs->lpCreateParams;
        if (pWnd) {
            pWnd->attach(hWnd);
        }
        SetWindowLong(hWnd, GWL_USERDATA, (LONG)(cs->lpCreateParams));
    } else if (uMsg == WM_INITDIALOG) {
        Window *pWnd = (Window *)lParam;
        if (pWnd) {
            pWnd->attach(hWnd);
        }

        // stash pointer to self
        SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);
    }

    // pass the messages into the BaseWnd
    Window *pBaseWnd = (Window *)GetWindowLong(hWnd, GWL_USERDATA);

    if (pBaseWnd == nullptr) {
        return defWindowProc(hWnd, uMsg, wParam, lParam);
    } else {
        // assert(pBaseWnd->getHandle());
        return pBaseWnd->wndProc(uMsg, wParam, lParam);
    }
}

bool mLRegisterWndClass(HINSTANCE hInstance, cstr_t szClassName) {
    WNDCLASSEX wc;

    wc.cbSize = sizeof(wc);

    if (GetClassInfoEx(hInstance, szClassName, &wc)) {
        return true;
    }

    // regiszter pane class
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = (WNDPROC)BaseWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = nullptr;
    wc.hIconSm = nullptr;
    wc.hCursor = loadCursor(nullptr, MAKEINTRESOURCE(IDC_ARROW));
    wc.hbrBackground= (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName= szClassName;

    int r = registerClassEx(&wc);
    if (r == 0) {
        int res = getLastError();
        if (res != ERROR_CLASS_ALREADY_EXISTS) {
            // LOG1("Failed to register class, err %d", res);
            return false;
        }
    }

    return true;
}



Window::Window() {
    m_hWnd = nullptr;
    m_WndSizeMode = (WndSizeMode)-1;
}

Window::~Window() {

}


bool Window::create(cstr_t szCaption, int x, int y, int nWidth, int nHeight, Window *pWndParent, uint32_t dwStyle, int nID) {
    return createEx(BASEWNDCLASSNAME, szCaption, x, y, nWidth, nHeight, pWndParent, dwStyle, nID);
}

bool Window::createEx(cstr_t szClassName, cstr_t szCaption, int x, int y, int nWidth, int nHeight, Window *pWndParent, uint32_t dwStyle, uint32_t dwExStyle, int nID) {
    if (!mLRegisterWndClass(getAppInstance(), szClassName)) {
        return false;
    }

    assert(m_hWnd == nullptr);

    // 创建窗口
    m_hWnd = CreateWindowEx(
        dwExStyle,                        // extend styles
        szClassName,            // window class name
        szCaption,                 // window title
        dwStyle,                // window style
        x, y,                    // screen position
        nWidth, nHeight,        // width & height of window
        pWndParent ? pWndParent->m_hWnd : nullptr,    // parent window
        (HMENU)nID,                    // menu or window id
        getAppInstance(),        // hInstance
        (void *)this);            // this for window creation data

    return m_hWnd != nullptr;
}

bool Window::createForSkin(cstr_t szClassName, cstr_t szCaption, int x, int y, int nWidth, int nHeight, Window *pWndParent, bool bToolWindow, bool bTopmost, bool bVisible) {
    uint32_t dwStyle = DS_NOIDLEMSG | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX;
    uint32_t dwExStyle = 0;
    if (bTopmost) {
        dwExStyle |= WS_EX_TOPMOST;
    }
    if (bToolWindow) {
        dwExStyle |= WS_EX_TOOLWINDOW;
    }
    if (bVisible) {
        dwStyle |= WS_VISIBLE;
    }

    // WS_THICKFRAME will cause window flashing on vista
    if (!createEx(szClassName, szCaption,
        x, y, nWidth, nHeight,
        pWndParent,
        dwStyle, dwExStyle)) {
        return false;
    }

    return true;
}

bool Window::destroy() {
    return tobool(::destroyWindow(m_hWnd));
}

void Window::postDestroy() {
    PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

bool Window::onClose() {
    return true;
}

bool Window::onCreate() {
    onLanguageChanged();

    return true;
}

void Window::onDestroy() {
    if (m_hWnd) {
        SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)0);
        defWindowProc(m_hWnd, WM_DESTROY, 0, 0);
        m_hWnd = nullptr;
    }
}

void Window::onPaint(CGraphics *canvas, CRect &rcClip) {
}

void Window::onLanguageChanged() {
}

void Window::activateWindow() {
    uint32_t dwTimeOutPrev = 0;

    sendMessage(m_hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    ::showWindow(m_hWnd, SW_SHOW);

    if (!isTopmost()) {
        ::setWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        ::setWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    }
    // save old
    SystemParametersInfo(0x2000, 0, (void *)&dwTimeOutPrev, 0);
    // set new
    SystemParametersInfo(0x2001, 0, (void *)0, SPIF_SENDWININICHANGE);

    if (::GetForegroundWindow() != m_hWnd) {
        SetForegroundWindow(m_hWnd);
    }
    if (IsWindowEnabled(m_hWnd)) {
        SetActiveWindow(m_hWnd);
    }

    // restore old
    SystemParametersInfo(0x2001, dwTimeOutPrev, (void *)0, SPIF_SENDWININICHANGE);
}

void Window::showNoActivate() {
    return ::showWindow(m_hWnd, SW_SHOWNOACTIVATE);
}

void Window::show() {
    return ::showWindow(m_hWnd, SW_SHOW);
}

void Window::hide() {
    return ::showWindow(m_hWnd, SW_HIDE);
}

void Window::minimize() {
    return ::showWindow(m_hWnd, SW_MINIMIZE);
}

void Window::maximize() {
    return ::showWindow(m_hWnd, SW_MAXIMIZE);
}

void Window::restore() {
    if (isZoomed()) {
        maximize();
    } else {
        minimize();
    }
}

void Window::minimizeNoActivate() {
    return ::showWindow(m_hWnd, SW_SHOWMINNOACTIVE);
}

bool Window::showWindow(int nCmdShow) {
    return ::showWindow(m_hWnd, nCmdShow);
}

uint32_t Window::getDlgItemText(int nIDItem, string &str) {
    int nLen;

    nLen = SendDlgItemMessage(m_hWnd, nIDItem, WM_GETTEXTLENGTH, 0, 0);
    if (nLen > 0) {
        str.resize(nLen + 1);
        nLen = ::getDlgItemText(m_hWnd, nIDItem, (char *)str.c_str(), str.capacity());

        // Under certain conditions, the defWindowProc function returns
        // a value that is larger than the actual length of the text
        // So, must call str.resize again.
        str.resize(nLen);
    } else {
        str.resize(0);
    }

    return nLen;
}

bool Window::getDlgItemRect(int nIDItem, CRect* lpRect) {
    HWND hWnd = ::getDlgItem(m_hWnd, nIDItem);
    if (!hWnd) {
        return false;
    }

    return tobool(::getWindowRect(hWnd, lpRect));
}

void Window::screenToClient(CRect &rc) {
    ::screenToClient(m_hWnd, (LPPOINT)&rc.left);
    ::screenToClient(m_hWnd, (LPPOINT)&rc.right);
}

void Window::clientToScreen(CRect &rc) {
    ::clientToScreen(m_hWnd, (LPPOINT)&rc.left);
    ::clientToScreen(m_hWnd, (LPPOINT)&rc.right);
}

void Window::screenToClient(CPoint &pt) {
    ::screenToClient(m_hWnd, &pt);
}

void Window::clientToScreen(CPoint &pt) {
    ::clientToScreen(m_hWnd, &pt);
}

bool Window::getWindowRect(CRect* lpRect) {
    return tobool(::getWindowRect(m_hWnd, lpRect));
}

bool Window::getClientRect(CRect* lpRect) {
    return tobool(::getClientRect(m_hWnd, lpRect));
}

void Window::setParent(Window *pWndParent) {
    if (pWndParent) {
        uint32_t dwStyle;
        dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
        SetWindowLong(m_hWnd, GWL_STYLE, dwStyle | WS_CHILD);
        ::setParent(m_hWnd, pWndParent->getHandle());
    } else {
        uint32_t dwStyle;
        dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
        SetWindowLong(m_hWnd, GWL_STYLE, dwStyle & ~WS_CHILD);
        ::setParent(m_hWnd, nullptr);
    }
}

Window *Window::getParent() {
    return fromHandle(::getParent(m_hWnd));
}

bool Window::setTimer(uint32_t nTimerId, uint32_t nElapse) {
    return tobool(::setTimer(m_hWnd, nTimerId, nElapse, nullptr));
}

void Window::killTimer(uint32_t nTimerId) {
    ::killTimer(m_hWnd, nTimerId);
}

int Window::getWindowText(string &str) {
    int nLen;

    nLen = sendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0);
    if (nLen > 0) {
        str.resize(nLen + 1);
        nLen = ::getWindowText(m_hWnd, (char *)str.c_str(), str.capacity());

        // Under certain conditions, the defWindowProc function returns
        // a value that is larger than the actual length of the text
        // So, must call str.resize again.
        str.resize(nLen);
    } else {
        str.resize(0);
    }

    return nLen;
}

bool Window::setWndCursor(Cursor *pCursor) {
    SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)pCursor->getHandle());
    return true;
}

bool Window::setFocus() {
    return ::setFocus(m_hWnd) != nullptr;
}

void Window::setUseWindowsAppearance(bool isUseWindowAppearance) {
    if (isUseWindowAppearance) {
        // Use windows appearance, set region to nullptr, windows xp will auto apply the system region.
        ::SetWindowRgn(m_hWnd, nullptr, true);
    } else {
        HRGN rgn = ::CreateRectRgn(0, 0, m_wndSize.cx, m_wndSize.cy);
        ::SetWindowRgn(m_hWnd, rgn, true);
    }
}

bool Window::setCapture() {
    ::setCapture(m_hWnd);

    return true;
}

void Window::releaseCapture() {
    ::releaseCapture();
}

CGraphics *Window::getGraphics() {
    HDC hdc;
    CGraphics *canvas;

    hdc = GetDC(m_hWnd);

    canvas = new CGraphics;
    canvas->attach(hdc);

    return canvas;
}

void Window::releaseGraphics(CGraphics *canvas) {
    ReleaseDC(m_hWnd, canvas->getHandle());

    delete canvas;
}

bool Window::invalidateRect(const CRect *lpRect, bool bErase) {
    return ::invalidateRect(m_hWnd, lpRect, bErase);
}

bool Window::isChild() {
    return tobool(::isChildWnd(m_hWnd));
}

bool Window::isMouseCaptured() {
    return getCapture() == m_hWnd;
}

bool Window::isIconic() {
    return tobool(::isIconic(m_hWnd));
}

bool Window::isWindow() {
    return tobool(::isWindow(m_hWnd));
}

bool Window::isVisible() {
    return tobool(::IsWindowVisible(m_hWnd));
}

bool Window::isTopmost() {
    uint32_t dwStyleEx;

    dwStyleEx = (uint32_t)::GetWindowLong(m_hWnd, GWL_EXSTYLE);

    return ((dwStyleEx & WS_EX_TOPMOST) == WS_EX_TOPMOST);
}

void Window::setTopmost(bool bTopmost) {
    uint32_t dwStyleEx;
    if (bTopmost) {
        dwStyleEx = (uint32_t)::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        dwStyleEx |= WS_EX_TOPMOST;
        ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyleEx);
        ::setWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
    } else {
        dwStyleEx = (uint32_t)::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        dwStyleEx &= ~WS_EX_TOPMOST;
        ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyleEx);
        ::setWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
    }
}

bool Window::isToolWindow() {
    return tobool(::isToolWindow(m_hWnd));
}

void Window::setToolWindow(bool bToolWindow) {
    if (bToolWindow) {
        showAsToolWindow(m_hWnd);
    } else {
        showAsAppWindow(m_hWnd);
    }
}

bool Window::setForeground() {
    return tobool(::SetForegroundWindow(m_hWnd));
}

void Window::setWindowPos(int x, int y) {
    ::setWindowPos(m_hWnd, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);
}

void Window::setWindowPosSafely(int x, int y) {
    if (isChild()) {
        HWND hWndParent;
        hWndParent = ::getParent(m_hWnd);
        if (hWndParent) {
            CPoint pt;
            pt.x = x;
            pt.y = y;
            ::screenToClient(hWndParent, &pt);
            x = pt.x;
            y = pt.y;
        }
    }
    ::setWindowPos(m_hWnd, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);
}

bool Window::moveWindow(int X, int Y, int nWidth, int nHeight, bool bRepaint) {
    return tobool(::moveWindow(m_hWnd, X, Y, nWidth, nHeight, bRepaint));
}

bool Window::moveWindowSafely(int X, int Y, int nWidth, int nHeight, bool bRepaint) {
    return tobool(::moveWindowSafely(m_hWnd, X, Y, nWidth, nHeight, bRepaint));
}

int Window::messageOut(cstr_t lpText, uint32_t uType, cstr_t lpCaption) {
    char szCap[512];
    if (lpCaption == nullptr) {
        emptyStr(szCap);
        getWindowText(szCap, CountOf(szCap));
        lpCaption = szCap;
    }

    return messageBox(m_hWnd, lpText, lpCaption, uType);
}

bool Window::replaceChildPos(int nIDChildSrcPos, Window *pChildNew) {
    CRect rcPos;

    getDlgItemRect(nIDChildSrcPos, &rcPos);
    screenToClient(rcPos);

    pChildNew->moveWindow(rcPos.left, rcPos.top, rcPos.width(), rcPos.height(), true);

    return true;
}

void Window::postUserMessage(int nMessageID, LPARAM param) {
    PostMessage(m_hWnd, ML_WM_USER, nMessageID, param);
}

void Window::sendUserMessage(int nMessageID, LPARAM param) {
    sendMessage(m_hWnd, ML_WM_USER, nMessageID, param);
}

LRESULT Window::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
    assert(m_hWnd);
    switch (message) {
    case ML_WM_LANGUAGE_CHANGED:
        onLanguageChanged();
        break;
    case ML_WM_USER:
        onUserMessage(wParam, lParam);
        break;
    case WM_ACTIVATE:
        {
            uint16_t wActived;
            // bool        bMinimized;
            // HWND        hWndOld;

            wActived = LOWORD(wParam);
            // bMinimized = HIWORD(wParam);
            // hWndOld = (HWND)lParam;

            onActivate(tobool(wActived));
        }
        break;
    case WM_CREATE:
        if (onCreate()) {
            return 0;
        } else {
            return -1;
        }
    case WM_CLOSE:
        if (onClose()) {
            destroy();
        }
        return 0;
    case WM_COMMAND:
        {
            uint32_t uId = LOWORD(wParam);
            uint32_t uEvent = HIWORD(wParam);
            onCommand(uId, uEvent);
        }
        break;
    case WM_CONTEXTMENU:
        {
            //            int xPos = LOWORD(lParam);
            //            int yPos = HIWORD(lParam);
            if (lParam == -1) {
                CPoint pt;
                getCursorPos(&pt);
                onContexMenu(pt.x, pt.y);
            } else {
                onContexMenu(LOWORD(lParam), HIWORD(lParam));
            }
        }
        break;
    case WM_PAINT:
        {
            CRect rcClip;
            if (!::getUpdateRect(m_hWnd, &rcClip, false)) {
                return 0;
            }

            PAINTSTRUCT ps;
            HDC hdc;
            hdc = BeginPaint(m_hWnd, &ps);
            CGraphics *canvas;

            canvas = new CGraphics;
            canvas->attach(hdc);

            onPaint(canvas, rcClip);

            delete canvas;

            EndPaint(m_hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        {
            onDestroy();
            if (m_hWnd) {
                SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)0);
                defWindowProc(m_hWnd, message, wParam, lParam);
                m_hWnd = nullptr;
            }
            // g_mapWnd.remove(m_hWnd);
        }
        return 0;
    case WM_DROPFILES:
        onDropFiles((HDROP)wParam);
        break;
    case WM_HSCROLL:
        onHScroll(LOWORD(wParam), HIWORD(wParam), nullptr);
        break;
    case WM_KEYDOWN:
        onKeyDown(wParam, lParam);
        break;
    case WM_KEYUP:
        onKeyUp(wParam, lParam);
        break;
    case WM_CHAR:
        //    case WM_UNICHAR:
        onChar(wParam);
        break;
    case WM_LBUTTONDOWN:
        onLButtonDown((uint32_t)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        break;
    case WM_LBUTTONDBLCLK:
        onLButtonDblClk((uint32_t)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        break;
    case WM_LBUTTONUP:
        onLButtonUp((uint32_t)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        break;
    case WM_RBUTTONDOWN:
        onRButtonDown((uint32_t)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        break;
    case WM_RBUTTONUP:
        onRButtonUp((uint32_t)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        break;
    case WM_MOUSEMOVE:
        {
            uint32_t nFlags = uint32_t(wParam);
            if (nFlags & MK_LBUTTON) {
                onMouseDrag((uint32_t)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
            } else {
                onMouseMove(CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
            }
        }
        break;
    case WM_MOUSEWHEEL:
        {
            onMouseWheel(short(HIWORD(wParam)), LOWORD(wParam), CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        }
        break;
    case WM_MOVE:
        onMove(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_KILLFOCUS:
        onKillFocus();
        break;
    case WM_SETFOCUS:
        onSetFocus();
        break;
    case WM_SIZE:
        if (wParam == SIZE_RESTORED) {
            onResized(LOWORD(lParam), HIWORD(lParam));
            if (m_WndSizeMode != WndSizeMode_Normal) {
                m_WndSizeMode = WndSizeMode_Normal;
                onSizeModeChanged(m_WndSizeMode);
            }
        } else if (wParam == SIZE_MINIMIZED) {
            m_WndSizeMode = WndSizeMode_Minimized;
            onSizeModeChanged(m_WndSizeMode);
        } else if (wParam == SIZE_MAXIMIZED) {
            onResized(LOWORD(lParam), HIWORD(lParam));
            m_WndSizeMode = WndSizeMode_Maximized;
            onSizeModeChanged(m_WndSizeMode);
        }
        break;
    case WM_VSCROLL:
        {
            // CSkinVScrollBarOSStyle
            if (lParam) {
                IScrollNotify *pNotify;
                pNotify = (IScrollNotify *)GetWindowLong((HWND)lParam, GWL_USERDATA);
                if (pNotify) {
                    pNotify->onVScroll(LOWORD(wParam), HIWORD(wParam), nullptr);
                    return 0;
                }
            }
            onVScroll(LOWORD(wParam), HIWORD(wParam), nullptr);
        }
        break;
    case WM_TIMER:
        onTimer((uint32_t)wParam);
        break;
    default:
        return defWindowProc(m_hWnd, message, wParam, lParam);
    }
    return 0;
}


void Window::setTransparent(uint8_t nAlpha, bool bClickThrough) {
    // If it's child old, ignore this.
    if (::getParent(m_hWnd)) {
        return;
    }

    m_nAlpha = nAlpha;
    m_bClickThrough = bClickThrough;

    if (m_bTranslucencyLayered) {
        invalidateRect(nullptr, true);
        return;
    }

    COLORREF clrTrans = RGB(0, 0, 0);
    uint32_t dwFlags = 0;

    if (nAlpha < 255) {
        dwFlags = LWA_ALPHA;
    }

    /*    if (bTransparentClr)
    {
        clrTrans = clrTransparent;
        dwFlags |= LWA_COLORKEY;
    }*/

    if (isClickThrough()) {
        SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
    } else {
        SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
    }

    if (dwFlags != 0 || (isClickThrough())) {
        ::setLayeredWindow(m_hWnd, clrTrans, nAlpha, dwFlags);
    } else {
        ::unSetLayeredWindow(m_hWnd);
    }
}

bool Window::updateLayeredWindowUsingMemGraph(CRawGraph *canvas) {
    HDC hdc, hdcSrc;
    CRect rcWnd;
    CPoint ptSrc, ptDest;
    SIZE sizeWnd;
    uint32_t dwStyle, dwStyleNew;
    BLENDFUNCTION blend;

    hdc = GetDC(m_hWnd);

    hdcSrc = canvas->getHandle();

    dwStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
    dwStyleNew = dwStyle | WS_EX_LAYERED;
    if (isClickThrough()) {
        dwStyleNew |= WS_EX_TRANSPARENT;
    } else {
        dwStyleNew &= ~WS_EX_TRANSPARENT;
    }
    if (dwStyle != dwStyleNew) {
        SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyleNew);
    }

    getWindowRect(&rcWnd);

    blend.AlphaFormat = AC_SRC_ALPHA;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = m_nAlpha;

    ptSrc.x = ptSrc.y = 0;
    ptDest.x = rcWnd.left;
    ptDest.y = rcWnd.top;
    sizeWnd.cx = rcWnd.width();
    sizeWnd.cy = rcWnd.height();

    bool bRet = UpdateLayeredWindow(m_hWnd, hdc, &ptDest, &sizeWnd, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);
    //bool bRet = UpdateLayeredWindow(m_hWnd, nullptr, nullptr, nullptr, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);

    ReleaseDC(m_hWnd, hdc);

    return bRet;
}

Window *Window::fromHandle(HWND hWnd) {
    assert(hWnd != nullptr);
    // if (hWnd == nullptr)
    //     return nullptr;
    return g_mapWnd.fromhandle(hWnd);
}

void Window::attach(HWND hWnd) {
    m_hWnd = hWnd;
}

void Window::detach() {
    m_hWnd = nullptr;
}
