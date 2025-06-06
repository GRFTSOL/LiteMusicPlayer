﻿#include "../MPlayerApp.h"
#include "../MPSkinMainWnd.h"

#ifdef _WIN32
#include "MPMsg.h"
#endif


const uint32_t g_msgTaskBarCreated = RegisterWindowMessageA("TaskBarCreated");


CMPSkinMainWnd::CMPSkinMainWnd() {
}

CMPSkinMainWnd::~CMPSkinMainWnd() {
}

void CMPSkinMainWnd::onCreate() {
    CMPSkinMainWndBase::onCreate();

    registerHandler(MPlayerApp::getEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_CUR_MEDIA_INFO_CHANGED);

    m_mlTrayIcon.init(this);

    m_nTimerFixWndFocus = registerTimerObject(nullptr, 1000);
    if (GetForegroundWindow() != m_hWnd && m_bActived) {
        onActivate(false);
    }
}

void CMPSkinMainWnd::onDestroy() {
    m_mlTrayIcon.quit();

    CMPSkinMainWndBase::onDestroy();
}

void CMPSkinMainWnd::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED
        || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED) {
        // update main window caption
        char szCaption[512];

        if (g_player.isMediaOpened()) {
            snprintf(szCaption, CountOf(szCaption), "%s - %s", g_player.getFullTitle(), SZ_APP_NAME);
        } else {
            strcpy_safe(szCaption, CountOf(szCaption), getAppNameLong().c_str());
        }

        setCaptionText(szCaption);

        m_mlTrayIcon.updateTrayIconText(szCaption);
    } else if (pEvent->eventType == ET_UI_SETTINGS_CHANGED) {
        if (isPropertyName(pEvent->name.c_str(), "ShowIconOn")) {
            m_mlTrayIcon.updateShowIconPos();
        } else {
            CMPSkinMainWndBase::onEvent(pEvent);
        }
    } else {
        CMPSkinMainWndBase::onEvent(pEvent);
    }
}

void CMPSkinMainWnd::onCopyData(WPARAM wParam, PCOPYDATASTRUCT pCopyData) {
    vector<string> vCmdLine;

    if (pCopyData->dwData != ML_SEND_CMD_LINE && wParam != ML_SEND_CMD_LINE) {
        return;
    }

    cstr_t szCmdLine = (cstr_t)pCopyData->lpData;
    cmdLineAnalyse(szCmdLine, vCmdLine);

    if (vCmdLine.empty()) {
        return;
    }

    // deal with option cmd line
    for (int i = 0; i < (int)vCmdLine.size(); i++) {
        string &str = vCmdLine[i];
        if (str[0] == '/' || str[0] == '-') {
            vCmdLine.erase(vCmdLine.begin() + i);
            i--;
        }
    }

    if (vCmdLine.size() == 1 && fileIsExtSame(vCmdLine[0].c_str(), ".m3u")) {
        // open playlist
        g_player.saveNowPlaying();
    } else {
        g_player.setNowPlayingModified(true);
    }

    g_player.clearNowPlaying();

    for (int i = 0; i < (int)vCmdLine.size(); i++) {
        g_player.addToNowPlaying(vCmdLine[i].c_str());
    }

    g_player.play();
}

void CMPSkinMainWnd::onSkinLoaded() {
    CMPSkinMainWndBase::onSkinLoaded();
}

void CMPSkinMainWnd::onTimer(uint32_t nIDEvent) {
    if (nIDEvent == m_nTimerFixWndFocus) {
        // Under some special situation, MusicPlayer may not receive WM_ACTIVATE !activate notifications.
        unregisterTimerObject(nullptr, m_nTimerFixWndFocus);
        if (GetForegroundWindow() != m_hWnd) {
            if (m_bActived) {
                onActivate(false);
            }
        }
        return;
    }

    return CMPSkinMainWndBase::onTimer(nIDEvent);
}

LRESULT CMPSkinMainWnd::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COPYDATA:
        onCopyData(wParam, (PCOPYDATASTRUCT)lParam);
        break;
    case MPWM_TRAYICN:
        m_mlTrayIcon.onMyNotifyIcon(wParam, lParam);
        return 0;
    case WM_HOTKEY:
        // onHotKey(int nId, uint32_t fuModifiers, uint32_t uVirtKey)
        MPlayerApp::getHotkey().onHotKey((int)wParam, (uint32_t)LOWORD(lParam), (uint32_t)HIWORD(lParam));
        return 0;
    default:
        if (message == g_msgTaskBarCreated) {
            m_mlTrayIcon.updateShowIconPos();
            m_mlTrayIcon.updatePlayerSysTrayIcon();
        }
        break;
    }

    return CMPSkinWnd::wndProc(message, wParam, lParam);
}
