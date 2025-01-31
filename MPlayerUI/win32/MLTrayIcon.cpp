

#include "../MPlayerApp.h"
#include "MLTrayIcon.h"
#include "../../Window/win32/ShellNotifyIcon.h"


#define IDN_MUSIC_PLAYER      1
#define IDN_PLAYER_CTRL     2
#define IDN_PLAYER_CTRL_MAX        (2 + 5)

SYSTRAY_ICON_CMD    g_sysTrayIconCmd[] = {
    { ID_PREVIOUS, "Previous Track", IDI_PREV, nullptr, false },
    { ID_PLAYPAUSE, "play/pause", IDI_PLAY, nullptr, false },
    { ID_NEXT,    "next Track", IDI_NEXT,    nullptr, false },
};

int MAX_PLAYER_TRAY_ICON_CMD = CountOf(g_sysTrayIconCmd);


CMLTrayIcon::CMLTrayIcon() {
    m_pWnd = nullptr;
    m_hIconTray = nullptr;
    m_bForceShow = false;
}

CMLTrayIcon::~CMLTrayIcon() {

}

void CMLTrayIcon::init(Window *pWnd) {
    m_pWnd = pWnd;

    //
    // 更新按钮托盘图标
    for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++) {
        string key = stringPrintf("TrayIcon%d", i);
        g_sysTrayIconCmd[i].isEnabled = g_profile.getBool(key.c_str(), false);
        g_sysTrayIconCmd[i].hIcon = (HICON)::LoadImage(getAppInstance(), MAKEINTRESOURCE(g_sysTrayIconCmd[i].uIconID), IMAGE_ICON, 16, 16, LR_VGACOLOR);
    }

    m_hIconTray = (HICON)::LoadImage(getAppInstance(), MAKEINTRESOURCE(IDI_MPLAYER), IMAGE_ICON, 16, 16, LR_VGACOLOR);

    updatePlayerSysTrayIcon();
    updateShowIconPos();
}

void CMLTrayIcon::quit() {
    //
    // remove icon in tray bar
    CShellNotifyIcon::delIcon(m_pWnd, IDN_MUSIC_PLAYER);

    if (m_hIconTray) {
        DestroyIcon(m_hIconTray);
        m_hIconTray = nullptr;
    }

    //
    // 删除系统托盘栏上播放器的控制图标
    for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++) {
        CShellNotifyIcon::delIcon(m_pWnd, IDN_PLAYER_CTRL + i);

        if (g_sysTrayIconCmd[i].hIcon) {
            DestroyIcon(g_sysTrayIconCmd[i].hIcon);
        }
    }
}

void CMLTrayIcon::updateShowIconPos() {
    string strCaption;
    SHOW_ICON_ON showIconOn;
    bool bToolWindow = false;
    bool bAddTrayIcon = true;

    showIconOn = (SHOW_ICON_ON)g_profile.getInt(SZ_SECT_UI, "ShowIconOn", SHOW_ICON_ON_TASKBAR);

    if (!isEmptyString(g_player.getFullTitle())) {
        strCaption = g_player.getFullTitle();
        strCaption += " - ";
        strCaption += SZ_APP_NAME;
    } else {
        strCaption = getAppNameLong();
    }

    switch (showIconOn) {
    case SHOW_ICON_ON_NONE:
        bAddTrayIcon = false;
        bToolWindow = true;
        break;
    case SHOW_ICON_ON_TASKBAR:
        bAddTrayIcon = false;
        bToolWindow = false;
        break;
    case SHOW_ICON_ON_SYSTRAY:
        bAddTrayIcon = true;
        bToolWindow = true;
        break;
    case SHOW_ICON_ON_BAR_TRAY:
        bAddTrayIcon = true;
        bToolWindow = false;
        break;
    }

    if (m_bForceShow) {
        bAddTrayIcon = true;
    }

    m_pWnd->setToolWindow(bToolWindow);

    if (bAddTrayIcon) {
        // add icon in tray bar
        CShellNotifyIcon::addIcon(m_pWnd, IDN_MUSIC_PLAYER, strCaption.c_str(), m_hIconTray, MPWM_TRAYICN);

        if (m_pWnd->isIconic()) {
            m_pWnd->hide();
        }
    } else {
        // remove icon in tray bar
        CShellNotifyIcon::delIcon(m_pWnd, IDN_MUSIC_PLAYER);
    }
}

// COMMENT:
//        将
void CMLTrayIcon::updatePlayerSysTrayIcon() {
    int i;

    for (i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++) {
        CShellNotifyIcon::delIcon(m_pWnd, IDN_PLAYER_CTRL + i);
    }
    for (i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++) {
        if (g_sysTrayIconCmd[i].isEnabled) {
            // add icon in tray bar
            CShellNotifyIcon::addIcon(m_pWnd, IDN_PLAYER_CTRL + i,
                _TL(g_sysTrayIconCmd[i].cmdText),
                g_sysTrayIconCmd[i].hIcon, MPWM_TRAYICN);
        }
    }
}

void CMLTrayIcon::updateTrayIconText(cstr_t szText) {
    int nShowPos;

    nShowPos = g_profile.getInt(SZ_SECT_UI, "ShowIconOn", SHOW_ICON_ON_TASKBAR);

    if (nShowPos == SHOW_ICON_ON_BAR_TRAY || nShowPos == SHOW_ICON_ON_SYSTRAY) {
        CShellNotifyIcon::modifyIcon(m_pWnd, IDN_MUSIC_PLAYER, szText, nullptr);
    }
}

void CMLTrayIcon::onMyNotifyIcon(WPARAM wParam, LPARAM lParam) {
    if (wParam == IDN_MUSIC_PLAYER) {
        if (lParam == WM_RBUTTONUP) {
            // show popup menu
            CPoint pt = getCursorPos();
            MPlayerApp::getMainWnd()->setForeground();
            MPlayerApp::getMainWnd()->onContexMenu(pt.x, pt.y);
        } else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP) {
            MPlayerApp::getMainWnd()->activateWindow();
        }
    } else {
        int i = wParam - IDN_PLAYER_CTRL;
        assert(i >= 0 && i < MAX_PLAYER_TRAY_ICON_CMD);
        if (i >= 0 && i < MAX_PLAYER_TRAY_ICON_CMD) {
            if (lParam == WM_RBUTTONUP) {
                //                 // show popup menu
                //                 CPoint    pt;
                //                 HMENU    hMenuPlayer;
                //
                //                 hMenuPlayer = GetSubMenu(m_hmenuContext, 4);
                //                 getCursorPos(&pt);
                //                 m_pWnd->setForeground();
                //
                //                 trackPopupMenu(hMenuPlayer, TPM_RIGHTBUTTON | TPM_LEFTALIGN, pt.x, pt.y, 0, m_pWnd->getHandle(), nullptr);
            } else if (lParam == WM_LBUTTONUP) {
                MPlayerApp::getMainWnd()->postShortcutKeyCmd(g_sysTrayIconCmd[i].cmdId);
            } else if (lParam == WM_LBUTTONDBLCLK) {
                // 激活播放器窗口
                MPlayerApp::getMainWnd()->activateWindow();
            }
        }
    }
}

void CMLTrayIcon::forceShow(bool bForceShow) {
    if (m_bForceShow != bForceShow) {
        m_bForceShow = bForceShow;

        updateShowIconPos();
    }
}
