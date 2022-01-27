// SkinFadeArea.cpp: implementation of the CSkinFadeArea class.
//
//////////////////////////////////////////////////////////////////////

#include "SkinTypes.h"
#include "Skin.h"
#include "SkinFadeArea.h"


UIOBJECT_CLASS_NAME_IMP(CSkinFadeArea, "FadeArea")


CSkinFadeArea::CSkinFadeArea()
{
}


CSkinFadeArea::~CSkinFadeArea()
{
}


void CSkinFadeArea::draw(CRawGraph *canvas)
{
    //
    // Adjust the alpha of the eara that CSkinFadeArea covers.
    //
    if (!m_pSkin->getEnableTranslucencyLayered())
        return;

    if ((int)m_pSkin->m_nCurTranslucencyAlpha == m_pSkin->m_nTranslucencyAlphaOnActive)
        return;

    canvas->multiplyAlpha((uint8_t)m_pSkin->m_nCurTranslucencyAlpha, &m_rcObj);
}
