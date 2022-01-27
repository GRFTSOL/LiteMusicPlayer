// MPSkinMainWnd.cpp: implementation of the CMPSkinMainWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "MPSkinMainWnd.h"

#ifdef _WIN32
#include "MPMsg.h"
#endif

#ifdef _MPLAYER
#include "MPHelper.h"
#endif

const uint32_t g_msgTaskBarCreated = RegisterWindowMessage("TaskBarCreated");


CMPSkinMainWnd::CMPSkinMainWnd()
{
}

CMPSkinMainWnd::~CMPSkinMainWnd()
{
}

bool CMPSkinMainWnd::onCreate()
{
    if (!CMPSkinMainWndBase::onCreate())
        return false;

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_CUR_MEDIA_INFO_CHANGED);

    m_mlTrayIcon.init(this);

    m_nTimerFixWndFocus = registerTimerObject(nullptr, 1000);
    if (GetForegroundWindow() != m_hWnd && m_bActived)
        onActivate(false);

    return true;
}

void CMPSkinMainWnd::onDestroy()
{
    m_mlTrayIcon.quit();

    CMPSkinMainWndBase::onDestroy();
}

void CMPSkinMainWnd::onEvent(const IEvent *pEvent)
{
    if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED 
        || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED)
    {
        // update main window caption
        char        szCaption[512];

        if (g_Player.isMediaOpened())
            snprintf(szCaption, CountOf(szCaption), "%s - %s", g_Player.getFullTitle(), SZ_APP_NAME);
        else
            strcpy_safe(szCaption, CountOf(szCaption), getAppNameLong().c_str());

        setWindowText(szCaption);

        m_mlTrayIcon.updateTrayIconText(szCaption);
    }
    else if (pEvent->eventType == ET_UI_SETTINGS_CHANGED)
    {
        if (isPropertyName(pEvent->name.c_str(), "ShowIconOn"))
            m_mlTrayIcon.updateShowIconPos();
        else
            CMPSkinMainWndBase::onEvent(pEvent);
    }
    else
        CMPSkinMainWndBase::onEvent(pEvent);
}

void CMPSkinMainWnd::onCopyData(WPARAM wParam, PCOPYDATASTRUCT pCopyData)
{
#ifdef _MPLAYER
    vector<string> vCmdLine;
    cstr_t            szCmdLine;
    int                i;

    if (pCopyData->dwData != ML_SEND_CMD_LINE && wParam != ML_SEND_CMD_LINE)
        return;

    szCmdLine = (cstr_t)pCopyData->lpData;
    cmdLineAnalyse(szCmdLine, vCmdLine);

    if (vCmdLine.empty())
        return;

    // deal with option cmd line
    for (i = 0; i < (int)vCmdLine.size(); i++)
    {
        string        &str = vCmdLine[i];
        if (str[0] == '/' || str[0] == '-')
        {
            vCmdLine.erase(vCmdLine.begin() + i);
            i--;
        }
    }

    if (vCmdLine.size() == 1 && fileIsExtSame(vCmdLine[0].c_str(), ".m3u"))
    {
        // open playlist
        g_Player.saveCurrentPlaylist();

        g_Player.m_strCurrentPlaylist = vCmdLine[0];
        g_Player.setPlaylistModified(false);
    }
    else
    {
        getDefaultPlaylistName(g_Player.m_strCurrentPlaylist);
        g_Player.setPlaylistModified(true);
    }

    g_Player.clearPlaylist();

    for (i = 0; i < (int)vCmdLine.size(); i++)
    {
        g_Player.addToPlaylist(vCmdLine[i].c_str());
    }

    g_Player.play();
#endif
}

void CMPSkinMainWnd::onSkinLoaded()
{
    CMPSkinMainWndBase::onSkinLoaded();
}

void CMPSkinMainWnd::onTimer(uint32_t nIDEvent)
{
    if (nIDEvent == m_nTimerFixWndFocus)
    {
        // Under some special situation, MiniLyrics may not receive WM_ACTIVATE !activate notifications.
        unregisterTimerObject(nullptr, m_nTimerFixWndFocus);
        if (GetForegroundWindow() != m_hWnd)
        {
            if (m_bActived)
                onActivate(false);
        }
        return;
    }

    return CMPSkinMainWndBase::onTimer(nIDEvent);
}

LRESULT CMPSkinMainWnd::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COPYDATA:
        onCopyData(wParam, (PCOPYDATASTRUCT)lParam);
        break;
    case MPWM_TRAYICN:
        m_mlTrayIcon.onMyNotifyIcon(wParam, lParam);
        return 0;
    case WM_HOTKEY:
        // onHotKey(int nId, uint32_t fuModifiers, uint32_t uVirtKey)
        CMPlayerAppBase::getHotkey().onHotKey((int)wParam, (uint32_t)LOWORD(lParam), (uint32_t)HIWORD(lParam));
        return 0;
#ifndef _MPLAYER
    case WM_SYSCOMMAND:
        if (wParam == SC_RESTORE)
        {
            if (isIconic())
                CMPlayerAppBase::getMPSkinFactory()->restoreAll();
        }
        break;
#endif
    default:
        if (message == g_msgTaskBarCreated)
        {
            m_mlTrayIcon.updateShowIconPos();
            m_mlTrayIcon.updatePlayerSysTrayIcon();
        }
        break;
    }

    return CMPSkinWnd::wndProc(message, wParam, lParam);
}

#ifndef _MPLAYER
bool CMPSkinMainWnd::isTopmost()
{
    HWND    hWndParent = getRootParentWnd();
    if (hWndParent)
    {
        return tobool(::isTopmostWindow(hWndParent));
    }

    return Window::isTopmost();
}

bool CMPSkinMainWnd::isIconic()
{
    HWND    hWndParent = getRootParentWnd();
    if (hWndParent)
    {
        return tobool(::isIconic(hWndParent));
    }

    return Window::isIconic();
}

void CMPSkinMainWnd::setTopmost(bool bTopmost)
{
    HWND    hWndParent = getRootParentWnd();
    if (hWndParent)
    {
        ::topmostWindow(hWndParent, bTopmost);
        return;
    }

    Window::setTopmost(bTopmost);
}

extern bool                g_bAutoMinimized;

void CMPSkinMainWnd::minimize()
{
    if (getRootParentWnd())
        return;

    showWindow(SW_MINIMIZE);

    g_bAutoMinimized = false;

    if (isToolWindow())
        showWindow(SW_HIDE);
}
#endif

void CMPSkinMainWnd::activateWindow()
{
    HWND    hWndParent = getRootParentWnd();
    if (hWndParent)
    {
        ::activateWindow(hWndParent);
        return;
    }

    ::activateWindow(m_hWnd);
}

HWND CMPSkinMainWnd::getRootParentWnd()
{
    HWND        hParent;
    HWND        hTemp;

    hTemp = hParent = ::getParent(m_hWnd);
    for (int i = 1; hTemp && i <= 3; i++)
    {
        hParent = hTemp;
        hTemp = ::getParent(hTemp);
    }

    return hParent;
}
