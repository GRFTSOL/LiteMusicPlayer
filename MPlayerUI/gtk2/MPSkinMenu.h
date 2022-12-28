#pragma once

#ifndef MPlayerUI_gtk2_MPSkinMenu_h
#define MPlayerUI_gtk2_MPSkinMenu_h



#include "SkinMenu.h"


class CMPSkinMenu : public CSkinMenu {
public:
    CMPSkinMenu();
    virtual ~CMPSkinMenu();

    virtual void trackPopupMenu(int x, int y, Window *pWnd, CRect *prcNotOverlap = nullptr);
    virtual void trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap = nullptr);

    virtual void onLoadMenu();

    void addUICheckStatusIf(IUICheckStatus *pIfUICheckStatus) {  }

protected:

};

#endif // !defined(MPlayerUI_gtk2_MPSkinMenu_h)
