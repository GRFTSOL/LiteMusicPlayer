#pragma once


class CDlgChooseColor {
public:
    CDlgChooseColor();
    virtual ~CDlgChooseColor();

    int doModal(Window *pWndParent, const CColor &clr);

    CColor &getColor() { return m_clr; }

public:
    CColor                      m_clr;

};
