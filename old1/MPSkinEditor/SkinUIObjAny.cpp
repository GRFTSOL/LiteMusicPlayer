// SkinUIObjAny.cpp: implementation of the CSkinUIObjAny class.
//
//////////////////////////////////////////////////////////////////////

#include "SkinUIObjAny.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinUIObjAny, "UIObjAny")

CSkinUIObjAny::CSkinUIObjAny()
{
    m_msgNeed = UO_MSG_WANT_ALL;
}

CSkinUIObjAny::~CSkinUIObjAny()
{

}

void CSkinUIObjAny::draw(CRawGraph *canvas)
{
}

bool CSkinUIObjAny::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (CUIObject::setProperty(szProperty, szValue))
        return true;

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

    return true;
}

void CSkinUIObjAny::enumProperties(CUIObjProperties &listProperties)
{
    CUIObjProperty        property;

    CUIObject::enumProperties(listProperties);

    for (int i = 0; i < m_vProperties.size(); i += 2)
    {
        property.name = m_vProperties[i].c_str();
        property.strValue = m_vProperties[i + 1].c_str();;
        property.valueType = CUIObjProperty::VT_STR;
        listProperties.push_back(property);
    }
}
