// MPSkinMainWnd.cpp: implementation of the CMPSkinMainWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "../MPlayerApp.h"
#include "../MPSkinMainWnd.h"
#include "MPSkinMainWndGtk2.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPSkinMainWnd::CMPSkinMainWnd()
{
}

CMPSkinMainWnd::~CMPSkinMainWnd()
{
}

bool CMPSkinMainWnd::onCreate()
{
    if (!CMPSkinMainWndBase::onCreate())
        return false;

    return true;
}

void CMPSkinMainWnd::onDestroy()
{
    CMPSkinMainWndBase::onDestroy();
}

void CMPSkinMainWnd::onEvent(const IEvent *pEvent)
{
    CMPSkinWnd::onEvent(pEvent);
}

