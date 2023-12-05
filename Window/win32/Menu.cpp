#include "../WindowLib.h"


bool toLocalMenu(HMENU hMenu) {
    if (g_LangTool.isEnglish() && !g_LangTool.hasMacro()) {
        return true;
    }

    // int                nCount;
    MENUITEMINFO info;
    char szString[256];
    char szAccKey[256];
    cstr_t szPos;

    // don't use get menu item count, under win ce, it will cost a lot time
    for (int i = 0; i < 256; i++) {
        memset(&info, 0, sizeof(info));

        info.cbSize = sizeof(info);
        info.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA;
        info.dwTypeData = szString;
        info.cch = CountOf(szString);
        szString[0] = '\0';

        // get menu item by pos.
        if (!GetMenuItemInfo(hMenu, i, true, &info)) {
            break;
        }

        if (info.hSubMenu != nullptr) {
            toLocalMenu(info.hSubMenu);
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

        info.cch = strlen(szString) + 1;
        info.fMask = MIIM_DATA | MIIM_TYPE;
        if (!SetMenuItemInfo(hMenu, i, true, &info)) {
            OSError Err;
            ERR_LOG1("SetMenuItemInfo: %s", Err.Description());
            continue;
        }
    }

    return true;
}

bool toLocalMenu(CMenu *pMenu) {
    return toLocalMenu(pMenu->getHandle());
}

//////////////////////////////////////////////////////////////////////

CMenu::CMenu() {
}

CMenu::~CMenu() {
    destroy();
}

bool CMenu::isValid() const {
    return m_hMenu != nullptr;
}

bool CMenu::loadMenu(int nID) {
    destroy();

    m_hMenu = ::LoadMenu(getAppInstance(), MAKEINTRESOURCE(nID));
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

    m_hMenu = ::CreatePopupMenu();

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

    updateMenuStatus(pWnd);

    TPMPARAMS tpmParam;

    if (prcNotOverlap) {
        tpmParam.cbSize = sizeof(tpmParam);
        tpmParam.rcExclude = *prcNotOverlap;
    }

    TrackPopupMenuEx(getHandle(), TPM_LEFTALIGN, x, y, pWnd->getWndHandle(), prcNotOverlap ? &tpmParam : nullptr);
}

void CMenu::trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap) {
    assert(pWnd);

    updateMenuStatus(pWnd);

    TPMPARAMS tpmParam;

    if (prcNotOverlap) {
        tpmParam.cbSize = sizeof(tpmParam);
        tpmParam.rcExclude = *prcNotOverlap;
    }

    TrackPopupMenuEx(getSubMenuHandle(nSubMenu), TPM_LEFTALIGN | TPM_VERTICAL, x, y, pWnd->getWndHandle(), prcNotOverlap ? &tpmParam : nullptr);
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
    return ::GetMenuItemCount(getHandle());
}

void CMenu::appendItem(uint32_t nID, cstr_t text, cstr_t szShortcutKey) {
    if (isEmptyString(szShortcutKey)) {
        ::AppendMenu(m_hMenu, MF_STRING, nID, text);
    } else {
        ::AppendMenu(m_hMenu, MF_STRING, nID, (string(text) + "\t" + szShortcutKey).c_str());
    }
}

void CMenu::appendSeperator() {
    AppendMenu(m_hMenu, MF_SEPARATOR, 0, nullptr);
}

void CMenu::insertItem(int nPos, uint32_t nID, cstr_t text, cstr_t szShortcutKey) {
    if (isEmptyString(szShortcutKey)) {
        ::InsertMenu(getHandle(), nPos, MF_BYPOSITION, nID, text);
    } else {
        ::InsertMenu(getHandle(), nPos, MF_BYPOSITION, nID, (string(text) + "\t" + szShortcutKey).c_str());
    }
}

