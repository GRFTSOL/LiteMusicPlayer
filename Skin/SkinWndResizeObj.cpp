#include "SkinTypes.h"
#include "Skin.h"
#include "SkinWndResizeObj.h"

UIOBJECT_CLASS_NAME_IMP(CSkinWndResizeObj, "WndResizer");

CSkinWndResizeObj::CSkinWndResizeObj(void)
{
    m_nResizeDirection = 0;
}

CSkinWndResizeObj::~CSkinWndResizeObj(void)
{
    m_pSkin->m_wndResizer.removeResizeArea(m_autoUniID);
}

IdToString    __idszResizeDirection[] = 
{
    { WndResizer::RD_LEFT, "left" },
    { WndResizer::RD_TOP, "top" },
    { WndResizer::RD_RIGHT, "right" },
    { WndResizer::RD_BOTTOM, "bottom" },
    { 0, nullptr }
};

bool CSkinWndResizeObj::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (CUIObject::setProperty(szProperty, szValue))
        return true;

    if (isPropertyName(szProperty, "Direction"))
        m_nResizeDirection = getCombinationValue(__idszResizeDirection, szValue);
    else
        return false;

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinWndResizeObj::enumProperties(CUIObjProperties &listProperties)
{
    CUIObject::enumProperties(listProperties);


    string        str;

    getCombinationStrValue(__idszResizeDirection, m_nResizeDirection, str);
    listProperties.addPropStr("Direction", str.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinWndResizeObj::onSize()
{
    CUIObject::onSize();

    m_pSkin->m_wndResizer.setResizeArea(m_autoUniID, m_nResizeDirection, m_rcObj);
}
