#include "DialogLocalization.h"


void DialogLocalization::init(HWND hWnd) {
    _hWnd = hWnd;

    WCHAR text[512];

    auto len = GetWindowTextW(_hWnd, text, CountOf(text));
    _text = ucs2ToUtf8(text, len);

    _dlgItems.clear();

    // hWndItem = FindWindowEx(_hWnd, NULL, NULL, NULL);
    auto hWndItem = GetWindow(_hWnd, GW_CHILD);

    while (hWndItem) {
        addDlgItem(hWndItem);
        // hWndItem = FindWindowEx(_hWnd, hWndItem, NULL, NULL);
        hWndItem = GetWindow(hWndItem, GW_HWNDNEXT);
    }
}

void DialogLocalization::addDlgItem(int nID) {
    assert(_hWnd);

    auto hWndItem = GetDlgItem(_hWnd, nID);
    assert(hWndItem);

    addDlgItem(hWndItem);
}

void DialogLocalization::addDlgItem(HWND hWndItem) {
    assert(_hWnd);
    assert(hWndItem);

    WCHAR text[512];

    auto len = GetWindowTextW(hWndItem, text, CountOf(text));
    if (len > 0) {
        _dlgItems.push_back({hWndItem, ucs2ToUtf8(text, len)});
    }
}

void DialogLocalization::toLocal()
{
    assert(_hWnd);

    SendMessage(_hWnd, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    if (!_text.empty()) {
        auto utf8 = _TL(_text.c_str());
        SetWindowTextW(_hWnd, utf8ToUCS2(utf8).c_str());
    }

    for (auto &item : _dlgItems) {
        auto utf8 = _TL(item.text.c_str());
        SetWindowTextW(item.hWnd, utf8ToUCS2(utf8).c_str());
    }
}

void DialogLocalization::removeDlgItem(int nID) {
    assert(_hWnd);

    auto hWndItem = GetDlgItem(_hWnd, nID);
    assert(hWndItem);

    removeDlgItem(hWndItem);
}

void DialogLocalization::removeDlgItem(HWND hWndItem) {
    assert(_hWnd);

    for (auto it = _dlgItems.begin(); it != _dlgItems.end(); it++) {
        if ((*it).hWnd == hWndItem) {
            SetWindowTextW((*it).hWnd, utf8ToUCS2((*it).text.c_str()).c_str());
            _dlgItems.erase(it);
            return;
        }
    }
}

void DialogLocalization::destroy() {
    _hWnd = NULL;
    _dlgItems.erase(_dlgItems.begin(), _dlgItems.end());
}
