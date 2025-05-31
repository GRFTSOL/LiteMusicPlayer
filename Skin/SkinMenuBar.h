#pragma once

#if !defined(_SKIN_MENU_BAR_H_)
#define _SKIN_MENU_BAR_H_

#include "UIObject.h"
#include "SkinMenu.h"
#include "SkinMenuItemsContainer.hpp"


class CSkinMenuBar : public CUIObject, public IMenuEventNotify {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinMenuBar();
    virtual ~CSkinMenuBar();

    void onCreate() override;

    void draw(CRawGraph *canvas) override;

    bool onLButtonDown(uint32_t nFlags, CPoint point) override;
    bool onMouseMove(CPoint point) override;
    void onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) override;
    bool onKeyDown(uint32_t nChar, uint32_t nFlags) override;
    bool onMenuKey(uint32_t nChar, uint32_t nFlags) override;
    void onKillFocus() override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void onLanguageChanged() override;

    void onPopupMenuClosed() override;
    void onPopupKeyDown(uint32_t nChar, uint32_t nFlags) override;

    int getPopupTopY() override;

protected:
    int hitTestSubMenu(const CPoint point);
    int getItemX(int n);

    void onMenuItemSelected(int index);
    void hideSubMenu();
    bool isMenuPopup();

    friend class CMenuBarMsgHandler;

protected:
    struct SubMenu : public MenuItemInfo {
        int                         nWidth;
        int                         chMenuKey;          // Menu Key: Alt + VK_X to popup associated menu.

        SubMenu() {
            nWidth = 0;
            chMenuKey = 0;
        }
    };
    typedef vector<SubMenu>        VecSubMenus;

    CSkinFontProperty           m_font;
    CColor                      m_clrHover, m_clrOutlinedHover, m_clrPressed, m_clrOutlinedPressed;
    CColor                      m_clrBgPressed;
    bool                        m_bOutlinedHover, m_bOutlinedPressed;

    string                      m_strMenu;

    bool                        m_isShowingMenu;
    CPoint                      m_ptLast;

    VecSubMenus                 m_vSubMenus;
    int                         m_nSelSubMenu;

    SkinMenuItemsContainer      *m_popupMenu;
    SkinMenuPtr                 m_pMenu;

    Cursor                      m_cursorArrow;

};

#endif // !defined(_SKIN_MENU_BAR_H_)
