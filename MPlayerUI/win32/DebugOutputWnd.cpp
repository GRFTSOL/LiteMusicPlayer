#include "DebugOutputWnd.h"


CDebugOutputWnd::CDebugOutputWnd() {

}

CDebugOutputWnd::~CDebugOutputWnd() {

}

bool CDebugOutputWnd::create() {
    bool nRet = Window::create("Debug Trace Message", 0, 0, 600, 300, nullptr, DS_NOIDLEMSG | WS_VISIBLE | WS_MINIMIZEBOX | WS_CAPTION | WS_THICKFRAME);

    if (nRet) {
        CRect rc;
        getClientRect(&rc);
        m_edit.create("", rc.left, rc.top, rc.width(), rc.height(), this);
        m_edit.showWindow(SW_SHOW);
    }
    /*
    HRGN        hRgn;

    hRgn = CreateRectRgn(0, 0, 1, 1);
    int        n = GetWindowRgn(m_hWnd, hRgn);
    DBG_LOG1("Rgn: %d", n);
    if (n == NULLREGION)
    {
        messageOut("NULLREGION");
    }
    else if (n == SIMPLEREGION)
    {
        messageOut("SIMPLEREGION");
    }
    else if (n == COMPLEXREGION)
    {
        messageOut("COMPLEXREGION");
    }
    else if (n == ERROR)
    {
        messageOut("ERROR");
    }
    else 
    {
        messageOut("UNKNOWN");
    }

    if (!::setWindowRgn(m_hWnd, nullptr, true))
    {
        messageOut("set nullptr rgn FAILED!");
    }

    n = GetWindowRgn(m_hWnd, hRgn);
    DBG_LOG1("Rgn: %d", n);
    if (n == NULLREGION)
    {
        messageOut("NULLREGION");
    }
    else if (n == SIMPLEREGION)
    {
        messageOut("SIMPLEREGION");
    }
    else if (n == COMPLEXREGION)
    {
        messageOut("COMPLEXREGION");
    }
    else if (n == ERROR)
    {
        messageOut("ERROR");
    }
    else 
    {
        messageOut("UNKNOWN");
    }*/

    return nRet;
}

void CDebugOutputWnd::onSize(int cx, int cy) {
    if (m_edit.getHandle()) {
        CRect rc;
        getClientRect(&rc);
        m_edit.moveWindow(rc.left, rc.top, rc.width(), rc.height(), true);
    }
}

LRESULT CDebugOutputWnd::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_COPYDATA) {
#define SENDDEBUGMESSAGE    19771212
        COPYDATASTRUCT *data = (COPYDATASTRUCT*)lParam;
        if (data->dwData == SENDDEBUGMESSAGE) {
            int ilen = m_edit.getLength();
            m_edit.setSel(ilen, ilen);
            string str = (cstr_t)data->lpData;
            str += " \r\n";
            m_edit.replaceSel(str.c_str());
        }
        return 0;
    }

    return Window::wndProc(message, wParam, lParam);
}
