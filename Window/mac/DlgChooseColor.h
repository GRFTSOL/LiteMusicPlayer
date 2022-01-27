#pragma once

#include "Window.h"
#include "../../GfxRaw/Color.h"


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
