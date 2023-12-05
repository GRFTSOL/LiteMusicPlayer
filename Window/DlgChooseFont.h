#pragma once

class CDlgChooseFont {
public:
    CDlgChooseFont();
    virtual ~CDlgChooseFont();

    int doModal(Window *pWndParent, cstr_t szFontFaceName, int nFontSize, int nWeight, int nItalic);

    cstr_t getFaceName();

    int getSize();

    int getWeight();

    int getItalic();

public:
    string                      m_strFontFaceName;
    int                         m_nFontSize, m_weight;
    int                         m_nItalic;

};
