// MPSkinMenu.h: interface for the CMPSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_)
#define AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "SkinMenu.h"

class CMPSkinMenu : public CSkinMenu  
{
public:
    CMPSkinMenu();
    virtual ~CMPSkinMenu();

    virtual void trackPopupMenu(int x, int y, Window *pWnd, CRect *prcNotOverlap = nullptr);
    virtual void trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap = nullptr);

    virtual void onLoadMenu();

    void addUICheckStatusIf(IUICheckStatus *pIfUICheckStatus) {  }

protected:

};

#endif // !defined(AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_)
