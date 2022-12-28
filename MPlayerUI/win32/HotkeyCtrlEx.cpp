#include "HotkeyCtrlEx.h"


bool isExtendedKey(uint32_t nVk) {
    return nVk >= VK_SPACE && nVk <= VK_HELP;
}

bool isKeyPressed(int nKey) {
    return (GetKeyState(nKey) & 0x8000) == 0x8000;
}

void formatHotkeyText(string &strText, uint32_t nVirtKey, uint32_t fsModifiers) {
    char szKeyName[256];
    uint32_t nScanCode;

    strText.resize(0);

    emptyStr(szKeyName);
    nScanCode = MapVirtualKey(nVirtKey, 0) << 16;
    if (isExtendedKey(nVirtKey)) {
        nScanCode |= 1 << 24;
    }
    GetKeyNameText(nScanCode, szKeyName, CountOf(szKeyName));

    if (isFlagSet(fsModifiers, MOD_WIN)) {
        strText += _TLT("Winkey+");
    }
    if (isFlagSet(fsModifiers, MOD_CONTROL)) {
        strText += _TLT("Ctrl+");
    }
    if (isFlagSet(fsModifiers, MOD_SHIFT)) {
        strText += _TLT("Shift+");
    }
    if (isFlagSet(fsModifiers, MOD_ALT)) {
        strText += _TLT("Alt+");
    }
    strText += szKeyName;
}



CHotkeyCtrlEx::CHotkeyCtrlEx() {
    m_pWndProcOrg = nullptr;
    m_bShiftDown = m_bAltDown = m_bCtrlDown = m_bWinDown = false;
    m_bCharDown = false;
    m_nChar = 0;
    m_nHotVk = 0;
    m_nfsModifiers = 0;
}

CHotkeyCtrlEx::~CHotkeyCtrlEx() {

}

bool CHotkeyCtrlEx::init(Window *pParentWnd, uint32_t uChildId) {
    m_hWnd = ::getDlgItem(pParentWnd->getHandle(), uChildId);
    if (!m_hWnd) {
        return false;
    }

    m_bShiftDown = false;
    m_bAltDown = false;
    m_bCtrlDown = false;
    m_bWinDown = false;
    m_bCharDown = false;

    m_nChar = 0;

    m_nHotVk = 0;
    m_nfsModifiers = 0;

    // stash pointer to self
    SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);

    m_pWndProcOrg = (WNDPROC)GetWindowLong(m_hWnd, GWL_WNDPROC);
    SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)BaseWndProc);

    return true;
}

void CHotkeyCtrlEx::onKeyDown(uint32_t nChar, uint32_t nFlags) {
    /*    if (nChar == VK_CONTROL)
        m_bCtrlDown = true;
    else if (nChar == VK_SHIFT)
        m_bShiftDown = true;
    else if (nChar == VK_MENU)
        m_bAltDown = true;
    else if (nChar == VK_LWIN || nChar == VK_RWIN)
        m_bWinDown = true;
    else
    {
        m_bCharDown = true;
        m_nChar = nChar;
    }*/
    m_bWinDown = m_bCtrlDown = m_bAltDown = m_bShiftDown = false;

    if (isKeyPressed(VK_CONTROL)) {
        m_bCtrlDown = true;
    }
    if (isKeyPressed(VK_SHIFT)) {
        m_bShiftDown = true;
    }
    if (isKeyPressed(VK_MENU)) {
        m_bAltDown = true;
    }
    if (isKeyPressed(VK_LWIN) || isKeyPressed(VK_RWIN)) {
        m_bWinDown = true;
    }

    if (nChar != VK_CONTROL && nChar != VK_MENU && nChar != VK_SHIFT
        && nChar != VK_LWIN && nChar != VK_RWIN) {
        m_bCharDown = true;
        m_nChar = nChar;
    }

    updateHotKeyText();
}

