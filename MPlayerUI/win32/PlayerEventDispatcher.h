#pragma once

#include "PlayerSmoothTimer.h"
#include "MPMsg.h"


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
    }

    void quit() {
        destroy();
    }

    void startLyrDrawUpdate()
        { setTimer(TIMER_LYR_DRAW_UPDATE, m_nTimeOutUpdateLyr); }

    void stopLyrDrawUpdate()
        { killTimer(TIMER_LYR_DRAW_UPDATE); }

    void onCreate() {
        Window::onCreate();

        ::ShowWindow(m_hWnd, SW_MINIMIZE);
        ::ShowWindow(m_hWnd, SW_HIDE);
    }

    void onTimer(uint32_t nIDEvent) {
        assert(nIDEvent == TIMER_LYR_DRAW_UPDATE);

        if (g_player.isUseSeekTimeAsPlayingTime()) {
            return;
        }

        int nPlayPos = g_player.getPlayPos();
        g_currentLyrics.SetPlayElapsedTime(nPlayPos);

        MPlayerApp::getEventsDispatcher()->dispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);

        dispatchPlayPosEvent(nPlayPos);
    }


    void onCopyData(WPARAM wParam, COPYDATASTRUCT *pcds) {
        if (wParam == ML_ACTIVATE && pcds->dwData == ML_ACTIVATE) {
            CMPSkinMainWnd *pWnd = MPlayerApp::getMainWnd();
            if (pWnd) {
                pWnd->activateWindow();
            } else {
                ERR_LOG0("MusicPlayer main window hasn't been started yet, can't activate it.");
            }
        } else if (wParam == ML_SEND_CMD_LINE && pcds->dwData == ML_SEND_CMD_LINE) {
        }
    }

    LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_COPYDATA) {
            onCopyData(wParam, (COPYDATASTRUCT *)lParam);
        }

        return Window::wndProc(message, wParam, lParam);
    }

protected:
    int                         m_nTimeOutUpdateLyr;

};

extern CPlayerEventDispatcher g_playerEventDispatcher;
