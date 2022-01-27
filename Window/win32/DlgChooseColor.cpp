// DlgChooseColor.cpp: implementation of the CDlgChooseColor class.
//
//////////////////////////////////////////////////////////////////////

#include "BaseWnd.h"
#include "DlgChooseColor.h"

CDlgChooseColor::CDlgChooseColor()
{
    m_clr = RGB(255, 255, 255);
}

CDlgChooseColor::~CDlgChooseColor()
{

}

int CDlgChooseColor::doModal(Window *pWndParent, const CColor &clr)
{
    static COLORREF     _clrCustom[16];

    m_clr = clr;

    CHOOSECOLOR choosecolor;
    choosecolor.lStructSize = sizeof(choosecolor);
    choosecolor.hwndOwner = pWndParent->getHandle();
    choosecolor.lpCustColors = _clrCustom;
    choosecolor.Flags = CC_RGBINIT;
    choosecolor.rgbResult = clr.get();
    if (ChooseColor(&choosecolor))
    {
        m_clr.set(choosecolor.rgbResult);
        return IDOK;
    }

    return IDCANCEL;
}
