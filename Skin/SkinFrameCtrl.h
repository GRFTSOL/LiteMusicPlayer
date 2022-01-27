
#pragma once

#include "UIObject.h"

class CSkinFrameCtrl : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinFrameCtrl();
    virtual ~CSkinFrameCtrl();

    void onCreate();

    void onAdjustHue(float hue, float saturation, float luminance);

    void draw(CRawGraph *canvas);
    bool setProperty(cstr_t szProperty, cstr_t szValue);

    int getBorderThick() { return m_nThickLeft; }

    void setFocusIndicator(CUIObject *pObjFocusIndicator) { m_pObjFocusIndicator = pObjFocusIndicator; }

protected:
    CSFImage            m_image, m_imageFocus;
    BlendPixMode        m_bpm;
    int16_t                m_nRoundWidthTop, m_nRoundWidthBottom;
    int16_t                m_nThickLeft, m_nThickTop, m_nThickRight, m_nThickBottom;
    int16_t                m_xBorder, m_yBorder;
    CSkinFontProperty    m_font;
    CUIObject            *m_pObjFocusIndicator;

};