HMENU findMenuPos(HMENU hMenu, UINT nIDMenu) {
    MENUITEMINFO	mii;

    // don't use get menu item count, under win ce, it will cost a lot time
    for (int i = 0; i < 256; i++) {
        memset(&mii, 0, sizeof(mii));

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
        mii.dwTypeData = NULL;
        mii.cch = 0;

        // get menu item by pos.
        if (!::GetMenuItemInfo(hMenu, i, TRUE, &mii)) {
            break;
        }

        if (mii.hSubMenu != NULL) {
            HMENU h = findMenuPos(mii.hSubMenu, nIDMenu);
            if (h != NULL) {
                return h;
            }
        } else {
            if (mii.wID == nIDMenu) {
                return hMenu;
            }
        }
    }

    return NULL;
}

void CMenu::insertItemByID(uint32_t nPosID, uint32_t nID, cstr_t text, cstr_t szShortcutKey) {
    HMENU hMenu = findMenuPos(m_hMenu, nPosID);
    assert(hMenu);
    if (hMenu == nullptr) {
        return;
    }

    if (isEmptyString(szShortcutKey)) {
        ::InsertMenu(hMenu, nPosID, MF_BYCOMMAND, nID, text);
    } else {
        ::InsertMenu(hMenu, nPosID, MF_BYCOMMAND, nID, (string(text) + "\t" + szShortcutKey).c_str());
    }
}

void CMenu::insertSeperator(int nPos) {
    ::InsertMenu(getHandle(), nPos, MF_SEPARATOR, 0, nullptr);
}

CMenu CMenu::appendSubmenu(cstr_t text) {
    HMENU hSubMenu = ::CreatePopupMenu();
    ::AppendMenu(getHandle(), MF_POPUP, (UINT_PTR)hSubMenu, text);

    CMenu menu;
    menu.attach(hSubMenu, false);

    return menu;
}

CMenu CMenu::insertSubmenu(int nPos, cstr_t text) {
    HMENU hSubMenu = ::CreatePopupMenu();
    ::InsertMenu(getHandle(), nPos, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hSubMenu, text);

    CMenu menu;
    menu.attach(hSubMenu, false);

    return menu;
}


void CMenu::removeItem(int nPos) {
    ::DeleteMenu(getHandle(), nPos, MF_BYPOSITION);
}

void CMenu::removeAllItems() {
    int count = getItemCount();
    for (count--; count >= 0; count--) {
        removeItem(count);
    }
}

bool CMenu::getMenuItemText(uint32_t index, string &text, bool byPosition) {
    MenuItemInfo info;

    if (!getMenuItemInfo(index, byPosition, info)) {
        return false;
    }
    text = info.text;

    return true;
}

bool CMenu::getMenuItemInfo(uint32_t index, bool byPosition, MenuItemInfo &itemOut) {
    MENUITEMINFO info;
    char szString[256];

    info.cbSize = sizeof(info);
    info.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA | MIIM_ID;
    info.dwTypeData = szString;
    info.cch = CountOf(szString);
    szString[0] = '\0';

    // get menu item by pos.
    if (!::GetMenuItemInfo(getHandle(), index, byPosition, &info)) {
        return false;
    }

    itemOut.text = szString;
    itemOut.id = info.wID;
    itemOut.isSubmenu = info.hSubMenu != nullptr;

    auto state = GetMenuState(m_hMenu, index, byPosition);

    itemOut.isChecked = state & MF_CHECKED;
    itemOut.isEnabled = !(state & MF_DISABLED);

    return true;
}

bool CMenu::isSeparator(int pos) {
    auto state = GetMenuState(m_hMenu, pos, MF_BYPOSITION);
    return state & MF_SEPARATOR;
}

bool CMenu::loadPopupMenu(int nID, int nSubMenu) {
    destroy();

    m_hMenu = ::LoadMenu(getAppInstance(), MAKEINTRESOURCE(nID));
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
    if (!::GetMenuItemInfo(getHandle(), nPos, true, &mii)) {
        return false;
    }

    return true;
}

bool CMenu::hasSubmenu(int nPos) {
    return getSubMenuHandle(nPos) != nullptr;
}

CMenu CMenu::getSubmenu(int nPos) {
    CMenu menu;

    menu.attach(getSubMenuHandle(nPos), false);

    return menu;
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

CMenu & CMenu::operator = (const CMenu &menu) {
    m_hMenu = menu.m_hMenu;
    m_nSubMenu = menu.m_nSubMenu;
    m_bFree = false;

    return *this;
}
