#pragma once

#ifndef Window_win32_DlgChooseColor_h
#define Window_win32_DlgChooseColor_h


class CDlgChooseColor {
public:
    CDlgChooseColor();
    virtual ~CDlgChooseColor();

    int doModal(Window *pWndParent, const CColor &clr);

    CColor &getColor() { return m_clr; }

public:
    CColor                      m_clr;

};

#endif // !defined(Window_win32_DlgChooseColor_h)
