﻿

#include "../WindowTypes.h"
#include "Window.h"
#include "../GfxRaw/RawGraph.h"


#define BASEWNDCLASSNAME    "Window"

#define WM_MOUSEWHEEL       0x020A


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
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
    } else if (uMsg == WM_INITDIALOG) {
        Window *pWnd = (Window *)lParam;
        if (pWnd) {
            pWnd->attach(hWnd);
        }

        // stash pointer to self
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)lParam);
    }

    // pass the messages into the BaseWnd
    Window *pBaseWnd = (Window *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pBaseWnd == nullptr) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
    wc.hCursor = LoadCursor(nullptr, MAKEINTRESOURCE(IDC_ARROW));
    wc.hbrBackground= (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName= szClassName;

    int r = RegisterClassEx(&wc);
    if (r == 0) {
        int res = GetLastError();
        if (res != ERROR_CLASS_ALREADY_EXISTS) {
            // LOG1("Failed to register class, err %d", res);
            return false;
        }
    }

    return true;
}

bool showAsAppWindowNoRefresh(HWND hWnd) {
    uint32_t dwStyleEx;
    dwStyleEx = (uint32_t)GetWindowLong(hWnd, GWL_EXSTYLE);
    if (isFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) || isFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW)) {
        dwStyleEx &= ~(WS_EX_TOOLWINDOW | WS_EX_PALETTEWINDOW);
        dwStyleEx |= WS_EX_APPWINDOW;
        SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
        return true;
    }
    return false;
}

bool showAsToolWindowNoRefresh(HWND hWnd) {
    uint32_t dwStyleEx;
    dwStyleEx = (uint32_t)GetWindowLong(hWnd, GWL_EXSTYLE);
    if (!isFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) && !isFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW)) {
        dwStyleEx |= WS_EX_TOOLWINDOW;// | WS_EX_PALETTEWINDOW;
        dwStyleEx &= ~WS_EX_APPWINDOW;
        SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
        return true;
    }
    return false;
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

