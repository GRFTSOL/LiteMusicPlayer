#pragma once

#ifndef Window_mac_Menu_h
#define Window_mac_Menu_h

class Window;

struct MenuItemInfo {
    bool                        isEnabled = true;
    bool                        isSeparator = false;
    bool                        isSubmenu = false;
    bool                        isChecked = false;
    std::string                 text, shortcutKey;
    uint32_t                    id = 0;
};

class CMenu {
public:
    CMenu();
    CMenu(const CMenu &src);
    virtual ~CMenu();

    bool isValid() const;

    bool createPopupMenu();

    virtual void onLoadMenu() { }

    void destroy();

    // 在弹出菜单前会调用此函数更新菜单状态
    virtual void updateMenuStatus(Window *window) { }

    virtual void trackPopupMenu(int x, int y, Window *pWnd, CRect *prcNotOverlap = nullptr);
    void trackPopupMenu(CPoint pos, Window *pWnd, CRect *prcNotOverlap = nullptr)
        { trackPopupMenu(pos.x, pos.y, pWnd, prcNotOverlap); }
    virtual void trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap = nullptr);

    void enableItem(int nID, bool bEnable);
    void checkItem(int nID, bool bCheck);
    void checkRadioItem(int nID, int nStartID, int nEndID);

    int getItemCount();

    virtual void appendItem(uint32_t nID, cstr_t szText, cstr_t szShortcutKey = "");
    virtual void insertItem(int nPos, uint32_t nID, cstr_t szText, cstr_t szShortcutKey = "");
    virtual void insertItemByID(uint32_t nPosID, uint32_t nID, cstr_t szText, cstr_t szShortcutKey = "");

    void appendSeperator();
    void insertSeperator(int nPos);

    CMenu appendSubmenu(cstr_t szText);
    CMenu insertSubmenu(int nPos, cstr_t szText);

    void removeItem(int nPos);
    void removeAllItems();

    void replaceAllItems(int idStartWith, const VecStrings &names);

    bool getMenuItemText(uint32_t item, string &strText, bool byPosition);

    bool getMenuItemInfo(uint32_t item, bool byPosition, MenuItemInfo &itemOut);
    bool isSeparator(int pos);
    string getShortcutKeyText(int pos);

    bool hasItem(int nPos);
    bool hasSubmenu(int nPos);
    CMenu getSubmenu(int nPos);

    // Find submenu if its first submenu id is same.
    CMenu getSubmenuByPlaceHolderID(uint32_t id);

    CMenu & operator = (const CMenu &menu);

public:
#ifdef _WIN32
    bool loadMenu(int nID);
    bool loadPopupMenu(int nID, int nSubMenu);
    HMENU getSubMenuHandle(int nPos);

    void attach(HMENU hMenu, bool isFree) { m_hMenu = hMenu; m_bFree = isFree; }
    HMENU getHandle();
protected:
    HMENU                       m_hMenu = NULL;
    int                         m_nSubMenu = -1;
    bool                        m_bFree = true;

#else // #ifdef _WIN32
    // For Mac
    void *getHandle(Window *window);
    void attachHandle(void *handle);
protected:
    struct MLMenuInfo    *m_info;
#endif // #ifdef _WIN32

};

inline void CMenu::replaceAllItems(int idStartWith, const VecStrings &names) {
    int count = getItemCount();
    int i = 0;
    for (i = 0; i < count; i++) {
        MenuItemInfo info;
        if (getMenuItemInfo(i, true, info)) {
            if (info.id == idStartWith) {
                break;
            }
        }
    }

    for (; i < count; count--) {
        removeItem(i);
    }

    for (auto &name : names) {
        appendItem(idStartWith++, name.c_str());
    }
}

bool toLocalMenu(CMenu *pMenu);

#endif // !defined(Window_mac_Menu_h)
