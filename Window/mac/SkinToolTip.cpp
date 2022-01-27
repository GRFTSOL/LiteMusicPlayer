// SkinToolTip.cpp: implementation of the CSkinToolTip class.
//
//////////////////////////////////////////////////////////////////////

#include "../WindowTypes.h"
#include "SkinToolTip.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinToolTip::CSkinToolTip()
{
    m_hWnd = nullptr;
}

CSkinToolTip::~CSkinToolTip()
{

}

bool CSkinToolTip::create(Window *pWndParent)
{
    return true;
}

bool CSkinToolTip::addTool(cstr_t szText, CRect *lpRectTool, uint32_t nIDTool)
{
    return true;
}

void CSkinToolTip::delTool(uint32_t nIDTool)
{
}

void CSkinToolTip::destroy()
{
}