bool Window::createEx(cstr_t szClassName, cstr_t szCaption, int x, int y, int nWidth, int nHeight, Window *pWndParent, uint32_t dwStyle, uint32_t dwExStyle, int64_t nID) {
    if (!mLRegisterWndClass(getAppInstance(), szClassName)) {
        return false;
    }

    assert(m_hWnd == nullptr);

    // 创建窗口
    m_hWnd = CreateWindowEx(
        dwExStyle,              // extend styles
        szClassName,            // window class name
        szCaption,              // window title
        dwStyle,                // window style
        x, y,                   // screen position
        nWidth, nHeight,        // width & height of window
        pWndParent ? pWndParent->m_hWnd : nullptr,    // parent window
        (HMENU)nID,             // menu or window id
        getAppInstance(),       // hInstance
        (void *)this);          // this for window creation data

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

void Window::destroy() {
    ::DestroyWindow(m_hWnd);
}

void Window::postDestroy() {
    PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

void Window::onPaint(CRawGraph *canvas, CRect *rcClip) {
}

void Window::activateWindow() {
    uint32_t dwTimeOutPrev = 0;

    SendMessage(m_hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    ::ShowWindow(m_hWnd, SW_SHOW);

    if (!isTopmost()) {
        ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
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
    ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
}

void Window::show() {
    ::ShowWindow(m_hWnd, SW_SHOW);
}

void Window::hide() {
    ::ShowWindow(m_hWnd, SW_HIDE);
}

void Window::minimize() {
    ::ShowWindow(m_hWnd, SW_MINIMIZE);
}

void Window::maximize() {
    ::ShowWindow(m_hWnd, SW_MAXIMIZE);
}

void Window::restore() {
    if (isZoomed()) {
        maximize();
    } else {
        minimize();
    }
}

void Window::minimizeNoActivate() {
    ::ShowWindow(m_hWnd, SW_SHOWMINNOACTIVE);
}

uint32_t Window::getDlgItemText(int nIDItem, string &str) {
    int nLen;

    nLen = SendDlgItemMessage(m_hWnd, nIDItem, WM_GETTEXTLENGTH, 0, 0);
    if (nLen > 0) {
        str.resize(nLen + 1);
        nLen = ::GetDlgItemText(m_hWnd, nIDItem, (char *)str.c_str(), str.capacity());

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
    HWND hWnd = ::GetDlgItem(m_hWnd, nIDItem);
    if (!hWnd) {
        return false;
    }

    return tobool(::GetWindowRect(hWnd, lpRect));
}

void Window::screenToClient(CRect &rc) {
    ::ScreenToClient(m_hWnd, (LPPOINT)&rc.left);
    ::ScreenToClient(m_hWnd, (LPPOINT)&rc.right);
}

void Window::clientToScreen(CRect &rc) {
    ::ClientToScreen(m_hWnd, (LPPOINT)&rc.left);
    ::ClientToScreen(m_hWnd, (LPPOINT)&rc.right);
}

void Window::screenToClient(CPoint &pt) {
    ::ScreenToClient(m_hWnd, &pt);
}

void Window::clientToScreen(CPoint &pt) {
    ::ClientToScreen(m_hWnd, &pt);
}

bool Window::getWindowRect(CRect* lpRect) {
    return tobool(::GetWindowRect(m_hWnd, lpRect));
}

bool Window::getClientRect(CRect* lpRect) {
    return tobool(::GetClientRect(m_hWnd, lpRect));
}

void Window::setCursor(Cursor *cursor) {
    setWndCursor(cursor);
}

bool Window::setTimer(uint32_t nTimerId, uint32_t nElapse) {
    return tobool(::SetTimer(m_hWnd, nTimerId, nElapse, nullptr));
}

void Window::killTimer(uint32_t nTimerId) {
    ::KillTimer(m_hWnd, nTimerId);
}

string Window::getTitle() {
	string str;
    int len = SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0);
    if (len > 0) {
        str.resize(len + 1);
        len = ::GetWindowText(m_hWnd, (char *)str.c_str(), str.capacity());

        str.resize(len);
    }

    return str;
}

bool Window::setWndCursor(Cursor *pCursor) {
    SetClassLongPtr(m_hWnd, GCLP_HCURSOR, (LONG_PTR)pCursor->getHandle());
    return true;
}

bool Window::setFocus() {
    return ::SetFocus(m_hWnd) != nullptr;
}

bool Window::setCapture() {
    ::SetCapture(m_hWnd);

    return true;
}

void Window::releaseCapture() {
    ::ReleaseCapture();
}

bool Window::invalidateRect(const CRect *lpRect, bool bErase) {
    return ::InvalidateRect(m_hWnd, lpRect, bErase);
}

bool Window::isChild() {
    return ::GetParent(m_hWnd) != NULL;
}

bool Window::isMouseCaptured() {
    return GetCapture() == m_hWnd;
}

bool Window::isIconic() {
    return tobool(::IsIconic(m_hWnd));
}

bool Window::isWindow() {
    return tobool(::IsWindow(m_hWnd));
}

bool Window::isVisible() {
    return tobool(::IsWindowVisible(m_hWnd));
}

HWND getParentOrSelf(HWND wnd) {
    HWND parent = wnd;
    while (parent) {
        auto tmp = GetParent(parent);
        if (!tmp) {
            break;
        }
        parent = tmp;
    }

    return parent;
}

bool Window::isTopmost() {
    auto hWnd = getParentOrSelf(m_hWnd);

    uint32_t dwStyleEx = (uint32_t)::GetWindowLong(hWnd, GWL_EXSTYLE);

    return ((dwStyleEx & WS_EX_TOPMOST) == WS_EX_TOPMOST);
}

void Window::setTopmost(bool bTopmost) {
    auto hWnd = getParentOrSelf(m_hWnd);

    uint32_t dwStyleEx;
    if (bTopmost) {
        dwStyleEx = (uint32_t)::GetWindowLong(hWnd, GWL_EXSTYLE);
        dwStyleEx |= WS_EX_TOPMOST;
        ::SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
        ::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
    } else {
        dwStyleEx = (uint32_t)::GetWindowLong(hWnd, GWL_EXSTYLE);
        dwStyleEx &= ~WS_EX_TOPMOST;
        ::SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
        ::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
    }
}

bool Window::isToolWindow() {
    DWORD dwStyleEx = (DWORD)GetWindowLong(m_hWnd, GWL_EXSTYLE);

    return (dwStyleEx & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW/* | WS_EX_PALETTEWINDOW*/;
}

BOOL showAsToolWindow(HWND hWnd) {
    DWORD dwStyleEx;
    dwStyleEx = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
    if (!isFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) && !isFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW)) {
        // not showed in task bar, so show it
        ShowWindow(hWnd, SW_HIDE);
        dwStyleEx |= WS_EX_TOOLWINDOW;// | WS_EX_PALETTEWINDOW;
        dwStyleEx &= ~WS_EX_APPWINDOW;
        SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
        ShowWindow(hWnd, SW_SHOW);
        return TRUE;
    }
    return FALSE;
}

// add it's WS_EX_APPWINDOW AND remove WS_EX_TOOLWINDOW
BOOL showAsAppWindow(HWND hWnd) {
    DWORD dwStyleEx;
    dwStyleEx = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
    if (isFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) || isFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW)) {
        // not showed in task bar, so show it
        ShowWindow(hWnd, SW_HIDE);
        dwStyleEx &= ~(WS_EX_TOOLWINDOW | WS_EX_PALETTEWINDOW);
        dwStyleEx |= WS_EX_APPWINDOW;
        SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
        ShowWindow(hWnd, SW_SHOW);
        return TRUE;
    }
    return FALSE;
}

BOOL isChildWnd(HWND hWnd) {
    DWORD	dwStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
    return (dwStyleEx & WS_CHILD) == WS_CHILD;
}

BOOL HasCaption(HWND hWnd)
{
    DWORD	dwStyleEx;

    dwStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
    return (dwStyleEx & WS_CAPTION) == WS_CAPTION;
}

void RemoveCaption(HWND	hWnd)
{
    DWORD	dwStyleEx;

    dwStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
    dwStyleEx &= ~WS_CAPTION;
    ::SetWindowLong(hWnd, GWL_STYLE, dwStyleEx);
    ::SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void AddCaption(HWND	hWnd)
{
    DWORD	dwStyleEx;

    dwStyleEx = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
    if ((dwStyleEx & WS_CAPTION) != WS_CAPTION)
    {
        dwStyleEx |= WS_CAPTION;
        ::SetWindowLong(hWnd, GWL_STYLE, dwStyleEx);
        ::SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

BOOL moveWindowSafely(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint) {
    if (isChildWnd(hWnd)) {
        POINT	pt;
        HWND	hWndParent;
        hWndParent = GetParent(hWnd);
        if (hWndParent) {
            pt.x = X;
            pt.y = Y;
            ScreenToClient(hWndParent, &pt);
            X = pt.x;
            Y = pt.y;
        }
    }

    BOOL bHasCap = HasCaption(hWnd);
    if (bHasCap)
        RemoveCaption(hWnd);

    BOOL bRet = MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);

    if (bHasCap)
        AddCaption(hWnd);

    return bRet;
}

BOOL IsLayeredWndSupported()
{
    OSVERSIONINFO		version;

    memset(&version, 0, sizeof(version));
    version.dwOSVersionInfoSize = sizeof(version);

    if (!GetVersionEx(&version))
    {
        //		LOGOUT1(LOG_LVL_ERROR, _T("GetVersionEx FAILED! Error Id: %d"), GetLastError);
        return FALSE;
    }

    // Major version: 5, windows 2000
    if (version.dwMajorVersion < 5)
        return FALSE;

    return TRUE;
}

//	COMMENT:
//		sets the opacity and transparency color key of a layered window
//	INPUT:
//		crKey,		specifies the color key
//		bAlpha,		value for the blend function, When bAlpha is 0, 
//					the window is completely transparent. 
//					When bAlpha is 255, the window is opaque. 
//		dwFlags		Specifies an action to take. This parameter can be one or 
//					more of the following values. Value Meaning 
//					LWA_COLORKEY Use crKey as the transparency color.  
//					LWA_ALPHA Use bAlpha to determine the opacity of the layered window 
BOOL SetLayeredWindow(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags) {
    DWORD dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    dwStyle |= WS_EX_LAYERED;
    SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);

    return SetLayeredWindowAttributes(hWnd, crKey, bAlpha, dwFlags);
}

void UnSetLayeredWindow(HWND hWnd) {
    DWORD dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    dwStyle &= ~WS_EX_LAYERED;
    SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);
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
    ::SetWindowPos(m_hWnd, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);
}

void Window::setWindowPosSafely(int x, int y) {
    if (isChild()) {
        HWND hWndParent;
        hWndParent = ::GetParent(m_hWnd);
        if (hWndParent) {
            CPoint pt;
            pt.x = x;
            pt.y = y;
            ::ScreenToClient(hWndParent, &pt);
            x = pt.x;
            y = pt.y;
        }
    }
    ::SetWindowPos(m_hWnd, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);
}

bool Window::moveWindow(int X, int Y, int nWidth, int nHeight, bool bRepaint) {
    return tobool(::MoveWindow(m_hWnd, X, Y, nWidth, nHeight, bRepaint));
}

bool Window::moveWindowSafely(int X, int Y, int nWidth, int nHeight, bool bRepaint) {
    return tobool(::moveWindowSafely(m_hWnd, X, Y, nWidth, nHeight, bRepaint));
}

int Window::messageOut(cstr_t lpText, uint32_t uType, cstr_t lpCaption) {
    char szCap[512] = { 0 };
    if (lpCaption == nullptr) {
        emptyStr(szCap);
        GetWindowText(m_hWnd, szCap, CountOf(szCap));
        lpCaption = szCap;
    }

    return MessageBox(m_hWnd, lpText, lpCaption, uType);
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
    SendMessage(m_hWnd, ML_WM_USER, nMessageID, param);
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
        onCreate();
    case WM_CLOSE:
        if (onClose()) {
            destroy();
        }
        return 0;
    case WM_COMMAND:
        {
            uint32_t uId = LOWORD(wParam);
            onCommand(uId);
        }
        break;
    case WM_CONTEXTMENU:
        {
            //            int xPos = LOWORD(lParam);
            //            int yPos = HIWORD(lParam);
            if (lParam == -1) {
                CPoint pt;
                GetCursorPos(&pt);
                onContexMenu(pt.x, pt.y);
            } else {
                onContexMenu(LOWORD(lParam), HIWORD(lParam));
            }
        }
        break;
    case WM_DESTROY:
        {
            onDestroy();
            if (m_hWnd) {
                SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG)0);
                DefWindowProc(m_hWnd, message, wParam, lParam);
                m_hWnd = nullptr;
            }
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
            onVScroll(LOWORD(wParam), HIWORD(wParam), nullptr);
        }
        break;
    case WM_TIMER:
        onTimer((uint32_t)wParam);
        break;
    default:
        return DefWindowProc(m_hWnd, message, wParam, lParam);
    }
    return 0;
}


