#pragma once

#include "win32/VolumeOutMaster.h"
#include "PlayerSmoothTimer.h"
#include "win32/MLPlayerMgr.h"


///////////////////////////////////////////////////////////////////////////////
// create window to get playback pos of player timely.

class CPlayerEventDispatcher : public Window {
public:
    CPlayerEventDispatcher() { m_nTimeOutUpdateLyr = 40; }
    ~CPlayerEventDispatcher() { }

    enum {
        TIMER_LYR_DRAW_UPDATE       = 1,
    };

    void init() {
        Window::createEx(MSG_WND_CLASS_NAME, "PlayerEventWnd", 0, 0, 1, 1, nullptr, 0);

        m_volMaster.init(getHandle());

        // latest playlist and latest playing song file will loaded while main window is created.
        cstr_t szCmdLine;

        // get command line
        szCmdLine = GetCommandLine();
        szCmdLine = cmdLineNext(szCmdLine);
        if (strcasecmp(szCmdLine, "/iPodLyricsDownloader") == 0) {
            szCmdLine = "";
        }
        if (isEmptyString(szCmdLine)) {
            // load default playlist
            string strCurrentPlaylist;

            strCurrentPlaylist = g_profile.getString("Latest Playlist", "");
            if (!g_player.loadPlaylist(strCurrentPlaylist.c_str(), true)) {
                g_player.newNowPlaying();
            }

            int nNowPlaying = g_profile.getInt(SZ_SECT_PLAYER, "NowPlayingIdx", 0);
            g_player.setCurrentMediaInPlaylist(nNowPlaying);
        } else {
            // excute cmdline
            sendCommandLine(CMPlayerAppBase::getMainWnd()->getHandle(), szCmdLine);
        }
    }

    void quit() {
        m_volMaster.quit();

        destroy();
    }

    long getMasterVolume() {
        return m_volMaster.getVolume();
    }

    void setMasterVolume(long volume) {
        m_volMaster.setVolume(volume);
    }

    void setLyrDrawUpdateFast(bool bFast) {
        if (bFast) {
            m_nTimeOutUpdateLyr = 20;
        } else {
            m_nTimeOutUpdateLyr = 40;
        }

        startLyrDrawUpdate();
    }

    void startLyrDrawUpdate()
        { setTimer(TIMER_LYR_DRAW_UPDATE, m_nTimeOutUpdateLyr); }

    void stopLyrDrawUpdate()
        { killTimer(TIMER_LYR_DRAW_UPDATE); }

    bool onCreate() {
        if (!Window::onCreate()) {
            return false;
        }

        showWindow(SW_MINIMIZE);
        showWindow(SW_HIDE);

        return true;
    }

    void onPaint(CGraphics *canvas, CRect &rcClip) {
        showWindow(SW_HIDE);
    }

    void onTimer(uint32_t nIDEvent) {
        assert(nIDEvent == TIMER_LYR_DRAW_UPDATE);

        if (g_player.isUseSeekTimeAsPlayingTime()) {
            return;
        }

        int nPlayPos = g_player.getPlayPos();
        g_currentLyrics.SetPlayElapsedTime(nPlayPos);

        CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);

        dispatchPlayPosEvent(nPlayPos);
    }


    void onCopyData(WPARAM wParam, COPYDATASTRUCT *pcds) {
        if (wParam == ML_ACTIVATE && pcds->dwData == ML_ACTIVATE) {
            CMPSkinMainWnd *pWnd = CMPlayerAppBase::getMainWnd();
            if (pWnd) {
                pWnd->activateWindow();
            } else {
                ERR_LOG0("MiniLyrics main window hasn't been started yet, can't activate it.");
            }
        } else if (wParam == ML_SEND_CMD_LINE && pcds->dwData == ML_SEND_CMD_LINE) {
        }
    }

    LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_COPYDATA) {
            onCopyData(wParam, (COPYDATASTRUCT *)lParam);
        }
#ifndef _MPLAYER
        else if (message == MM_MIXM_CONTROL_CHANGE) {
            // Volume changed message.
            CEventPlayerSettingChanged *pEvent = new CEventPlayerSettingChanged();
            pEvent->eventType = ET_PLAYER_SETTING_CHANGED;
            pEvent->settingType = IMPEvent::MPS_VOLUME;
            pEvent->value = g_player.getVolume();

            CMPlayerApp::getEventsDispatcher()->dispatchUnsyncEvent(pEvent);
        }
#endif

        return Window::wndProc(message, wParam, lParam);
    }

protected:
    int                         m_nTimeOutUpdateLyr;
    CVolumeOutMaster            m_volMaster;

};

extern CPlayerEventDispatcher g_playerEventDispatcher;
