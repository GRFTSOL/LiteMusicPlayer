#pragma once

#ifndef Window_mac_Menu_h
#define Window_mac_Menu_h

class Window;

class CMenu {
public:
    CMenu();
    CMenu(const CMenu &src);
    virtual ~CMenu();

    bool isValid() const;

    bool createPopupMenu();

    virtual void onLoadMenu() { }

    void destroy();

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

    bool getMenuItemText(uint32_t nItem, string &strText, bool bByPosition);

    bool getMenuItemInfo(uint32_t nItem, string &strText, uint32_t &nID, bool &bSubMenu, bool bByPosition);

    bool hasItem(int nPos);
    bool hasSubmenu(int nPos);
    CMenu getSubmenu(int nPos);

    CMenu & operator = (CMenu &menu);

    // For Mac
    void *getHandle();

protected:
    struct MLMenuInfo    *m_info;

};

bool toLocalMenu(CMenu *pMenu);

#endif // !defined(Window_mac_Menu_h)