void Window::setTransparent(uint8_t nAlpha, bool bClickThrough) {
    // If it's child old, ignore this.
    if (::GetParent(m_hWnd)) {
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
        ::SetLayeredWindow(m_hWnd, clrTrans, nAlpha, dwFlags);
    } else {
        ::UnSetLayeredWindow(m_hWnd);
    }
}

bool Window::updateLayeredWindowUsingMemGraph(CRawGraph *canvas) {
    HDC hdc = GetDC(m_hWnd);
    HDC hdcSrc = canvas->getHandle();

    DWORD dwStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
    DWORD dwStyleNew = dwStyle | WS_EX_LAYERED;
    if (isClickThrough()) {
        dwStyleNew |= WS_EX_TRANSPARENT;
    } else {
        dwStyleNew &= ~WS_EX_TRANSPARENT;
    }
    if (dwStyle != dwStyleNew) {
        SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyleNew);
    }

    CRect rcWnd;
    getWindowRect(&rcWnd);

    BLENDFUNCTION blend;
    blend.AlphaFormat = AC_SRC_ALPHA;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = m_nAlpha;

    CPoint ptSrc, ptDest(rcWnd.left, rcWnd.top);
    CSize sizeWnd(rcWnd.width(), rcWnd.height());

    bool bRet = UpdateLayeredWindow(m_hWnd, hdc, &ptDest, &sizeWnd, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);

    ReleaseDC(m_hWnd, hdc);

    return bRet;
}

void Window::attach(HWND hWnd) {
    m_hWnd = hWnd;
}

void Window::detach() {
    m_hWnd = nullptr;
}

WindowHandle Window::getHandle() {
    return (WindowHandle)m_hWnd;
}
