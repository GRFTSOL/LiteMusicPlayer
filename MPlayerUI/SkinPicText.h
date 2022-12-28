#pragma once

#ifndef MPlayerUI_SkinPicText_h
#define MPlayerUI_SkinPicText_h


class CSkinPicText : public CUIObject {
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
    string                      m_strImgText;
    CSFImage                    m_imgText;
    int                         m_nCharWidth;

    string                      m_strImgChars;
    int                         m_nSpaceCharPos;

    uint32_t                    m_dwAlignTextFlags;

    vector<int>                 m_vPos;

};

#endif // !defined(MPlayerUI_SkinPicText_h)
