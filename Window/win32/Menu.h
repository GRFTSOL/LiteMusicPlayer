// Menu.h: interface for the CMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_)
#define AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_

class Window;

class CMenu  
{
public:
    CMenu();
    virtual ~CMenu();

    bool isValid() const { return m_hMenu != nullptr; }

    bool createPopupMenu();

    virtual void onLoadMenu() { }

    void destroy();

    virtual void trackPopupMenu(int x, int y, Window *pWnd, CRect *prcNotOverlap = nullptr);

    virtual void trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap = nullptr);

    int getItemCount();

    void enableItem(int nID, bool bEnable);
    void checkItem(int nID, bool bCheck);
    void checkRadioItem(int nID, int nStartID, int nEndID);

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

public:
    //
    // APIs for Win32
    //

    bool loadMenu(int nID);
    bool loadPopupMenu(int nID, int nSubMenu);

    void attach(HMENU hMenu) { m_hMenu = hMenu; }
    HMENU getSubMenuHandle(int nPos);
    HMENU getHandle();

    HMENU findMenuPos(HMENU hMenu, uint32_t nIDMenu);

protected:
    HMENU            m_hMenu;
    int                m_nSubMenu;
    bool            m_bFree;

};

bool toLocalMenu(CMenu *pMenu);
bool toLocalMenu(HMENU hMenu);

#endif // !defined(AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_)
