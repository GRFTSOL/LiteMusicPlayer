#include "MPlayerAppBase.h"
#include "MPSkinMediaNumInfoCtrl.h"


UIOBJECT_CLASS_NAME_IMP(CMPSkinMediaNumInfoCtrl, "NumInfo")

CMPSkinMediaNumInfoCtrl::CMPSkinMediaNumInfoCtrl() {
    m_infoType = IT_BITRATE;
}

CMPSkinMediaNumInfoCtrl::~CMPSkinMediaNumInfoCtrl() {
}

void CMPSkinMediaNumInfoCtrl::onCreate() {
    CSkinPicText::onCreate();

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_CUR_MEDIA_INFO_CHANGED);

    updateShowTrackInfo();
}

void CMPSkinMediaNumInfoCtrl::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED) {
        updateShowTrackInfo();
        invalidate();
    }
}

bool CMPSkinMediaNumInfoCtrl::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinPicText::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "InfoType") == 0) {
        if (strcasecmp(szValue, "kbps") == 0) {
            m_infoType = IT_BITRATE;
        } else {
            m_infoType = IT_SAMPLERATE;
        }
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CMPSkinMediaNumInfoCtrl::enumProperties(CUIObjProperties &listProperties) {

}
#endif // _SKIN_EDITOR_

void CMPSkinMediaNumInfoCtrl::updateShowTrackInfo() {
    CMPAutoPtr<IMedia> pMedia;
    int value = 0;
    char szText[64];

    if (g_Player.getCurrentMedia(&pMedia) != ERR_OK) {
        setText("");
        return;
    }

    if (m_infoType == IT_BITRATE) {
        pMedia->getAttribute(MA_BITRATE, &value);
        value /= 1000;
    } else {
        pMedia->getAttribute(MA_SAMPLE_RATE, &value);
    }

    sprintf(szText, "%d", value);
    setText(szText);
}
