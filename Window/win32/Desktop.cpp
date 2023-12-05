#include "../WindowLib.h"


void openUrl(Window *pWnd, cstr_t szUrl) {
    ShellExecute(pWnd->getWndHandle(), "open", szUrl, nullptr, nullptr, SW_SHOWNORMAL);
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

CPoint getCursorPos() {
	CPoint pt;
    ::GetCursorPos(&pt);

	return { pt.x, pt.y };
}

bool getMonitorRestrictRect(const CRect &rcIn, CRect &rcRestrict) {
   const HWND hDesktop = GetDesktopWindow();

    RECT desktop;
    GetWindowRect(hDesktop, &desktop);

    rcRestrict.setLTRB(desktop.left, desktop.top, desktop.right, desktop.bottom);

    return true;
}

bool copyTextToClipboard(cstr_t text) {
    if (!OpenClipboard(NULL)) {
        return false;
    }

    auto len = strlen(text);
    auto data = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(char));
    if (data == NULL) {
        CloseClipboard();
        return false;
    }

    auto buf = (char *)GlobalLock(data);
    memcpy(buf, text, len * sizeof(char));
    buf[len] = '\0';
    GlobalUnlock(data);

    SetClipboardData(CF_TEXT, data);

    return true;
}

bool getClipBoardText(string &str) {
    if (!IsClipboardFormatAvailable(CF_TEXT)) {
        return false;
    }

    if (!OpenClipboard(NULL)) {
        return false;
    }

    bool succeeded = false;
    auto data = GetClipboardData(CF_TEXT);
    if (data != NULL) {
        auto text = (char *)GlobalLock(data);
        if (text != NULL) {
            str.assign(text);
            GlobalUnlock(data);
            succeeded = true;
        }
    }
    CloseClipboard();

    return succeeded;
}
