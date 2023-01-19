#pragma once

#ifndef MPlayerUI_MPSkinMenu_h
#define MPlayerUI_MPSkinMenu_h



#include "../Skin/SkinMenu.h"


int cmdIdToEncoding(int nCmdId);

int encodingIdToCmd(CharEncodingType nEncodingId);

bool onCommandSkin(int nCmdId);

struct MenuItemCheck {
    uint32_t                    nID;
    cstr_t                      szSection;
    cstr_t                      szName;
    cstr_t                      szCheckValue;
    CMenu                       menu;
};

struct MenuRadioGroupIDName {
    uint32_t                    nID;
    cstr_t                      szCheckValue;
};

struct MenuRadioGroup {
    uint32_t                    nRadioStartID, nRadioEndID;
    MenuRadioGroupIDName        *idNames;
    CMenu                       menu;
};

class CMenuAutoCheck {
public:
    CMenuAutoCheck() {
        m_nInsertSkinsPos = 0;
        m_nInsertNewItemPos = 0;
    }

    void initProcSubMenu(CMenu &menu);

    void updateMenuCheckStatus();
    void addUICheckStatusIf(IUICheckStatus *pIfUICheckStatus);

protected:
    bool getChecked(uint32_t nID);
    bool getRadioChecked(MenuRadioGroupIDName *idNames, uint32_t &nIDChecked);

protected:
    typedef list<MenuItemCheck>    LIST_MENU_ITEMS_CHECK;
    typedef list<MenuRadioGroup>    LIST_MENU_ITEMS_RADIO;
    typedef list<IUICheckStatus *>    LIST_UI_CHECK_STATUS;

    LIST_MENU_ITEMS_CHECK       m_vMenuItemsCheck;
    LIST_MENU_ITEMS_RADIO       m_vMenuItemsRadio;
    LIST_UI_CHECK_STATUS        m_listUICheckStatusIf;

public:
    CMenu                       m_hInsertSkinsMenu;
    int                         m_nInsertSkinsPos;

    CMenu                       m_hInsertNewItemMenu;
    int                         m_nInsertNewItemPos;

};

class CMPSkinMenu : public CSkinMenu {
public:
    CMPSkinMenu();
    virtual ~CMPSkinMenu();

    virtual void onLoadMenu() override;
    virtual void updateMenuStatus(Window *window) override;

    void addUICheckStatusIf(IUICheckStatus *pIfUICheckStatus) { m_autoCheckMenu.addUICheckStatusIf(pIfUICheckStatus); }

protected:
    virtual bool getShortcutKey(int nMenuID, string &strShortcut) override;

    void insertSkinMenu(CMenu &menu, int nPosStart);

protected:
    CMenuAutoCheck              m_autoCheckMenu;

};

class CLyrEditorMenu : public CMPSkinMenu {
public:
    virtual void trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap = nullptr);

};

#endif // !defined(MPlayerUI_MPSkinMenu_h)
