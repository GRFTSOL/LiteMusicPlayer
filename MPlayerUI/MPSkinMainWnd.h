// MPSkinMainWnd.h: interface for the CMPSkinMainWndBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINMAINWND_H__5D734F0B_51A3_4A5C_BC56_E5064B5410B2__INCLUDED_)
#define AFX_MPSKINMAINWND_H__5D734F0B_51A3_4A5C_BC56_E5064B5410B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MPSkinWnd.h"

class CMPSkinMainWndBase : public CMPSkinWnd
{
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

#endif // !defined(AFX_MPSKINMAINWND_H__5D734F0B_51A3_4A5C_BC56_E5064B5410B2__INCLUDED_)
