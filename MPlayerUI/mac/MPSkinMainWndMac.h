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

    virtual void onCreate() override;
    virtual void onDestroy() override;

    // IEventHandler
    virtual void onEvent(const IEvent *pEvent) override;

    void updatePlayerSysTrayIcon() { m_mlTrayIcon.updatePlayerSysTrayIcon(); }

    CMLTrayIcon &getTrayIcon() { return m_mlTrayIcon; }

protected:
    CMLTrayIcon        m_mlTrayIcon;

};
