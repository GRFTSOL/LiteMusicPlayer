#include "MPlayerApp.h"
#include "MPSkinMenu.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"


struct CommandIDEncoding {
    int                         nCmdId;
    CharEncodingType            nEncodingId;
};

CommandIDEncoding    _arrCmdIdCharset[] = {
    {IDC_ENC_DEFAULT, ED_SYSDEF },
    {IDC_ENC_UNICODE, ED_UNICODE },
    {IDC_ENC_UTF8, ED_UTF8 },
    {IDC_ENC_BALTIC, ED_BALTIC_WINDOWS },
    {IDC_ENC_WEST_EUROPE, ED_WESTERN_EUROPEAN_WINDOWS},
    {IDC_ENC_CENTRAL_EUROPE, ED_CENTRAL_EUROPEAN_WINDOWS},
    {IDC_ENC_GREEK, ED_GREEK_WINDOWS },
    {IDC_ENC_CYRILLIC, ED_CYRILLIC_WINDOWS },
    {IDC_ENC_TURKISH, ED_TURKISH_WINDOWS },
    {IDC_ENC_VIETNAMESE, ED_VIETNAMESE },
    {IDC_ENC_KOREAN, ED_KOREAN },
    {IDC_ENC_ARABIC, ED_ARABIC },
    {IDC_ENC_HEBREW, ED_HEBREW_WINDOWS },
    {IDC_ENC_THAI, ED_THAI},
    {IDC_ENC_JAPANESE, ED_JAPANESE_SHIFT_JIS},
    {IDC_ENC_CHT, ED_BIG5 },
    {IDC_ENC_CHS, ED_GB2312 },
    {IDC_ENC_LATIN9_ISO, ED_LATIN9_ISO },
    {-1, ED_SYSDEF}
};

int cmdIdToEncoding(int nCmdId) {
    for (int i = 0; i < CountOf(_arrCmdIdCharset); i++) {
        if (_arrCmdIdCharset[i].nCmdId == nCmdId) {
            return _arrCmdIdCharset[i].nEncodingId;
        }
    }

    return -1;
}

int encodingIdToCmd(CharEncodingType nEncodingId) {
    for (int i = 0; i < CountOf(_arrCmdIdCharset); i++) {
        if (_arrCmdIdCharset[i].nEncodingId == nEncodingId) {
            return _arrCmdIdCharset[i].nCmdId;
        }
    }

    return -1;
}

//////////////////////////////////////////////////////////////////////////

MenuItemCheck _menuItemsCheck[] = {
    { IDC_SHUFFLE, SZ_SECT_PLAYER, "shuffle", "1", },
    { IDC_ANTIAlIAS, SZ_SECT_UI, "Antialias", "1", },
    { IDC_CLICK_THROUGH, nullptr, nullptr, "1",  },
    { IDC_SETTOPMOST, nullptr, nullptr, "1",  },
    { IDC_LDO_KARAOKE, nullptr, nullptr, "1",  },
    { IDC_FLOATING_LYRICS, nullptr, nullptr, "1", },
};

MenuRadioGroupIDName    _MenuRadioLoop[] = {
    { IDC_REPEAT_ALL, "all" },
    { IDC_REPEAT_TRACK, "track" },
    { IDC_REPEAT_OFF, "off" },
    { 0, nullptr },
};

MenuRadioGroupIDName    _MenuRadioLyrDisplayStyle[] = {
    { IDC_LDS_MULTI_LINE, nullptr },
    { IDC_LDS_STATIC_TXT, nullptr },
    { IDC_LDS_TWO_LINE, nullptr },
    { IDC_LDS_SINGLE_LINE, nullptr },
    { IDC_LDS_VOBSUB, nullptr },
    { 0, nullptr },
};

MenuRadioGroupIDName    _MenuRadioLyrDrawOpt[] = {
    { IDC_LDO_AUTO, nullptr },
    { IDC_LDO_NORMAL, nullptr },
    { IDC_LDO_FADE_IN, nullptr },
    { IDC_LDO_FADEOUT_BG, nullptr },
    { 0, nullptr },
};

