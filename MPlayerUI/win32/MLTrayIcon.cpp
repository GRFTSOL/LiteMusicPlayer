// MLTrayIcon.cpp: implementation of the CMLTrayIcon class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "MLTrayIcon.h"
#include "MLCmd.h"
#include "ShellNotifyIcon.h"

#define IDN_MINILYRICS            1
#define IDN_PLAYER_CTRL            2
#define IDN_PLAYER_CTRL_MAX        (2 + 5)

SYSTRAY_ICON_CMD    g_SysTrayIconCmd[] = 
{
    { CMD_PREVIOUS, "Previous Track", IDI_PREV, nullptr, false },
    { CMD_PLAYPAUSE, "play/pause", IDI_PLAY, nullptr, false },
    { CMD_NEXT,    "next Track", IDI_NEXT,    nullptr, false },
};

int MAX_PLAYER_TRAY_ICON_CMD = CountOf(g_SysTrayIconCmd);


CMLTrayIcon::CMLTrayIcon()
{
    m_pWnd = nullptr;
    m_hIconTray = nullptr;
    m_bForceShow = false;
}

CMLTrayIcon::~CMLTrayIcon()
{

}

void CMLTrayIcon::init(Window *pWnd)
{
    m_pWnd = pWnd;

    //
    // 更新按钮托盘图标
    for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
    {
        char    szKey[128];

        wsprintf(szKey, "TrayIcon%d", i);
        g_SysTrayIconCmd[i].bEnable = g_profile.getBool(szKey, false);
        g_SysTrayIconCmd[i].hIcon = (HICON)::LoadImage(getAppInstance(), MAKEINTRESOURCE(g_SysTrayIconCmd[i].uIconID), IMAGE_ICON, 16, 16, LR_VGACOLOR);
    }

    m_hIconTray = (HICON)::LoadImage(getAppInstance(), MAKEINTRESOURCE(IDI_MPLAYER), IMAGE_ICON, 16, 16, LR_VGACOLOR);

    updatePlayerSysTrayIcon();
    updateShowIconPos();
}

void CMLTrayIcon::quit()
{
    //
    // remove icon in tray bar
    CShellNotifyIcon::delIcon(m_pWnd, IDN_MINILYRICS);

    if (m_hIconTray)
    {
        DestroyIcon(m_hIconTray);
        m_hIconTray = nullptr;
    }

    //
    // 删除系统托盘栏上播放器的控制图标
    for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
    {
        CShellNotifyIcon::delIcon(m_pWnd, IDN_PLAYER_CTRL + i);

        if (g_SysTrayIconCmd[i].hIcon)
            DestroyIcon(g_SysTrayIconCmd[i].hIcon);
    }
}

