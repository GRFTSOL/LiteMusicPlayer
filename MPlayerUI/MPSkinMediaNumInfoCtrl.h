#pragma once

#ifndef MPlayerUI_MPSkinMediaNumInfoCtrl_h
#define MPlayerUI_MPSkinMediaNumInfoCtrl_h


#include "SkinPicText.h"


class CMPSkinMediaNumInfoCtrl : public CSkinPicText, IEventHandler {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinPicText)
public:
    CMPSkinMediaNumInfoCtrl();
    virtual ~CMPSkinMediaNumInfoCtrl();

    void onCreate() override;

    void onEvent(const IEvent *pEvent) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    enum INFO_TYPE {
        IT_BITRATE,
        IT_SAMPLERATE,
    };

protected:
    void updateShowTrackInfo();

protected:
    INFO_TYPE                   m_infoType;

};

#endif // !defined(MPlayerUI_MPSkinMediaNumInfoCtrl_h)
