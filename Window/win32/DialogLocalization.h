
#pragma once

#include "Window.h"

class DialogLocalization {
public:
    void init(HWND hWnd);
    void destroy();

    void addDlgItem(int nID);
    void addDlgItem(HWND hWndItem);

    void removeDlgItem(HWND hWndItem);
    void removeDlgItem(int nID);

    void toLocal();

protected:
    struct DlgItem {
        HWND                        hWnd;
        string                      text;
    };

    std::vector<DlgItem>            _dlgItems;

    HWND                            _hWnd = nullptr;
    string                          _text;
};