MenuRadioGroupIDName    _MenuRadioCharEncoding[] = {
    { IDC_ENC_DEFAULT, "" },
    { IDC_ENC_ARABIC, "" },
    { IDC_ENC_BALTIC, "" },
    { IDC_ENC_CYRILLIC, "" },
    { IDC_ENC_CENTRAL_EUROPE, "" },
    { IDC_ENC_WEST_EUROPE, "" },
    { IDC_ENC_GREEK, "" },
    { IDC_ENC_HEBREW, "" },
    { IDC_ENC_JAPANESE, "" },
    { IDC_ENC_KOREAN, "" },
    { IDC_ENC_LATIN9_ISO, "" },
    { IDC_ENC_CHS, "" },
    { IDC_ENC_CHT, "" },
    { IDC_ENC_THAI, "" },
    { IDC_ENC_TURKISH, "" },
    { IDC_ENC_UNICODE, "" },
    { IDC_ENC_UTF8, "" },
    { IDC_ENC_VIETNAMESE, "" },
    { 0, nullptr },
};

MenuRadioGroupIDName    _MenuRadioOpaque[] = {
    { IDC_SET_OPAQUE_100, nullptr },
    { IDC_SET_OPAQUE_90, nullptr },
    { IDC_SET_OPAQUE_80, nullptr },
    { IDC_SET_OPAQUE_70, nullptr },
    { IDC_SET_OPAQUE_60, nullptr },
    { IDC_SET_OPAQUE_50, nullptr },
    { IDC_SET_OPAQUE_40, nullptr },
    { IDC_SET_OPAQUE_30, nullptr },
    { IDC_SET_OPAQUE_20, nullptr },
    { IDC_SET_OPAQUE_10, nullptr },
    { 0, nullptr },
};

typedef uint32_t(*FUNGetCheckMenuID)();

static uint32_t getCharEncodingCheckMenuID() {
    CharEncodingType encoding = ED_SYSDEF;
    return encodingIdToCmd(encoding);
}

MenuRadioGroup _menuRadioGroup[] = {
    { 0, 0, _MenuRadioLoop, },
    { 0, 0, _MenuRadioLyrDrawOpt, },
    { 0, 0, _MenuRadioCharEncoding, },
    { 0, 0, _MenuRadioOpaque, },
    { 0, 0, _MenuRadioLyrDisplayStyle, },
};

void CMenuAutoCheck::clear() {
    m_vMenuItemsCheck.clear();
    m_vMenuItemsRadio.clear();
}

void CMenuAutoCheck::initProcSubMenu(CMenu &menu) {
    int nInRadioGroupIndex = -1;
    MenuRadioGroup radioGroup;

    int count = menu.getItemCount();
    for (int i = 0; i < count; i++) {
        MenuItemInfo info;

        if (!menu.getMenuItemInfo(i, true, info)) {
            break;
        }

        if (info.isSubmenu) {
            CMenu subMenu = menu.getSubmenu(i);
            initProcSubMenu(subMenu);
        } else if (!info.isSeparator) {
            int nID = info.id;
            if (nID == IDC_SKIN_0) {
                m_hInsertSkinsMenu = menu;
                m_nInsertSkinsPos = i;
                continue;
            } else if (nID == IDC_NEW_MENU_POS) {
                m_hInsertNewItemMenu = menu;
                m_nInsertNewItemPos = i;
                continue;
            }

            int k;
            for (k = 0; k < CountOf(_menuItemsCheck); k++) {
                if (nID == _menuItemsCheck[k].nID) {
                    MenuItemCheck item = _menuItemsCheck[k];

                    item.menu = menu;
                    m_vMenuItemsCheck.push_back(item);
                    break;
                }
            }
            if (k >= CountOf(_menuItemsCheck)) {
                for (int k = 0; k < CountOf(_menuRadioGroup); k++) {
                    for (int n = 0; _menuRadioGroup[k].idNames[n].nID != 0; n++) {
                        if (_menuRadioGroup[k].idNames[n].nID == nID) {
                            if (nInRadioGroupIndex != -1 && nInRadioGroupIndex != k) {
                                m_vMenuItemsRadio.push_back(radioGroup);
                                nInRadioGroupIndex = -1;
                            }

                            if (nInRadioGroupIndex == -1) {
                                // first group item.
                                radioGroup = _menuRadioGroup[k];
                                radioGroup.menu = menu;
                                radioGroup.nRadioStartID = nID;
                                radioGroup.nRadioEndID = nID;
                                nInRadioGroupIndex = k;
                            } else {
                                // next group item.
                                radioGroup.nRadioEndID = nID;
                            }
                            k = CountOf(_menuRadioGroup);
                            break;
                        }
                    }
                }
            }
        }
    }

    if (nInRadioGroupIndex != -1) {
        m_vMenuItemsRadio.push_back(radioGroup);
    }
}

