#include "BaseWnd.h"


void openUrl(Window *pWnd, cstr_t szUrl) {
    shellExecute(pWnd->getHandle(), "open", szUrl, nullptr, nullptr, SW_SHOWNORMAL);
}

bool isModifierKeyPressed(int nKey, uint32_t nFlags) {
    assert(nKey == MK_SHIFT || nKey == MK_CONTROL || nKey == MK_ALT);
    if (nKey == MK_SHIFT) {
        nKey = VK_SHIFT;
    } else if (nKey == MK_CONTROL) {
        nKey = VK_CONTROL;
    } else if (nKey == MK_ALT) {
        nKey = VK_MENU;
    }

    return (GetKeyState(nKey) & 0x8000) == 0x8000;
}

void getCursorPos(LPPOINT lpPoint) {
    ::getCursorPos(lpPoint);
}

// set new cursor, and return the old cursor.
bool setCursor(Cursor &Cursor, Cursor *pCursorOld) {
    HCURSOR hCursorOld;
    hCursorOld = ::setCursor(Cursor.m_cursor);

    if (pCursorOld) {
        pCursorOld->m_cursor = hCursorOld;
    }

    return true;
}

Window *findWindow(cstr_t szClassName, cstr_t szWindowName) {
    HWND hWnd = ::findWindow(szClassName, szWindowName);
    if (!hWnd) {
        return nullptr;
    }

    return Window::fromHandle(hWnd);
}
