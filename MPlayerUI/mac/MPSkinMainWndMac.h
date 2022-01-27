// MPSkinMainWnd.h: interface for the CMPSkinMainWnd class.
#pragma once

#include "MPSkinWnd.h"
#include "MLTrayIcon.h"
#include "MPSkinMainWnd.h"

class CMPSkinMainWnd : public CMPSkinMainWndBase
{
public:
    CMPSkinMainWnd();
    virtual ~CMPSkinMainWnd();

    virtual void onCreate();
    virtual void onDestroy();

    // IEventHandler
    virtual void onEvent(const IEvent *pEvent);

    void updatePlayerSysTrayIcon() { m_mlTrayIcon.updatePlayerSysTrayIcon(); }

    CMLTrayIcon &getTrayIcon() { return m_mlTrayIcon; }

protected:
    CMLTrayIcon        m_mlTrayIcon;

};