void CMenuAutoCheck::updateMenuCheckStatus() {
    {
        //
        // update check item status.
        //
        LIST_MENU_ITEMS_CHECK::iterator it, itEnd;
        itEnd = m_vMenuItemsCheck.end();
        for (it = m_vMenuItemsCheck.begin(); it != itEnd; ++it) {
            MenuItemCheck &item = *it;
            if (!item.menu.isValid()) {
                continue;
            }
            bool bCheck;
            if (item.szSection == nullptr || item.szName == nullptr) {
                bCheck = getChecked(item.nID);
            } else {
                bCheck = strcasecmp(item.szCheckValue, g_profile.getString(item.szSection, item.szName, "")) == 0;
            }
            item.menu.checkItem(item.nID, bCheck);
        }
    }

    {
        //
        // update radio item status.
        //
        string strValue;
        LIST_MENU_ITEMS_RADIO::iterator it, itEnd;
        itEnd = m_vMenuItemsRadio.end();
        for (it = m_vMenuItemsRadio.begin(); it != itEnd; ++it) {
            MenuRadioGroup &item = *it;
            if (!item.menu.isValid()) {
                continue;
            }

            if (item.nRadioStartID == IDC_ENC_DEFAULT) {
                // user function
                uint32_t nID;
                nID = getCharEncodingCheckMenuID();
                item.menu.checkRadioItem(nID, item.nRadioStartID, item.nRadioEndID);
            } else {
                uint32_t nIDChecked;
                if (getRadioChecked(item.idNames, nIDChecked)) {
                    item.menu.checkRadioItem(nIDChecked, item.nRadioStartID, item.nRadioEndID);
                }
            }
        }
    }
}

void CMenuAutoCheck::addUICheckStatusIf(IUICheckStatus *pIfUICheckStatus) {
    m_listUICheckStatusIf.push_back(pIfUICheckStatus);
}

bool CMenuAutoCheck::getChecked(uint32_t nID) {
    LIST_UI_CHECK_STATUS::iterator it, itEnd;

    itEnd = m_listUICheckStatusIf.end();
    for (it = m_listUICheckStatusIf.begin(); it != itEnd; ++it) {
        IUICheckStatus *p = *it;
        bool bChecked;
        if (p->getChecked(nID, bChecked)) {
            return bChecked;
        }
    }
    return false;
}

