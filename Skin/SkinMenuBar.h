#pragma once

#if !defined(_SKIN_MENU_BAR_H_)
#define _SKIN_MENU_BAR_H_

#include "UIObject.h"
#include "SkinMenu.h"


class CSkinMenuBar;

#ifdef _WIN32

//
// Win32 only, responsible for Hooking mouse and keyboard message,
// then forward to CSkinMenuBar, CSkinMenuBar will decide when to popup
// the next or prev sub menu.
//
class CMenuBarMsgHandler : public Window {
public:
    enum {
        WM_MB_TRACKMENU             = WM_USER + 1,
    };

    CMenuBarMsgHandler() {
        m_pMenuBar = nullptr;
        m_bMenuPopup = false;
    }

    void create();

    void postTrackPopupMenu();

    bool trackPopupSubMenu(int x, int y, CMenu *pMenu, int nSubMenu, Window *pWnd, CRect *prcNotOverlap);

    bool isMenuPopuped() { return m_bMenuPopup; }

    void onKeyDown(uint32_t nChar, uint32_t nFlags);

    void onMouseMove(CPoint point);

    virtual LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

protected:
    //
    // Hook messages while menu is popup.
    //
    static LRESULT CALLBACK MessageHookProc(int code, WPARAM wParam, LPARAM lParam);
    static void processHookMessage(uint32_t message, WPARAM wParam, LPARAM lParam);

    static HHOOK                ms_hMsgHook;
    static bool                 ms_bPrimaryMenu;
    static bool                 ms_bSubMenuItem;
    static HWND                 ms_hWndMsgReceiver;
    static HMENU                ms_hMenuPopup;

public:
    CSkinMenuBar                *m_pMenuBar;

protected:
    bool                        m_bMenuPopup;

};

#else // _WIN32

class CMenuBarMsgHandler {
public:
    CMenuBarMsgHandler() {
        m_pMenuBar = nullptr;
        m_bMenuPopup = false;
    }

    void create() { }

    void destroy() { }

    void postTrackPopupMenu() { }

    bool trackPopupSubMenu(int x, int y, CMenu *pMenu, int nSubMenu, Window *pWnd, CRect *prcNotOverlap);

    bool isMenuPopuped() { return m_bMenuPopup; }

public:
    CSkinMenuBar                *m_pMenuBar;
    bool                        m_bMenuPopup;

};

#endif // _WIN32

class CSkinMenuBar : public CUIObject {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinMenuBar();
    virtual ~CSkinMenuBar();

    void onCreate() override;

    void draw(CRawGraph *canvas) override;

    bool onLButtonDown(uint32_t nFlags, CPoint point) override;
    bool onLButtonUp(uint32_t nFlags, CPoint point) override;

    bool onMouseMove(CPoint point) override;

    void onKeyDown(uint32_t nChar, uint32_t nFlags) override;

    bool onMenuKey(uint32_t nChar, uint32_t nFlags) override;

    void onSetFocus() override;
    void onKillFocus() override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    virtual void onLanguageChanged() override;

protected:
    int hitTestSubMenu(const CPoint point);
    int getItemX(int n);

    void trackPopupMenu();

    friend class CMenuBarMsgHandler;

protected:
    struct SubMenu {
        string                      strText;
        int                         nWidth;
        bool                        bHasSubMenu;
        int                         chMenuKey;          // Menu Key: Alt + VK_X to popup associated menu.
        uint32_t                    nID;                // If bHasSubMenu is false, this memember will be available.
    };
    typedef vector<SubMenu>        VecSubMenus;

    CSkinFontProperty           m_font;
    CColor                      m_clrHover, m_clrOutlinedHover, m_clrPressed, m_clrOutlinedPressed;
    CColor                      m_clrBgPressed;
    bool                        m_bOutlinedHover, m_bOutlinedPressed;

    string                      m_strMenu;

    bool                        m_bHover;
    bool                        m_bLBtDown;
    CPoint                      m_ptLast;

    VecSubMenus                 m_vSubMenus;
    int                         m_nSelSubMenu;

    CMenu                       *m_pMenu;

    CMenuBarMsgHandler          m_wndMsgReceiver;

};

#endif // !defined(_SKIN_MENU_BAR_H_)
