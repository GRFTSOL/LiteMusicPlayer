#pragma once

#ifndef Window_mac_Menu_h
#define Window_mac_Menu_h

class Window;

struct MenuItemInfo {
    bool                        isEnabled;
    bool                        isSeparator;
    bool                        isSubmenu;
    bool                        isChecked;
    string                      text, shortcutKey;
    uint32_t                    id;

    MenuItemInfo() {
        isEnabled = true;
        isSeparator = false;
        isSubmenu = false;
        isChecked = false;
        id = 0;
    }
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

    bool getMenuItemText(uint32_t item, string &strText, bool byPosition);

    bool getMenuItemInfo(uint32_t item, bool byPosition, MenuItemInfo &itemOut);
    bool isSeparator(int pos);
    string getShortcutKeyText(int pos);

    bool hasItem(int nPos);
    bool hasSubmenu(int nPos);
    CMenu getSubmenu(int nPos);

    CMenu & operator = (const CMenu &menu);

    // For Mac
    void *getHandle(Window *window);

protected:
    struct MLMenuInfo    *m_info;

};

bool toLocalMenu(CMenu *pMenu);

#endif // !defined(Window_mac_Menu_h)
