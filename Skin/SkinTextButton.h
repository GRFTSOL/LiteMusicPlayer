#pragma once

#ifndef Skin_SkinTextButton_h
#define Skin_SkinTextButton_h


class CSkinTextButton : public CSkinButton {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)
public:
    CSkinTextButton();
    virtual ~CSkinTextButton();

    void draw(CRawGraph *canvas) override;

    virtual void onCreate() override;

    virtual bool onMenuKey(uint32_t nChar, uint32_t nFlags) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    virtual void setText(cstr_t szText) override;

    virtual void onLanguageChanged() override;

protected:
    uint32_t                    m_dwAlignTextFlags;
    bool                        m_bResizeToContent;
    CSFImage                    m_imgContent;

    int                         m_chMenuKey;        // Menu Key: Alt + VK_X to execute it.

    int                         m_nTextLeftMargin, m_nTextRightMargin;

    CColor                      m_clrTextSelected;
    CSkinFontProperty           m_font;

};

#endif // !defined(Skin_SkinTextButton_h)
