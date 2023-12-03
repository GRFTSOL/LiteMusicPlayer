#pragma once

#ifndef MPlayerUI_MPSkinMainWnd_h
#define MPlayerUI_MPSkinMainWnd_h


#include "MPSkinWnd.h"

class CMPSkinMainWndBase : public CMPSkinWnd {
public:
    CMPSkinMainWndBase();
    virtual ~CMPSkinMainWndBase();

    virtual void onCreate() override;
    virtual void onDestroy() override;

    // IEventHandler
    virtual void onEvent(const IEvent *pEvent) override;

protected:
    void updateCaptionText();

};

#ifdef _WIN32
#include "win32/MPSkinMainWndWin32.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/MPSkinMainWndGtk2.h"
#endif

#ifdef _MAC_OS
#include "mac/MPSkinMainWndMac.h"


#endif

#endif // !defined(MPlayerUI_MPSkinMainWnd_h)
