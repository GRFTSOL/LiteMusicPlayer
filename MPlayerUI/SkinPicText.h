// SkinPicText.h: interface for the CSkinPicText class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINPICTEXT_H__4D559DA1_FA33_4D9E_83F8_42C34705D79C__INCLUDED_)
#define AFX_SKINPICTEXT_H__4D559DA1_FA33_4D9E_83F8_42C34705D79C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinPicText : public CUIObject  
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject);
public:
    CSkinPicText();
    virtual ~CSkinPicText();

    void draw(CRawGraph *canvas) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void setText(cstr_t szText) override;

protected:
    string            m_strImgText;
    CSFImage        m_imgText;
    int                m_nCharWidth;

    string            m_strImgChars;
    int                m_nSpaceCharPos;

    uint32_t            m_dwAlignTextFlags;

    vector<int>        m_vPos;

};

#endif // !defined(AFX_SKINPICTEXT_H__4D559DA1_FA33_4D9E_83F8_42C34705D79C__INCLUDED_)
