#pragma once

#ifndef MPlayerUI_mac_MLTrayIcon_h
#define MPlayerUI_mac_MLTrayIcon_h

#include "Skin/Skin.h"


enum SHOW_ICON_ON {
    SHOW_ICON_ON_TASKBAR        = 0,
    SHOW_ICON_ON_SYSTRAY,
    SHOW_ICON_ON_BAR_TRAY,
    SHOW_ICON_ON_NONE
};

struct SYSTRAY_ICON_CMD {
    uint32_t                    cmdId;
    cstr_t                      cmdText;
    bool                        isEnabled;
};

extern SYSTRAY_ICON_CMD g_sysTrayIconCmd[];
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

    void forceShow(bool bForceShow);

protected:
    struct MLtrayIconPrivate        *_data = nullptr;

};

#endif // !defined(MPlayerUI_mac_MLTrayIcon_h)
