#include "BaseWnd.h"
#include "MLMenu.h"


bool toLocalMenu(CMenu *pMenu) {
    return toLocalMenu(pMenu->getHandle());
}

bool toLocalMenu(HMENU hMenu) {
    if (g_LangTool.isEnglish() && !g_LangTool.hasMacro()) {
        return true;
    }

    // int                nCount;
    MENUITEMINFO MenuItemInfo;
    char szString[256];
    char szAccKey[256];
    cstr_t szPos;

    // don't use get menu item count, under win ce, it will cost a lot time
    for (int i = 0; i < 256; i++) {
        memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));

        MenuItemInfo.cbSize = sizeof(MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA;
        MenuItemInfo.dwTypeData = szString;
        MenuItemInfo.cch = CountOf(szString);
        szString[0] = '\0';

        // get menu item by pos.
        if (!getMenuItemInfo(hMenu, i, true, &MenuItemInfo)) {
            break;
        }

        if (MenuItemInfo.hSubMenu != nullptr) {
            toLocalMenu(MenuItemInfo.hSubMenu);
        }

        if (isEmptyString(szString)) {
            continue;
        }

        //
        // translate menu string
        //
        szPos = strchr(szString, '\t');
        if (!szPos) {
            strcpy_safe(szString, CountOf(szString), _TL(szString));
        } else {
            strcpy_safe(szAccKey, CountOf(szAccKey), szPos);
            szString[int(szPos - szString)] = '\0';
            strcpy_safe(szString, CountOf(szString), _TL(szString));
            strcat_safe(szString, CountOf(szString), szAccKey);
        }

        MenuItemInfo.cch = strlen(szString) + 1;
        MenuItemInfo.fMask = MIIM_DATA | MIIM_TYPE;
        if (!SetMenuItemInfo(hMenu, i, true, &MenuItemInfo)) {
            OSError Err;
            ERR_LOG1("SetMenuItemInfo: %s", Err.Description());
            continue;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////

CMenu::CMenu() {
    m_hMenu = nullptr;
    m_nSubMenu = -1;
    m_bFree = true;
}

CMenu::~CMenu() {
    destroy();
}

bool CMenu::loadMenu(int nID) {
    destroy();

    m_hMenu = ::loadMenu(getAppInstance(), MAKEINTRESOURCE(nID));
    assert(m_hMenu);
    m_nSubMenu = -1;

    toLocalMenu(this);

    if (m_hMenu != nullptr) {
        onLoadMenu();
    }

    return m_hMenu != nullptr;
}

bool CMenu::createPopupMenu() {
    destroy();

    m_hMenu = ::createPopupMenu();

    return m_hMenu != nullptr;
}

void CMenu::destroy() {
    if (m_hMenu && m_bFree) {
        DestroyMenu(m_hMenu);
        m_hMenu = nullptr;
    }
}

void CMenu::trackPopupMenu(int x, int y, Window *pWnd, CRect *prcNotOverlap) {
    assert(pWnd);

    TPMPARAMS tpmParam;

    if (prcNotOverlap) {
        tpmParam.cbSize = sizeof(tpmParam);
        tpmParam.rcExclude = *prcNotOverlap;
    }

    TrackPopupMenuEx(getHandle(), TPM_LEFTALIGN, x, y, pWnd->getHandle(), prcNotOverlap ? &tpmParam : nullptr);
}

void CMenu::trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap) {
    assert(pWnd);

    TPMPARAMS tpmParam;

    if (prcNotOverlap) {
        tpmParam.cbSize = sizeof(tpmParam);
        tpmParam.rcExclude = *prcNotOverlap;
    }

    TrackPopupMenuEx(getSubMenuHandle(nSubMenu), TPM_LEFTALIGN | TPM_VERTICAL, x, y, pWnd->getHandle(), prcNotOverlap ? &tpmParam : nullptr);
}

void CMenu::enableItem(int nID, bool bEnable) {
    HMENU hMenu = getHandle();

    EnableMenuItem(hMenu, nID, bEnable ? MF_ENABLED : MF_GRAYED);
}

void CMenu::checkItem(int nID, bool bCheck) {
    HMENU hMenu = getHandle();

    CheckMenuItem(hMenu, nID, bCheck ? MF_CHECKED : MF_UNCHECKED);
}

void CMenu::checkRadioItem(int nID, int nStartID, int nEndID) {
    HMENU hMenu = getHandle();

    CheckMenuRadioItem(hMenu, nStartID, nEndID, nID, MF_BYCOMMAND);
}

int CMenu::getItemCount() {
    return ::getMenuItemCount(getHandle());
}

void CMenu::appendItem(uint32_t nID, cstr_t szText, cstr_t szShortcutKey) {
    if (isEmptyString(szShortcutKey)) {
        ::AppendMenu(m_hMenu, MF_STRING, nID, szText);
    } else {
        ::AppendMenu(m_hMenu, MF_STRING, nID, (string(szText) + "\t" + szShortcutKey).c_str());
    }
}

void CMenu::appendSeperator() {
    AppendMenu(m_hMenu, MF_SEPARATOR, 0, nullptr);
}

void CMenu::insertItem(int nPos, uint32_t nID, cstr_t szText, cstr_t szShortcutKey) {
    if (isEmptyString(szShortcutKey)) {
        ::InsertMenu(getHandle(), nPos, MF_BYPOSITION, nID, szText);
    } else {
        ::InsertMenu(getHandle(), nPos, MF_BYPOSITION, nID, (string(szText) + "\t" + szShortcutKey).c_str());
    }
}

void CMenu::insertItemByID(uint32_t nPosID, uint32_t nID, cstr_t szText, cstr_t szShortcutKey) {
    HMENU hMenu = findMenuPos(getHandle(), nPosID);
    assert(hMenu);
    if (hMenu == nullptr) {
        return;
    }

    if (isEmptyString(szShortcutKey)) {
        ::InsertMenu(hMenu, nPosID, MF_BYCOMMAND, nID, szText);
    } else {
        ::InsertMenu(hMenu, nPosID, MF_BYCOMMAND, nID, (string(szText) + "\t" + szShortcutKey).c_str());
    }
}

void CMenu::insertSeperator(int nPos) {
    ::InsertMenu(getHandle(), nPos, MF_SEPARATOR, 0, nullptr);
}

CMenu CMenu::appendSubmenu(cstr_t szText) {
    HMENU hSubMenu = ::createPopupMenu();
    ::AppendMenu(getHandle(), MF_POPUP, (UINT_PTR)hSubMenu, szText);

    CMenu menu;
    menu.attach(hSubMenu);
    menu.m_bFree = false;

    return menu;
}

CMenu CMenu::insertSubmenu(int nPos, cstr_t szText) {
    HMENU hSubMenu = ::createPopupMenu();
    ::InsertMenu(getHandle(), nPos, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hSubMenu, szText);

    CMenu menu;
    menu.attach(hSubMenu);
    menu.m_bFree = false;

    return menu;
}


void CMenu::removeItem(int nPos) {
    ::DeleteMenu(getHandle(), nPos, MF_BYPOSITION);
}

bool CMenu::getMenuItemText(uint32_t nItem, string &strText, bool bByPosition) {
    uint32_t nID;
    bool bSubMenu;

    return getMenuItemInfo(nItem, strText, nID, bSubMenu, bByPosition);
}

bool CMenu::getMenuItemInfo(uint32_t nItem, string &strText, uint32_t &nID, bool &bSubMenu, bool bByPosition) {
    MENUITEMINFO MenuItemInfo;
    char szString[256];

    MenuItemInfo.cbSize = sizeof(MenuItemInfo);
    MenuItemInfo.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA | MIIM_ID;
    MenuItemInfo.dwTypeData = szString;
    MenuItemInfo.cch = CountOf(szString);
    szString[0] = '\0';

    // get menu item by pos.
    if (!::getMenuItemInfo(getHandle(), nItem, bByPosition, &MenuItemInfo)) {
        return false;
    }

    strText = szString;
    nID = MenuItemInfo.wID;
    bSubMenu = MenuItemInfo.hSubMenu != nullptr;

    return true;
}

bool CMenu::loadPopupMenu(int nID, int nSubMenu) {
    destroy();

    m_hMenu = ::loadMenu(getAppInstance(), MAKEINTRESOURCE(nID));
    assert(m_hMenu);
    m_nSubMenu = nSubMenu;

    toLocalMenu(::GetSubMenu(m_hMenu, m_nSubMenu));

    if (m_hMenu != nullptr) {
        onLoadMenu();
    }

    return m_hMenu != nullptr;
}

bool CMenu::hasItem(int nPos) {
    MENUITEMINFO mii;

    memset(&mii, 0, sizeof(mii));

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE;
    mii.dwTypeData = nullptr;
    mii.cch = 0;

    // get menu item by pos.
    if (!::getMenuItemInfo(getHandle(), nPos, true, &mii)) {
        return false;
    }

    return true;
}

bool CMenu::hasSubmenu(int nPos) {
    return getSubMenuHandle(nPos) != nullptr;
}

CMenu CMenu::getSubmenu(int nPos) {
    CMenu menu;

    menu.attach(getSubMenuHandle(nPos));
    menu.m_bFree = false;

    return menu;
}

CMenu & CMenu::operator = (CMenu &menu) {
    m_hMenu = menu.m_hMenu;
    m_nSubMenu = menu.m_nSubMenu;
    m_bFree = false;

    return *this;
}

HMENU CMenu::getSubMenuHandle(int nPos) {
    if (m_nSubMenu == -1) {
        return ::GetSubMenu(m_hMenu, nPos);
    } else {
        HMENU hSubMenu;
        hSubMenu = ::GetSubMenu(m_hMenu, m_nSubMenu);
        return ::GetSubMenu(hSubMenu, nPos);
    }
}

HMENU CMenu::getHandle() {
    if (m_nSubMenu == -1) {
        return m_hMenu;
    } else {
        return ::GetSubMenu(m_hMenu, m_nSubMenu);
    }
}

HMENU CMenu::findMenuPos(HMENU hMenu, uint32_t nIDMenu) {
    MENUITEMINFO mii;

    // don't use get menu item count, under win ce, it will cost a lot time
    for (int i = 0; i < 256; i++) {
        memset(&mii, 0, sizeof(mii));

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
        mii.dwTypeData = nullptr;
        mii.cch = 0;

        // get menu item by pos.
        if (!::getMenuItemInfo(hMenu, i, true, &mii)) {
            break;
        }

        if (mii.hSubMenu != nullptr) {
            HMENU h = findMenuPos(mii.hSubMenu, nIDMenu);
            if (h != nullptr) {
                return h;
            }
        } else {
            if (mii.wID == nIDMenu) {
                return hMenu;
            }
        }
    }

    return nullptr;
}
