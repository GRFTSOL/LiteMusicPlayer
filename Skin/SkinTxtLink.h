
#pragma once

class CSkinTxtLink : public CUIObject  
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinTxtLink();
    virtual ~CSkinTxtLink();

    void draw(CRawGraph *canvas);
    bool setProperty(cstr_t szProperty, cstr_t szValue);

#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void onCreate();

    void onSize();

    bool onLButtonUp(uint32_t nFlags, CPoint point);

    bool onLButtonDown(uint32_t nFlags, CPoint point);

    bool onMouseMove(CPoint point);

protected:
    string                m_strLink;
    uint32_t                m_dwAlignText;
    CSkinFontProperty    m_font;
    Cursor                m_Cursor;
    bool                m_bNeedResize;

};