void CMLTrayIcon::updateShowIconPos()
{
    string        strCaption;
    SHOW_ICON_ON    showIconOn;
    bool        bToolWindow = false;
    bool        bAddTrayIcon = true;

    showIconOn = (SHOW_ICON_ON)g_profile.getInt(SZ_SECT_UI, "ShowIconOn", SHOW_ICON_ON_TASKBAR);

    if (!isEmptyString(g_Player.getFullTitle()))
    {
        strCaption = g_Player.getFullTitle();
        strCaption += " - ";
        strCaption += SZ_APP_NAME;
    }
    else
        strCaption = getAppNameLong();

    switch (showIconOn)
    {
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

    if (m_bForceShow)
        bAddTrayIcon = true;

    m_pWnd->setToolWindow(bToolWindow);

    if (bAddTrayIcon)
    {
        // add icon in tray bar
        CShellNotifyIcon::addIcon(m_pWnd, IDN_MINILYRICS, strCaption.c_str(), m_hIconTray, MPWM_TRAYICN);

        if (m_pWnd->isIconic())
            m_pWnd->showWindow(SW_HIDE);
    }
    else
    {
        // remove icon in tray bar
        CShellNotifyIcon::delIcon(m_pWnd, IDN_MINILYRICS);
    }
}

// COMMENT:
//        将
void CMLTrayIcon::updatePlayerSysTrayIcon()
{
    int        i;

    for (i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
    {
        CShellNotifyIcon::delIcon(m_pWnd, IDN_PLAYER_CTRL + i);
    }
    for (i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
    {
        if (g_SysTrayIconCmd[i].bEnable)
        {
            // add icon in tray bar
            CShellNotifyIcon::addIcon(m_pWnd, IDN_PLAYER_CTRL + i, 
                _TL(g_SysTrayIconCmd[i].szCmd), 
                g_SysTrayIconCmd[i].hIcon, MPWM_TRAYICN);
        }
    }
}

void CMLTrayIcon::updateTrayIconText(cstr_t szText)
{
    int            nShowPos;

    nShowPos = g_profile.getInt(SZ_SECT_UI, "ShowIconOn", SHOW_ICON_ON_TASKBAR);

    if (nShowPos == SHOW_ICON_ON_BAR_TRAY || nShowPos == SHOW_ICON_ON_SYSTRAY)
        CShellNotifyIcon::modifyIcon(m_pWnd, IDN_MINILYRICS, szText, nullptr);
}

void CMLTrayIcon::onMyNotifyIcon(WPARAM wParam, LPARAM lParam)
{
    if (wParam == IDN_MINILYRICS)
    {
        if (lParam == WM_RBUTTONUP)
        {
            // show popup menu
            CPoint pt;
            getCursorPos(&pt);
            CMPlayerAppBase::getMainWnd()->setForeground();
            CMPlayerAppBase::getMainWnd()->onContexMenu(pt.x, pt.y);
        }
        else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP)
        {
#ifndef _MPLAYER
            if (CMPlayerAppBase::getInstance()->isEmbeddedMode()
                && g_Player.getPlayerType() == P_WINAMP5)
            {
                // buf fix for winamp5
                HWND    hWnd = g_Player.getMainWnd();
                if (::isIconic(hWnd))
                {
                    ::showWindow(hWnd, SW_RESTORE);
                    if (CMPlayerAppBase::getMainWnd()->isTopmost())
                        CMPlayerAppBase::getMainWnd()->setTopmost(CMPlayerAppBase::getMainWnd()->isTopmost());
                }
            }
            CMPlayerAppBase::getMainWnd()->activateWindow();
#else
            activateWindow(CMPlayerAppBase::getMainWnd()->getHandle());
#endif

            // show cancel transparent with mouse click menu.
            if (CMPlayerAppBase::getMainWnd()->m_bClickThrough)
            {
                CMenu        menu;
                if (menu.loadPopupMenu(IDM_MOUSE_CLICK_THROUGH, 0))
                {
                    CPoint pt;
                    getCursorPos(&pt);
                    menu.checkItem(IDC_CLICK_THROUGH, true);
                    menu.trackPopupMenu(pt.x, pt.y, CMPlayerAppBase::getMainWnd());
                }
            }
        }
    }
    else
    {
        int i = wParam - IDN_PLAYER_CTRL;
        assert(i >= 0 && i < MAX_PLAYER_TRAY_ICON_CMD);
        if (i >= 0 && i < MAX_PLAYER_TRAY_ICON_CMD)
        {
            if (lParam == WM_RBUTTONUP)
            {
//                 // show popup menu
//                 CPoint    pt;
//                 HMENU    hMenuPlayer;
// 
//                 hMenuPlayer = GetSubMenu(m_hmenuContext, 4);
//                 getCursorPos(&pt);
//                 m_pWnd->setForeground();
// 
//                 trackPopupMenu(hMenuPlayer, TPM_RIGHTBUTTON | TPM_LEFTALIGN, pt.x, pt.y, 0, m_pWnd->getHandle(), nullptr);
            }
            else if (lParam == WM_LBUTTONUP)
            {
                CMPlayerAppBase::getMainWnd()->postShortcutKeyCmd(g_SysTrayIconCmd[i].dwCmd);
            }
            else if (lParam == WM_LBUTTONDBLCLK)
            {
                // 激活播放器窗口
                activateWindow(CMPlayerAppBase::getMainWnd()->getHandle());
            }
        }
    }
}

void CMLTrayIcon::forceShow(bool bForceShow)
{
    if (m_bForceShow != bForceShow)
    {
        m_bForceShow = bForceShow;

        updateShowIconPos();
    }
}
