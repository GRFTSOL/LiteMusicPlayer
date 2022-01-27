// MediaInfoTextCtrl.cpp: implementation of the CMediaInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "MediaInfoTextCtrl.h"

IdToString __mediaInfoType[] = 
{
    { MA_ARTIST, "Artist" },
    { MA_TITLE, "Title" },
    { MA_ALBUM, "Album" },
    { MA_TRACK_NUMB, "Track" },
    { MA_YEAR, "Year" },
    { MA_GENRE, "Genre" },
    { MA_BITRATE, "Bit rate" },
    { MA_SAMPLE_RATE, "Sample rate" },
    { 0, nullptr },
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMediaInfoTextCtrl, "MediaInfoText")

CMediaInfoTextCtrl::CMediaInfoTextCtrl()
{
    m_nSwitchTime = 5000;
    m_nCurrentItem = 0;
    m_bShowTitle = true;
}

CMediaInfoTextCtrl::~CMediaInfoTextCtrl()
{
    m_pSkin->unregisterTimerObject(this);
}

void CMediaInfoTextCtrl::onCreate()
{
    CSkinScrollText::onCreate();

    if (m_vInfoType.size() > 1)
        m_pSkin->registerTimerObject(this, m_nSwitchTime);

    updateShowTrackInfo();
}

void CMediaInfoTextCtrl::onTimer(int nId)
{
    if (nId == m_nIDTimerUpdateTrackInfo)
    {
        m_nCurrentItem++;
        if (m_nCurrentItem >= m_vInfoType.size())
            m_nCurrentItem = 0;
        updateShowTrackInfo();
        invalidate();
    }
    else
        CSkinScrollText::onTimer(nId);
}

bool CMediaInfoTextCtrl::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (CSkinScrollText::setProperty(szProperty, szValue))
        return true;

    if (strcasecmp(szProperty, "MediaInfoType") == 0)
    {
        VecStrings        vStr;

        m_vInfoType.clear();
        StrBreak(szValue, '|', vStr);
        for (int i = 0; i < vStr.size(); i++)
        {
            trimStr(vStr[i]);
            m_vInfoType.push_back((MediaAttribute)stringToID(__mediaInfoType, vStr[i].c_str(), MA_TITLE));
        }
    }
    else if (strcasecmp(szProperty, "SwitchTime") == 0)
        m_nSwitchTime = atoi(szValue);
    else if (strcasecmp(szProperty, "ShowTitle") == 0)
        m_bShowTitle = isTRUE(szValue);
    else
        return false;

    return true;
}

void CMediaInfoTextCtrl::enumProperties(CUIObjProperties &listProperties)
{
    CSkinScrollText::enumProperties(listProperties);

    CUIObjProperty    prop;
    int                i;

    prop.valueType = CUIObjProperty::VT_COMB_STR;
    prop.name = "MediaInfoType";
    for (i = 0; i < m_vInfoType.size(); i++)
    {
        prop.strValue += iDToString(__mediaInfoType, m_vInfoType[i], "Title");
        prop.strValue += "|";
    }
    if (!prop.strValue.empty())
        prop.strValue.resize(prop.strValue.size() - 1);

    for (i = 0; __mediaInfoType[i].szId != nullptr; i++)
        prop.options.push_back(__mediaInfoType[i].szId);
    listProperties.push_back(prop);

    listProperties.addPropInt("SwitchTime", m_nSwitchTime, true);
    listProperties.addPropBoolStr("ShowTitle", m_bShowTitle, !m_bShowTitle);
}

void CMediaInfoTextCtrl::updateShowTrackInfo()
{
    if (m_vInfoType.empty())
        return;

    if (m_nCurrentItem < 0 || m_nCurrentItem >= m_vInfoType.size())
        m_nCurrentItem = 0;
    MediaAttribute        infoType = m_vInfoType[m_nCurrentItem];

    m_strText = iDToString(__mediaInfoType, infoType, "");
    m_strText += ": ";
}
