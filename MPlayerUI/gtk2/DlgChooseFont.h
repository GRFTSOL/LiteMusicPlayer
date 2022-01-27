// DlgChooseFont.h: interface for the CDlgChooseFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGCHOOSEFONT_H__53DFBF2E_6C3A_4BC6_B059_FEB4814E504A__INCLUDED_)
#define AFX_DLGCHOOSEFONT_H__53DFBF2E_6C3A_4BC6_B059_FEB4814E504A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDlgChooseFont
{
public:
    CDlgChooseFont();
    virtual ~CDlgChooseFont();

    int doModal(Window *pWndParent, cstr_t szFontFaceName, int nFontSize, int nWeight, int nItalic);

    cstr_t getFaceName();

    int getSize();

    int getWeight();

    int getItalic();

public:
    string            m_strFontFaceName;
    int                m_nFontSize, m_weight;
    int                m_nItalic;

    GtkWidget        *m_window;

};

#endif // !defined(AFX_DLGCHOOSEFONT_H__53DFBF2E_6C3A_4BC6_B059_FEB4814E504A__INCLUDED_)
