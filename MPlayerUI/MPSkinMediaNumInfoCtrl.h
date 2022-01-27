// MPSkinMediaNumInfoCtrl.h: interface for the CMPSkinMediaNumInfoCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINNUMINFOCTRL_H__9FFF7438_90E3_49C2_BB0D_58CE8C77E14C__INCLUDED_)
#define AFX_MPSKINNUMINFOCTRL_H__9FFF7438_90E3_49C2_BB0D_58CE8C77E14C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinPicText.h"

class CMPSkinMediaNumInfoCtrl : public CSkinPicText, IEventHandler 
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinPicText)
public:
    CMPSkinMediaNumInfoCtrl();
    virtual ~CMPSkinMediaNumInfoCtrl();

    void onCreate();

    void onEvent(const IEvent *pEvent);

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    enum INFO_TYPE
    {
        IT_BITRATE,
        IT_SAMPLERATE,
    };

protected:
    void updateShowTrackInfo();

protected:
    INFO_TYPE        m_infoType;

};

#endif // !defined(AFX_MPSKINNUMINFOCTRL_H__9FFF7438_90E3_49C2_BB0D_58CE8C77E14C__INCLUDED_)
