// MPFloatingLyrWnd.h: interface for the CMPFloatingLyrWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPFloatingLyrWnd_H__45561DD4_521E_428B_9174_527AACE5C84E__INCLUDED_)
#define AFX_MPFloatingLyrWnd_H__45561DD4_521E_428B_9174_527AACE5C84E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MPSkinWnd.h"

class CMPFloatingLyrWnd : public CMPSkinWnd  
{
public:
    CMPFloatingLyrWnd();
    virtual ~CMPFloatingLyrWnd();

    int create();

    virtual bool onCustomCommand(int nId);

protected:
    LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

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

extern CMPFloatingLyrWnd        g_wndFloatingLyr;

#endif // !defined(AFX_MPFloatingLyrWnd_H__45561DD4_521E_428B_9174_527AACE5C84E__INCLUDED_)
