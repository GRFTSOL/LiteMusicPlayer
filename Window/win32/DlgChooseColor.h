// DlgChooseColor.h: interface for the CDlgChooseColor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGCHOOSECOLOR_H__4F9B7780_1940_41A6_974A_8DADF6A5EEDC__INCLUDED_)
#define AFX_DLGCHOOSECOLOR_H__4F9B7780_1940_41A6_974A_8DADF6A5EEDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDlgChooseColor
{
public:
    CDlgChooseColor();
    virtual ~CDlgChooseColor();

    int doModal(Window *pWndParent, const CColor &clr);

    CColor &getColor() { return m_clr; }

public:
    CColor            m_clr;

};

#endif // !defined(AFX_DLGCHOOSECOLOR_H__4F9B7780_1940_41A6_974A_8DADF6A5EEDC__INCLUDED_)
