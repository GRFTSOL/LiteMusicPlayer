#pragma once

#ifndef MPlayerUI_win32_MLTrayIcon_h
#define MPlayerUI_win32_MLTrayIcon_h


#define MPWM_TRAYICN        (WM_USER + 10)

enum SHOW_ICON_ON {
    SHOW_ICON_ON_TASKBAR        = 0,
    SHOW_ICON_ON_SYSTRAY,
    SHOW_ICON_ON_BAR_TRAY,
    SHOW_ICON_ON_NONE
};

struct SYSTRAY_ICON_CMD {
    uint32_t                    dwCmd;
    cstr_t                      szCmd;
    uint32_t                    uIconID;
    HICON                       hIcon;
    bool                        bEnable;
};

extern SYSTRAY_ICON_CMD g_SysTrayIconCmd[];
extern int MAX_PLAYER_TRAY_ICON_CMD;

class CMLTrayIcon {
public:
    CMLTrayIcon();
    virtual ~CMLTrayIcon();

    void init(Window *pWnd);
    void quit();

    void updateShowIconPos();
    void updatePlayerSysTrayIcon();

    void updateTrayIconText(cstr_t szText);

    void onMyNotifyIcon(WPARAM wParam, LPARAM lParam);

    void forceShow(bool bForceShow);

protected:
    Window                      *m_pWnd;
    HICON                       m_hIconTray;

    bool                        m_bForceShow;

};

#endif // !defined(MPlayerUI_win32_MLTrayIcon_h)
