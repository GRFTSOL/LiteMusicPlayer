#pragma once

class CSkinTxtLink : public CUIObject {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinTxtLink();
    virtual ~CSkinTxtLink();

    void draw(CRawGraph *canvas) override;
    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void onCreate() override;

    void onSize() override;

    bool onLButtonUp(uint32_t nFlags, CPoint point) override;

    bool onLButtonDown(uint32_t nFlags, CPoint point) override;

    bool onMouseMove(CPoint point) override;

protected:
    string                      m_strLink;
    uint32_t                    m_dwAlignText;
    CSkinFontProperty           m_font;
    Cursor                      m_cursor;
    bool                        m_bNeedResize;

};
