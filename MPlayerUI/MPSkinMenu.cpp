#include "MPlayerApp.h"
#include "MPSkinMenu.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"


struct CommandIDEncoding {
    int                         nCmdId;
    CharEncodingType            nEncodingId;
};

CommandIDEncoding    _arrCmdIdCharset[] = {
    {ID_ENC_DEFAULT, ED_SYSDEF },
    {ID_ENC_UNICODE, ED_UNICODE },
    {ID_ENC_UTF8, ED_UTF8 },
    {ID_ENC_BALTIC, ED_BALTIC_WINDOWS },
    {ID_ENC_WEST_EUROPE, ED_WESTERN_EUROPEAN_WINDOWS},
    {ID_ENC_CENTRAL_EUROPE, ED_CENTRAL_EUROPEAN_WINDOWS},
    {ID_ENC_GREEK, ED_GREEK_WINDOWS },
    {ID_ENC_CYRILLIC, ED_CYRILLIC_WINDOWS },
    {ID_ENC_TURKISH, ED_TURKISH_WINDOWS },
    {ID_ENC_VIETNAMESE, ED_VIETNAMESE },
    {ID_ENC_KOREAN, ED_KOREAN },
    {ID_ENC_ARABIC, ED_ARABIC },
    {ID_ENC_HEBREW, ED_HEBREW_WINDOWS },
    {ID_ENC_THAI, ED_THAI},
    {ID_ENC_JAPANESE, ED_JAPANESE_SHIFT_JIS},
    {ID_ENC_CHT, ED_BIG5 },
    {ID_ENC_CHS, ED_GB2312 },
    {ID_ENC_LATIN9_ISO, ED_LATIN9_ISO },
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
    { ID_SHUFFLE, SZ_SECT_PLAYER, "shuffle", "1", },
    { ID_ANTIAlIAS, SZ_SECT_UI, "Antialias", "1", },
    { ID_CLICK_THROUGH, nullptr, nullptr, "1",  },
    { ID_TOPMOST, nullptr, nullptr, "1",  },
    { ID_LDO_KARAOKE, nullptr, nullptr, "1",  },
    { ID_FLOATING_LYRICS, nullptr, nullptr, "1", },
};

MenuRadioGroupIDName    _MenuRadioLoop[] = {
    { ID_LOOP_ALL, "all" },
    { ID_LOOP_TRACK, "track" },
    { ID_LOOP_OFF, "off" },
    { 0, nullptr },
};

MenuRadioGroupIDName    _MenuRadioLyrDisplayStyle[] = {
    { ID_LDS_MULTI_LINE, nullptr },
    { ID_LDS_STATIC_TXT, nullptr },
    { ID_LDS_TWO_LINE, nullptr },
    { ID_LDS_SINGLE_LINE, nullptr },
    { ID_LDS_VOBSUB, nullptr },
    { 0, nullptr },
};

MenuRadioGroupIDName    _MenuRadioLyrDrawOpt[] = {
    { ID_LDO_AUTO, nullptr },
    { ID_LDO_NORMAL, nullptr },
    { ID_LDO_FADE_IN, nullptr },
    { ID_LDO_FADEOUT_BG, nullptr },
    { 0, nullptr },
};

MenuRadioGroupIDName    _MenuRadioCharEncoding[] = {
    { ID_ENC_DEFAULT, "" },
    { ID_ENC_UNICODE, "" },
    { ID_ENC_UTF8, "" },
    { ID_ENC_BALTIC, "" },
    { ID_ENC_WEST_EUROPE, "" },
    { ID_ENC_CENTRAL_EUROPE, "" },
    { ID_ENC_GREEK, "" },
    { ID_ENC_CYRILLIC, "" },
    { ID_ENC_TURKISH, "" },
    { ID_ENC_VIETNAMESE, "" },
    { ID_ENC_KOREAN, "" },
    { ID_ENC_ARABIC, "" },
    { ID_ENC_HEBREW, "" },
    { ID_ENC_THAI, "" },
    { ID_ENC_JAPANESE, "" },
    { ID_ENC_CHT, "" },
    { ID_ENC_CHS, "" },
    { ID_ENC_LATIN9_ISO, "" },
    { 0, nullptr },
};

MenuRadioGroupIDName    _MenuRadioOpaque[] = {
    { ID_SET_OPAQUE_100, nullptr },
    { ID_SET_OPAQUE_90, nullptr },
    { ID_SET_OPAQUE_80, nullptr },
    { ID_SET_OPAQUE_70, nullptr },
    { ID_SET_OPAQUE_60, nullptr },
    { ID_SET_OPAQUE_50, nullptr },
    { ID_SET_OPAQUE_40, nullptr },
    { ID_SET_OPAQUE_30, nullptr },
    { ID_SET_OPAQUE_20, nullptr },
    { ID_SET_OPAQUE_10, nullptr },
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
            if (nID == ID_SKIN_START) {
                m_hInsertSkinsMenu = menu;
                continue;
            } else if (nID == ID_NEW_MENU_POS) {
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

            if (item.nRadioStartID == ID_ENC_DEFAULT) {
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
        insertSkinMenu(m_autoCheckMenu.m_hInsertSkinsMenu);
    }
}

bool CMPSkinMenu::getShortcutKey(int id, string &strShortcut) {
    return MPlayerApp::getHotkey().getHotkeyText(id, strShortcut);
}

void CMPSkinMenu::insertSkinMenu(CMenu &menuSkin) {
    // 查找所有的Skin，并且添加到菜单中
    vector<string> vSkins;
    if (MPlayerApp::getMPSkinFactory()->enumAllSkins(vSkins)) {
        menuSkin.replaceAllItems(ID_SKIN_START, vSkins);

        // 取得上次加载的Skin
        string strDefaultSkin = CSkinApp::getInstance()->getDefaultSkin();
        auto it = std::find(vSkins.begin(), vSkins.end(), strDefaultSkin);
        if (it != vSkins.end()) {
            menuSkin.checkItem(ID_SKIN_START + (int)(it - vSkins.begin()), true);
        }
    }
}

bool onCommandSkin(int id) {
    if (id < ID_SKIN_START || id >= ID_SKIN_END) {
        return false;
    }

    vector<string> vSkins;
    if (MPlayerApp::getMPSkinFactory()->enumAllSkins(vSkins)) {
        int index = id - ID_SKIN_START;
        if (index >= 0 && index < (int)vSkins.size()) {
            auto &strSkin = vSkins[index];
            string strDefaultSkin = CSkinApp::getInstance()->getDefaultSkin();
            if (strcmp(strSkin.c_str(), strDefaultSkin.c_str()) != 0) {
                MPlayerApp::getInstance()->changeSkinByUserCmd(strSkin.c_str());
            }
        }
    }

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
            menu.enableItem(ID_EDIT_CUT, bEnable);
            menu.enableItem(ID_EDIT_COPY, bEnable);

            bEnable = pEditor->canUndo();
            menu.enableItem(ID_EDIT_UNDO, bEnable);

            bEnable = pEditor->canRedo();
            menu.enableItem(ID_EDIT_REDO, bEnable);
        }
    }

    CMPSkinMenu::trackPopupSubMenu(x, y, nSubMenu, pWnd, prcNotOverlap);
}
