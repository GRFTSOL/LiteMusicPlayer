// SkinTextButton.h: interface for the CSkinTextButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINTEXTBUTTON_H__86F68037_E86D_4927_8B88_47034765E7D0__INCLUDED_)
#define AFX_SKINTEXTBUTTON_H__86F68037_E86D_4927_8B88_47034765E7D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinTextButton : public CSkinButton
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)
public:
    CSkinTextButton();
    virtual ~CSkinTextButton();

    void draw(CRawGraph *canvas);

    virtual void onCreate();

    virtual bool onMenuKey(uint32_t nChar, uint32_t nFlags);

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    virtual void setText(cstr_t szText);

    virtual void onLanguageChanged();

protected:
    uint32_t                m_dwAlignTextFlags;
    bool                m_bResizeToContent;
    CSFImage            m_imgContent;

    int                    m_chMenuKey;    // Menu Key: Alt + VK_X to execute it.

    int                    m_nTextLeftMargin, m_nTextRightMargin;

    CSkinFontProperty    m_font;

};

#endif // !defined(AFX_SKINTEXTBUTTON_H__86F68037_E86D_4927_8B88_47034765E7D0__INCLUDED_)
