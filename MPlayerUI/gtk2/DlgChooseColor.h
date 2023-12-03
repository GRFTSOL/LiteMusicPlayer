#pragma once

#ifndef MPlayerUI_gtk2_DlgChooseColor_h
#define MPlayerUI_gtk2_DlgChooseColor_h


class CDlgChooseColor {
public:
    CDlgChooseColor();
    virtual ~CDlgChooseColor();

    int doModal(Window *pWndParent, const CColor &clr);

    CColor &getColor() { return m_clr; }

public:
    CColor                      m_clr;
    GtkWidget                   *m_window;

};

#endif // !defined(MPlayerUI_gtk2_DlgChooseColor_h)