void CHotkeyCtrlEx::onKeyUp(uint32_t nChar, uint32_t nFlags) {
    /*    if (nChar == VK_CONTROL)
        m_bCtrlDown = false;
    else if (nChar == VK_SHIFT)
        m_bShiftDown = false;
    else if (nChar == VK_MENU)
        m_bAltDown = false;
    else if (nChar == VK_APPS)
        m_bWinDown = false;
    else
    {
        m_bCharDown = false;
        m_nChar = 0;
        return;
    }*/
    m_bWinDown = m_bCtrlDown = m_bAltDown = m_bShiftDown = false;

    if (GetKeyState(VK_CONTROL) & 0x8000) {
        m_bCtrlDown = true;
    }
    if (GetKeyState(VK_SHIFT) & 0x8000) {
        m_bShiftDown = true;
    }
    if (GetKeyState(VK_MENU) & 0x8000) {
        m_bAltDown = true;
    }
    if ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000)) {
        m_bWinDown = true;
    }

    if (nChar != VK_CONTROL && nChar != VK_MENU && nChar != VK_SHIFT
        && nChar != VK_LWIN && nChar != VK_RWIN) {
        m_bCharDown = false;
        m_nChar = nChar;
    }

    if (m_bCharDown) {
        updateHotKeyText();
    }
}

void CHotkeyCtrlEx::getHotkey(uint32_t &nVirKey, uint32_t &nfsModifiers) {
    nVirKey = m_nHotVk;
    nfsModifiers = m_nfsModifiers;
}

void CHotkeyCtrlEx::setHotkey(uint32_t nVirKey, uint32_t nfsModifiers) {
    m_nHotVk = nVirKey;
    m_nfsModifiers = nfsModifiers;

    string strText;

    formatHotkeyText(strText, m_nHotVk, m_nfsModifiers);

    setWindowText(strText.c_str());
}

LRESULT CHotkeyCtrlEx::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
    //     if (message == WM_KEYDOWN)
    //     {
    //         char    szText[128];
    //         GetKeyNameText(lParam, szText, CountOf(szText));
    //         DBG_LOG1("Keydown: %s", szText);
    //         uint32_t    a, b, c, d, e;
    //         HKL    kb = LoadKeyboardLayout(stringPrintf("%08X", GetUserDefaultLangID()).c_str(), KLF_ACTIVATE);
    //         a = MapVirtualKeyEx(wParam, 0, kb);
    //         b = MapVirtualKeyEx(wParam, 1, kb);
    //         c = MapVirtualKeyEx(wParam, 2, kb);
    //         d = MapVirtualKeyEx(a, 1, kb);
    //         e = MapVirtualKeyEx(b, 0, kb);
    //         UnloadKeyboardLayout(kb);
    //     }
    //     else if (message == WM_KEYUP)
    //     {
    //         char    szText[128];
    //         GetKeyNameText(lParam, szText, CountOf(szText));
    //         DBG_LOG1("Keyup: %s", szText);
    //     }

    if (message == WM_KEYDOWN || message == WM_KEYUP || message == WM_CHAR
        || message == WM_RBUTTONUP || message == WM_CONTEXTMENU) {
        return Window::wndProc(message, wParam, lParam);
    } else if (message == WM_SYSKEYDOWN) {
        onKeyDown(wParam, lParam);
        return 0;
    } else if (message == WM_SYSKEYUP) {
        onKeyUp(wParam, lParam);
        return 0;
    } else if (message == WM_DESTROY) {
        SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)m_pWndProcOrg);
    } else if (message == WM_GETDLGCODE) {
        return DLGC_WANTALLKEYS;
    }

    return callWindowProc(m_pWndProcOrg,m_hWnd, message, wParam, lParam);
}

void CHotkeyCtrlEx::updateHotKeyText() {
    if (m_bCharDown) {
        m_nHotVk = m_nChar;
    } else {
        m_nHotVk = 0;
    }

    m_nfsModifiers = 0;
    if (m_bWinDown) {
        m_nfsModifiers |= MOD_WIN;
    }
    if (m_bShiftDown) {
        m_nfsModifiers |= MOD_SHIFT;
    }
    if (m_bCtrlDown) {
        m_nfsModifiers |= MOD_CONTROL;
    }
    if (m_bAltDown) {
        m_nfsModifiers |= MOD_ALT;
    }

    string strText;

    formatHotkeyText(strText, m_nHotVk, m_nfsModifiers);

    setWindowText(strText.c_str());
}