bool CMenuAutoCheck::getRadioChecked(MenuRadioGroupIDName *idNames, uint32_t &nIDChecked) {
    LIST_UI_CHECK_STATUS::iterator it, itEnd;

    vector<uint32_t> vIDs;

    while (idNames->nID != 0) {
        vIDs.push_back(idNames->nID);
        idNames++;
    }

    itEnd = m_listUICheckStatusIf.end();
    for (it = m_listUICheckStatusIf.begin(); it != itEnd; ++it) {
        IUICheckStatus *p = *it;
        if (p->getRadioChecked(vIDs, nIDChecked)) {
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////

CMPSkinMenu::CMPSkinMenu() {
}

CMPSkinMenu::~CMPSkinMenu() {
}

void CMPSkinMenu::onLoadMenu() {
    m_autoCheckMenu.clear();

    m_autoCheckMenu.initProcSubMenu(*this);
    if (m_autoCheckMenu.m_nInsertNewItemPos != -1) {
        setOrgAppendPos(m_autoCheckMenu.m_nInsertNewItemPos);
        m_autoCheckMenu.m_hInsertNewItemMenu.removeItem(m_autoCheckMenu.m_nInsertNewItemPos);
    }
}

void CMPSkinMenu::updateMenuStatus(Window *window) {
    m_autoCheckMenu.updateMenuCheckStatus();

    if (m_autoCheckMenu.m_hInsertSkinsMenu.isValid()) {
        insertSkinMenu(m_autoCheckMenu.m_hInsertSkinsMenu,
            m_autoCheckMenu.m_nInsertSkinsPos);
    }
}

bool CMPSkinMenu::getShortcutKey(int nMenuID, string &strShortcut) {
    int nCmdID;
    nCmdID = CMPlayerApp::getMPSkinFactory()->getUIDByMenuID(nMenuID);
    if (nCmdID != UID_INVALID) {
        CMPlayerAppBase::getHotkey().getHotkeyText(nCmdID, strShortcut);
        return true;
    }

    return false;
}

static int        __vSkinMenuId[] = {IDC_SKIN_0, IDC_SKIN_1, IDC_SKIN_2, IDC_SKIN_3, IDC_SKIN_4, IDC_SKIN_5,
    IDC_SKIN_6, IDC_SKIN_7, IDC_SKIN_8, IDC_SKIN_9, IDC_SKIN_10, IDC_SKIN_11, IDC_SKIN_12,
    IDC_SKIN_13, IDC_SKIN_14, IDC_SKIN_15, IDC_SKIN_16, IDC_SKIN_17, IDC_SKIN_18, IDC_SKIN_19,
    IDC_SKIN_20 };

#define __SkinMenuCount CountOf(__vSkinMenuId)

void CMPSkinMenu::insertSkinMenu(CMenu &menuSkin, int nPosStart) {
    // 取得上次加载的Skin
    string strDefaultSkin = CSkinApp::getInstance()->getDefaultSkin();

    // 取得原来的SKIN子菜单，并且删除之
    menuSkin.removeAllItems();

    // 查找所有的Skin，并且添加到菜单中
    vector<string> vSkins;
    if (CMPlayerAppBase::getMPSkinFactory()->enumAllSkins(vSkins)) {
        for (int i = 0; i < (int)vSkins.size() && i < __SkinMenuCount; i++) {
            menuSkin.appendItem(__vSkinMenuId[i], vSkins[i].c_str());
            if (strcasecmp(strDefaultSkin.c_str(), vSkins[i].c_str()) == 0) {
                menuSkin.checkItem(__vSkinMenuId[i], true);
            }
        }
    }
}

bool onCommandSkin(int nCmdId) {
    int i;

    for (i = 0; i < __SkinMenuCount; i++) {
        if (nCmdId == __vSkinMenuId[i]) {
            break;
        }
    }
    if (i == __SkinMenuCount) {
        return false;
    }

    // 取得上次加载的Skin
    string strDefaultSkin = CSkinApp::getInstance()->getDefaultSkin();

    vector<string> vSkins;
    string strSkin;
    if (CMPlayerAppBase::getMPSkinFactory()->enumAllSkins(vSkins)) {
        if (i >= (int)vSkins.size()) {
            return false;
        }
    }

    strSkin = vSkins[i];
    if (strcmp(strSkin.c_str(), strDefaultSkin.c_str()) == 0) {
        return true;
    }

    CMPlayerAppBase::getInstance()->changeSkinByUserCmd(strSkin.c_str());

    return true;
}

void CLyrEditorMenu::trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap) {
    if (nSubMenu == 2) {
        bool bEnable;
        CSkinWnd *pSkinWnd = (CSkinWnd *)pWnd;
        CMenu menu = getSubmenu(nSubMenu);

        CLyricShowTextEditObj *pEditor = (CLyricShowTextEditObj*)pSkinWnd->getUIObjectByClassName(CLyricShowTextEditObj::className());
        if (pEditor) {
            bEnable = pEditor->isSelected();
            menu.enableItem(IDC_EDIT_CUT, bEnable);
            menu.enableItem(IDC_EDIT_COPY, bEnable);

            bEnable = pEditor->canUndo();
            menu.enableItem(IDC_EDIT_UNDO, bEnable);

            bEnable = pEditor->canRedo();
            menu.enableItem(IDC_EDIT_REDO, bEnable);
        }
    }

    CMPSkinMenu::trackPopupSubMenu(x, y, nSubMenu, pWnd, prcNotOverlap);
}
