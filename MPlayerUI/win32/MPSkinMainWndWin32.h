// MPSkinMainWnd.h: interface for the CMPSkinMainWnd class.
#pragma once

#include "MPSkinWnd.h"
#include "MLTrayIcon.h"
#include "MPSkinMainWnd.h"


class CMPSkinMainWnd : public CMPSkinMainWndBase {
public:
    CMPSkinMainWnd();
    virtual ~CMPSkinMainWnd();

    virtual bool onCreate();
    virtual void onDestroy();

    // IEventHandler
    virtual void onEvent(const IEvent *pEvent);

    virtual void onCopyData(WPARAM wParam, PCOPYDATASTRUCT pCopyData);

    virtual void onSkinLoaded();

    virtual void onTimer(uint32_t nIDEvent);

    void updatePlayerSysTrayIcon() { m_mlTrayIcon.updatePlayerSysTrayIcon(); }

    CMLTrayIcon &getTrayIcon() { return m_mlTrayIcon; }

#ifndef _MPLAYER
public:
    //
    // functions for embedded into other windows...
    //
    virtual bool isTopmost();
    virtual bool isIconic();
    virtual void setTopmost(bool bTopmost);
    virtual void minimize();
#endif
    virtual void activateWindow();

    HWND getRootParentWnd();

protected:
    virtual LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

protected:
    CMLTrayIcon                 m_mlTrayIcon;

    uint32_t                    m_nTimerFixWndFocus;

};
