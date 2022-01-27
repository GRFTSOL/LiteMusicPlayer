/********************************************************************
    Created  :    2002/01/04    21:42
    FileName :    SkinStaticText.h
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#if !defined(AFX_SKINSTATICTEXT_H__7ED53727_726A_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_SKINSTATICTEXT_H__7ED53727_726A_11D5_9E04_02608CAD9330__INCLUDED_


class CSkinStaticText : public CUIObject  
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinStaticText();
    virtual ~CSkinStaticText();

    void draw(CRawGraph *canvas);

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void onCreate();

    void onMeasureSizeByContent();

    void onAdjustHue(float hue, float saturation, float luminance);

    bool onLButtonUp(uint32_t nFlags, CPoint point);

protected:
    uint32_t            m_dwAlignText;
    bool            m_bCaptionText;

    int                m_nLeftMargin;

    CSkinFontProperty    m_font;

};

#endif // !defined(AFX_SKINSTATICTEXT_H__7ED53727_726A_11D5_9E04_02608CAD9330__INCLUDED_)
