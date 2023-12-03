#pragma once

#ifndef MPlayerUI_win32_ShellNotifyIcon_h
#define MPlayerUI_win32_ShellNotifyIcon_h


class CShellNotifyIcon {
public:
    CShellNotifyIcon();
    ~CShellNotifyIcon();

    static bool addIcon(Window *pWnd, uint32_t nID, cstr_t szTip, HICON hIcon, uint32_t nCallbackMessage);
    static bool delIcon(Window *pWnd, uint32_t nID);
    static bool modifyIcon(Window *pWnd, uint32_t nID, cstr_t szTip, HICON hIcon = nullptr);

};

#endif // !defined(MPlayerUI_win32_ShellNotifyIcon_h)
