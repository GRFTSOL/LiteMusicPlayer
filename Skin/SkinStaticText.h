#pragma once

/********************************************************************
    Created  :    2002/01/04    21:42
    FileName :    SkinStaticText.h
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#ifndef Skin_SkinStaticText_h
#define Skin_SkinStaticText_h


class CSkinStaticText : public CUIObject {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinStaticText();
    virtual ~CSkinStaticText();

    void draw(CRawGraph *canvas) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void onCreate() override;

    void onMeasureSizeByContent() override;

    void onAdjustHue(float hue, float saturation, float luminance) override;

    bool onLButtonUp(uint32_t nFlags, CPoint point) override;

protected:
    uint32_t                    m_dwAlignText;
    bool                        m_bCaptionText;

    int                         m_nLeftMargin;

    CSkinFontProperty           m_font;

};

#endif // !defined(Skin_SkinStaticText_h)
