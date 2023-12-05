#include "MPlayerApp.h"
#include "MediaInfoTextCtrl.h"


IdToString __mediaInfoType[] = {
    { MA_ARTIST, "Artist" },
    { MA_TITLE, "Title" },
    { MA_ALBUM, "Album" },
    { MA_TRACK_NUMB, "Track" },
    { MA_YEAR, "Year" },
    { MA_GENRE, "Genre" },
    { MA_BITRATE, "Bit rate" },
    { MA_SAMPLE_RATE, "Sample rate" },
    { MA_CHANNELS, "Channels" },
    { MA_FORMAT, "File Format" },
    { MA_FILESIZE, "File size" },
    { MA_DURATION, "Duration" },
    { 0, nullptr },
};


void formatFloatStr(double value, int nPrecision, string & out) {
    char szBuf[64];
    char szFormat[64];

    snprintf(szFormat, CountOf(szFormat), "%%.%df", nPrecision);
    snprintf(szBuf, CountOf(szBuf), szFormat, value);

    // remove '0' from end. remove decimal point if the digit after it is 0.
    trimStrRight(szBuf, "0");
    trimStrRight(szBuf, ".");

    out = szBuf;
}



UIOBJECT_CLASS_NAME_IMP(CMediaInfoTextCtrl, "MediaInfoText")

CMediaInfoTextCtrl::CMediaInfoTextCtrl() {
    m_nSwitchTime = 5000;
    m_nCurrentItem = 0;
    m_nIDTimerUpdateTrackInfo = 0;
    m_bShowTitle = false;
}

CMediaInfoTextCtrl::~CMediaInfoTextCtrl() {
    m_pSkin->unregisterTimerObject(this);
}

void CMediaInfoTextCtrl::onCreate() {
    CSkinScrollText::onCreate();

    registerHandler(MPlayerApp::getEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_CUR_MEDIA_INFO_CHANGED);

    if (m_vInfoType.size() > 1 && m_strCombineWith.empty()) {
        m_nIDTimerUpdateTrackInfo = m_pSkin->registerTimerObject(this, m_nSwitchTime);
    }

    updateShowTrackInfo();
}

void CMediaInfoTextCtrl::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED) {
        updateShowTrackInfo();
        invalidate();
    }
}

void CMediaInfoTextCtrl::onTimer(int nId) {
    if (nId == m_nIDTimerUpdateTrackInfo) {
        m_nCurrentItem++;
        if (m_nCurrentItem >= (int)m_vInfoType.size()) {
            m_nCurrentItem = 0;
        }
        updateShowTrackInfo();
        invalidate();
    } else {
        CSkinScrollText::onTimer(nId);
    }
}

bool CMediaInfoTextCtrl::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinScrollText::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "MediaInfoType") == 0) {
        VecStrings vStr;

        m_vInfoType.clear();
        strSplit(szValue, '|', vStr);

        for (int i = 0; i < (int)vStr.size(); i++) {
            trimStr(vStr[i]);
            m_vInfoType.push_back((MediaAttribute)stringToID(__mediaInfoType, vStr[i].c_str(), MA_TITLE));
        }
    } else if (strcasecmp(szProperty, "CombineWith") == 0) {
        m_strCombineWith = szValue;
    } else if (strcasecmp(szProperty, "SwitchTime") == 0) {
        m_nSwitchTime = atoi(szValue);
    } else if (strcasecmp(szProperty, "ShowTitle") == 0) {
        m_bShowTitle = isTRUE(szValue);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CMediaInfoTextCtrl::enumProperties(CUIObjProperties &listProperties) {
    CSkinScrollText::enumProperties(listProperties);

    CUIObjProperty prop;
    int i;

    prop.valueType = CUIObjProperty::VT_COMB_STR;
    prop.name = "MediaInfoType";
    for (i = 0; i < (int)m_vInfoType.size(); i++) {
        prop.strValue += idToString(__mediaInfoType, m_vInfoType[i], "Title");
        prop.strValue += "|";
    }
    if (!prop.strValue.empty()) {
        prop.strValue.resize(prop.strValue.size() - 1);
    }

    for (i = 0; __mediaInfoType[i].szId != nullptr; i++) {
        prop.options.push_back(__mediaInfoType[i].szId);
    }
    listProperties.push_back(prop);

    listProperties.addPropInt("SwitchTime", m_nSwitchTime, true);
    listProperties.addPropBoolStr("ShowTitle", m_bShowTitle, !m_bShowTitle);
}
#endif // _SKIN_EDITOR_

void CMediaInfoTextCtrl::updateShowTrackInfo() {
    if (m_vInfoType.empty()) {
        return;
    }

    string strText;

    if (m_strCombineWith.empty()) {
        // Display media information timely
        if (m_nCurrentItem < 0 || m_nCurrentItem >= (int)m_vInfoType.size()) {
            m_nCurrentItem = 0;
        }

        if (m_bShowTitle) {
            strText = _TL(idToString(__mediaInfoType, m_vInfoType[m_nCurrentItem], ""));
            strText += ": ";
        }
        strText += getMediaAttrInfo(m_vInfoType[m_nCurrentItem]);
    } else {
        for (uint32_t i = 0; i < m_vInfoType.size(); i++) {
            if (strText.size() > 0) {
                strText += m_strCombineWith;
            }
            strText += getMediaAttrInfo(m_vInfoType[i]);
        }
    }

    setText(strText.c_str());
}

string CMediaInfoTextCtrl::getMediaAttrInfo(MediaAttribute infoType) {
    string strValue;

    g_player.getCurMediaAttribute(infoType, strValue);
    if (infoType == MA_CHANNELS) {
        // Channel info should be deal specially
        int nChannels = atoi(strValue.c_str());
        if (nChannels == 1) {
            strValue = _TLT("Mono");
        } else {
            strValue = _TLT("Stereo");
        }
    } else if (infoType == MA_BITRATE) {
        // Convert bitrate to Kbps
        formatFloatStr(atoi(strValue.c_str()) / 1000.0, 1, strValue);
        strValue += "Kbps";
    } else if (infoType == MA_FILESIZE) {
        // Convert file size to MB
        formatFloatStr(atoi(strValue.c_str()) / (1024.0 * 1024), 1, strValue);
        strValue += "M";
    }

    return strValue;
}
