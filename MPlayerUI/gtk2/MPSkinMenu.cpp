// MPSkinMenu.cpp: implementation of the CMPSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "MPSkinMenu.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"
#include "CharsetEncoding.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPSkinMenu::CMPSkinMenu()
{

}

CMPSkinMenu::~CMPSkinMenu()
{

}

void CMPSkinMenu::onLoadMenu()
{
}

void CMPSkinMenu::trackPopupMenu(int x, int y, Window *pWnd, CRect *prcNotOverlap)
{

    CSkinMenu::trackPopupMenu(x, y, pWnd);
}

void CMPSkinMenu::trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap)
{
    CSkinMenu::trackPopupSubMenu(x, y, nSubMenu, pWnd, prcNotOverlap);
}
