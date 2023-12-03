#pragma once

#ifndef MPlayerUI_MPFloatingLyrWnd_h
#define MPlayerUI_MPFloatingLyrWnd_h


#include "MPSkinWnd.h"


class CMPFloatingLyrWnd : public CMPSkinWnd {
public:
    CMPFloatingLyrWnd();
    virtual ~CMPFloatingLyrWnd();

    int create();

    void onCommand(uint32_t nId) override;

protected:
    //
    // Special settings for different kind of winodw (Floating lyrics and Normal window).
    //
    bool settingGetTopmost() override;
    int settingGetOpaquePercent() override;
    bool settingGetClickThrough() override;

    void settingReverseTopmost() override;
    void settingSetOpaquePercent(int nPercent) override;
    void settingReverseClickThrough() override;

};

extern CMPFloatingLyrWnd g_wndFloatingLyr;

#endif // !defined(MPlayerUI_MPFloatingLyrWnd_h)
