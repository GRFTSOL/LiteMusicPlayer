// MPSkinMainWnd.h: interface for the CMPSkinMainWnd class.
#pragma once

#include "MPSkinWnd.h"
#include "MLTrayIcon.h"
#include "MPSkinMainWnd.h"


class CMPSkinMainWnd : public CMPSkinMainWndBase {
public:
    CMPSkinMainWnd();
    virtual ~CMPSkinMainWnd();

    virtual void onCreate();
    virtual void onDestroy();

    // IEventHandler
    virtual void onEvent(const IEvent *pEvent);

    virtual void onCopyData(WPARAM wParam, PCOPYDATASTRUCT pCopyData);

    virtual void onSkinLoaded();

    virtual void onTimer(uint32_t nIDEvent);

    void updatePlayerSysTrayIcon() { m_mlTrayIcon.updatePlayerSysTrayIcon(); }

    CMLTrayIcon &getTrayIcon() { return m_mlTrayIcon; }

protected:
    virtual LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

protected:
    CMLTrayIcon                 m_mlTrayIcon;

    uint32_t                    m_nTimerFixWndFocus;

};
