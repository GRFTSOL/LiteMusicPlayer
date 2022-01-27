#include "SkinTypes.h"
#include "Skin.h"
#include "SkinDataObj.h"


UIOBJECT_CLASS_NAME_IMP(CSkinDataObj, "DataObj")

CSkinDataObj::CSkinDataObj()
{
    m_visible = false;
}


CSkinDataObj::~CSkinDataObj()
{
}


bool CSkinDataObj::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (isPropertyName(szProperty, SZ_PN_ID))
    {
        m_id = m_pSkin->getSkinFactory()->getIDByName(szValue);
    }
    else if (isPropertyName(szProperty, SZ_PN_NAME))
        m_strName = szValue;
    else
        return false;

    return true;
}


#ifdef _SKIN_EDITOR_
void CSkinDataObj::enumProperties(CUIObjProperties &listProperties)
{
    listProperties.addPropStr(SZ_PN_NAME, m_strName.c_str(), !m_strName.empty());

    listProperties.addPropID(SZ_PN_ID, m_pSkin->getSkinFactory()->getStringOfID(m_id).c_str(), m_id != UID_INVALID);
}
#endif // _SKIN_EDITOR_
