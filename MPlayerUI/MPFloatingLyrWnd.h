#pragma once

#ifndef MPlayerUI_MPFloatingLyrWnd_h
#define MPlayerUI_MPFloatingLyrWnd_h


#include "MPSkinWnd.h"


class CMPFloatingLyrWnd : public CMPSkinWnd {
public:
    CMPFloatingLyrWnd();
    virtual ~CMPFloatingLyrWnd();

    int create();

    virtual bool onCustomCommand(int nId);

protected:
    //
    // Special settings for different kind of winodw (Floating lyrics and Normal window).
    //
    virtual bool settingGetTopmost();
    virtual int settingGetOpaquePercent();
    virtual bool settingGetClickThrough();

    virtual void settingReverseTopmost();
    virtual void settingSetOpaquePercent(int nPercent);
    virtual void settingReverseClickThrough();

};

extern CMPFloatingLyrWnd g_wndFloatingLyr;

#endif // !defined(MPlayerUI_MPFloatingLyrWnd_h)
