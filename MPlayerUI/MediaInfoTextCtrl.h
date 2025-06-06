﻿#pragma once

#include "../Skin/SkinScrollText.h"


class CMediaInfoTextCtrl : public CSkinScrollText, public IEventHandler {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinScrollText)
public:
    CMediaInfoTextCtrl();
    virtual ~CMediaInfoTextCtrl();

    void onCreate() override;

    virtual void onEvent(const IEvent *pEvent) override;

    void onTimer(int nId) override;

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    void updateShowTrackInfo();

    string getMediaAttrInfo(MediaAttribute infoType);

    vector<MediaAttribute>      m_vInfoType;
    int                         m_nSwitchTime;
    int                         m_nCurrentItem;
    bool                        m_bShowTitle;
    int                         m_nIDTimerUpdateTrackInfo;

    // If CombineWith exists, all information will be combined with it.
    string                      m_strCombineWith;

};
