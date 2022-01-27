// MPSkinMainWnd.h: interface for the CMPSkinMainWnd class.
#pragma once

#include "../MPSkinWnd.h"

class CMPSkinMainWnd : public CMPSkinMainWndBase
{
public:
    CMPSkinMainWnd();
    virtual ~CMPSkinMainWnd();

    virtual bool onCreate();
    virtual void onDestroy();

    // IEventHandler
    virtual void onEvent(const IEvent *pEvent);

protected:

};
