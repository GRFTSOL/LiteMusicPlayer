// MPPPCWorkArea.cpp: implementation of the CMPPPCWorkArea class.
//
//////////////////////////////////////////////////////////////////////

#include "MPPPCWorkArea.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPPPCWorkArea, "WorkArea")

CMPPPCWorkArea::CMPPPCWorkArea()
{
}

CMPPPCWorkArea::~CMPPPCWorkArea()
{
}

bool CMPPPCWorkArea::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (CUIObject::setProperty(szProperty, szValue))
        return true;

    if (strcasecmp(szProperty, "DivideImage") == 0)
    {
        m_strDivideImgFile = szValue;
    }
    else if (strcasecmp(szProperty, "DivideImageRect") == 0)
    {
        if (!getRectValue(szValue, m_imgDivide))
            ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
    }
    else if (strcasecmp(szProperty, "DivideImageBltMode") == 0)
    {
        m_strDivideImgBltMode = szValue;
    }
    else
    {
        for (int i = 0; i < m_vProperties.size(); i += 2)
        {
            if (strcasecmp(m_vProperties[i].c_str(), szProperty) == 0)
            {
                m_vProperties[i + 1] = szValue;
                return true;
            }
        }

        m_vProperties.push_back(szProperty);
        m_vProperties.push_back(szValue);
    }

    return true;
}

void CMPPPCWorkArea::enumProperties(CUIObjProperties &listProperties)
{
    CUIObject::enumProperties(listProperties);

    listProperties.addPropImage("DivideImage", "DivideImageRect", m_strDivideImgFile.c_str(), m_imgDivide, !m_strDivideImgFile.empty());
    listProperties.addPropBltMode("DivideImageBltMode", bltModeFromStr(m_strDivideImgBltMode.c_str()));

    for (int i = 0; i < m_vProperties.size(); i += 2)
    {
        listProperties.addPropStr(m_vProperties[i].c_str(), m_vProperties[i + 1].c_str());
    }
}