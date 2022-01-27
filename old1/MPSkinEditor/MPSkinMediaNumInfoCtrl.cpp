// MPSkinNumInfoCtrl.cpp: implementation of the CMPSkinMediaNumInfoCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "MPSkinMediaNumInfoCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPSkinMediaNumInfoCtrl, "NumInfo")

CMPSkinMediaNumInfoCtrl::CMPSkinMediaNumInfoCtrl()
{
    m_infoType = IT_BITRATE;
}

CMPSkinMediaNumInfoCtrl::~CMPSkinMediaNumInfoCtrl()
{
}

void CMPSkinMediaNumInfoCtrl::onCreate()
{
    CSkinPicText::onCreate();

    updateShowTrackInfo();
}

bool CMPSkinMediaNumInfoCtrl::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (CSkinPicText::setProperty(szProperty, szValue))
        return true;

    if (strcasecmp(szProperty, "InfoType") == 0)
    {
        if (strcasecmp(szValue, "kbps") == 0)
            m_infoType = IT_BITRATE;
        else
            m_infoType = IT_SAMPLERATE;

        updateShowTrackInfo();
    }
    else
        return false;

    return true;
}

void CMPSkinMediaNumInfoCtrl::enumProperties(CUIObjProperties &listProperties)
{
    CSkinPicText::enumProperties(listProperties);

    CUIObjProperty        prop;

    prop.name = "InfoType";
    if (m_infoType == IT_BITRATE)
        prop.strValue = "kbps";
    else
        prop.strValue = "samplerate";
    prop.valueType = CUIObjProperty::VT_COMB_STR;
    prop.options.push_back("kbps");
    prop.options.push_back("samplerate");
    listProperties.push_back(prop);
}

void CMPSkinMediaNumInfoCtrl::updateShowTrackInfo()
{
    if (m_infoType == IT_SAMPLERATE)
        setText("44100");
    else
        setText("192");
}
