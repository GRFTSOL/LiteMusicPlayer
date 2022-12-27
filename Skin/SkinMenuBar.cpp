#include "SkinTypes.h"
#include "Skin.h"
#include "SkinMenuBar.h"


#ifdef _WIN32

HHOOK CMenuBarMsgHandler::ms_hMsgHook = nullptr;
bool CMenuBarMsgHandler::ms_bPrimaryMenu = true;
bool CMenuBarMsgHandler::ms_bSubMenuItem = false;
HWND CMenuBarMsgHandler::ms_hWndMsgReceiver = 0;
HMENU CMenuBarMsgHandler::ms_hMenuPopup = nullptr;

void CMenuBarMsgHandler::create() {
    Window::create(nullptr, 0, 0, 1, 1, nullptr, WS_POPUP);
}

void CMenuBarMsgHandler::postTrackPopupMenu() {
    // Exit popup menu first
    sendMessage(m_pMenuBar->m_pSkin->getHandle(), WM_CANCELMODE, 0 , 0);

    PostMessage(m_hWnd, WM_MB_TRACKMENU, 0, 0);
}

bool CMenuBarMsgHandler::trackPopupSubMenu(int x, int y, CMenu *pMenu, int nSubMenu, Window *pWnd, CRect *prcNotOverlap) {
    HMENU hMenu = pMenu->getSubMenuHandle(nSubMenu);
    assert(pWnd);
    assert(hMenu);
    if (!hMenu) {
        return false;
    }

    assert(ms_hMsgHook == nullptr);
    if (ms_hMsgHook == nullptr) {
        // install message hook
        ms_bPrimaryMenu = true;
        ms_bSubMenuItem = false;
        ms_hWndMsgReceiver = m_hWnd;
        ms_hMenuPopup = hMenu;
        ms_hMsgHook = ::SetWindowsHookEx(WH_MSGFILTER, MessageHookProc, 0, ::GetCurrentThreadId());
    }

    m_bMenuPopup = true;
    m_pMenuBar->invalidate();

    pMenu->trackPopupSubMenu(x, y, nSubMenu, pWnd, prcNotOverlap);
    // ::TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_VERTICAL, x, y, pWnd->getHandle(), prcNotOverlap ? &tpmParam : nullptr);

    m_bMenuPopup = false;
    m_pMenuBar->invalidate();

    if (ms_hMsgHook) {
        // uninstall message hook
        ::UnhookWindowsHookEx(ms_hMsgHook);
        ms_hMsgHook = 0;
        ms_hMenuPopup = nullptr;
        ms_hWndMsgReceiver = nullptr;
    }

    return true;
}

void CMenuBarMsgHandler::onKeyDown(uint32_t nChar, uint32_t nFlags) {
    m_pMenuBar->onKeyDown(nChar, nFlags);
}


void CMenuBarMsgHandler::onMouseMove(CPoint point) {
    if (WindowFromPoint(point) == m_pMenuBar->m_pSkin->getHandle()) {
        m_pMenuBar->m_pSkin->screenToClient(point);
        m_pMenuBar->onMouseMove(point);
    }
}


LRESULT CMenuBarMsgHandler::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_MB_TRACKMENU && m_pMenuBar) {
        m_pMenuBar->trackPopupMenu();

        return 0;
    } else {
        return Window::wndProc(message, wParam, lParam);
    }
}


LRESULT CALLBACK CMenuBarMsgHandler::MessageHookProc(int code, WPARAM wParam, LPARAM lParam ) {
    if (code == MSGF_MENU) {
        MSG* pMsg = ( MSG* )lParam;

        processHookMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
    }

    return ::CallNextHookEx(ms_hMsgHook, code, wParam, lParam );
}

void CMenuBarMsgHandler::processHookMessage(uint32_t message, WPARAM wParam, LPARAM lParam) {
    switch ( message ) {
    case WM_MENUSELECT:
        {
            HMENU hMenu = (HMENU)lParam;
            uint32_t wFlags = HIWORD(wParam);
            ms_bPrimaryMenu = (ms_hMenuPopup == hMenu);
            ms_bSubMenuItem = (wFlags & MF_POPUP ) != 0;
        }
        break;
    case WM_INITMENUPOPUP:
        {
            HMENU hMenu = (HMENU)wParam;
            int nIndex = LOWORD(lParam);
            ms_bPrimaryMenu = (ms_hMenuPopup == hMenu);
            ms_bSubMenuItem = (::GetSubMenu(hMenu, nIndex) != 0);
        }
        break;
    case WM_MOUSEMOVE:
        {
            ::sendMessage(ms_hWndMsgReceiver, message, wParam, lParam);
            break;
        }
    case WM_KEYDOWN:
        {
            uint32_t nChar = ( uint32_t )wParam;
            bool bForward = false; // by default

            switch (nChar) {
            case VK_LEFT:
                bForward = ms_bPrimaryMenu;
                break;
            case VK_RIGHT:
                bForward = !ms_bSubMenuItem;
                break;
                // Do not process escape
                // case VK_ESCAPE:
                //    m_bEscape = m_bPrimaryMenu;
                //    break;
            }

            // Should we forward this message to the menu bar?
            if (bForward) {
                ::sendMessage(ms_hWndMsgReceiver, message, wParam, lParam);
            }
            break;
        }
    default:
        break;
    }
}

#else

bool CMenuBarMsgHandler::trackPopupSubMenu(int x, int y, CMenu *pMenu, int nSubMenu, Window *pWnd, CRect *prcNotOverlap) {
    pMenu->trackPopupSubMenu(x, y, nSubMenu, pWnd, nullptr);
    return true;
}

#endif // _WIN32

//////////////////////////////////////////////////////////////////////////

#define SPACE_SUBMENUS        (m_font.getHeight()   / 2 + 5)

UIOBJECT_CLASS_NAME_IMP(CSkinMenuBar, "MenuBar")

CSkinMenuBar::CSkinMenuBar() {
    m_wndMsgReceiver.m_pMenuBar = this;

    m_msgNeed = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MENU_KEY;

    m_bLBtDown = false;
    m_bHover = false;

    m_nSelSubMenu = -1;

    m_bOutlinedHover = false;
    m_bOutlinedPressed = false;

    m_clrHover.set(0);
    m_clrPressed.set(0);
    m_clrBgPressed.set(0);
    m_clrBgPressed.setAlpha(128);

    m_pMenu = nullptr;
    m_ptLast.x = m_ptLast.y = 0;
}


CSkinMenuBar::~CSkinMenuBar() {
    m_wndMsgReceiver.destroy();

    if (m_pMenu) {
        delete m_pMenu;
    }
}


void CSkinMenuBar::onCreate() {
    CUIObject::onCreate();

    m_wndMsgReceiver.create();

    m_font.onCreate(m_pSkin);

    onLanguageChanged();
}


void CSkinMenuBar::draw(CRawGraph *canvas) {
    int x = 0;
    int nSpaceSubMenus = SPACE_SUBMENUS;
    CRect rcItem;

    if (m_font.isGood()) {
        canvas->setFont(m_font.getFont());
    }

    rcItem.top = m_rcObj.top;
    rcItem.bottom = m_rcObj.bottom;
    x = m_rcObj.left;

    for (int i = 0; i < (int)m_vSubMenus.size(); i++) {
        SubMenu &item = m_vSubMenus[i];
        CColor clrText, clrOutlined;
        bool bOutlined;
        CSize size;

        canvas->getTextExtentPoint32(item.strText.c_str(), item.strText.size(), &size);
        item.nWidth = size.cx + nSpaceSubMenus;

        if (x + item.nWidth > m_rcObj.right) {
            break;
        }

        rcItem.left = x;
        rcItem.right = x + item.nWidth;

        if (i == m_nSelSubMenu && (m_bLBtDown || m_wndMsgReceiver.isMenuPopuped())) {
            clrText = m_clrPressed;
            clrOutlined = m_clrOutlinedPressed;
            bOutlined = m_bOutlinedPressed;
            canvas->fillRect(&rcItem, m_clrBgPressed, BPM_OP_BLEND | BPM_CHANNEL_RGB);
        } else if (i == m_nSelSubMenu && m_bHover) {
            clrText = m_clrHover;
            clrOutlined = m_clrOutlinedHover;
            bOutlined = m_bOutlinedHover;
        } else {
            clrText = m_font.getTextColor(m_enable);
            clrOutlined = m_font.getColorOutlined();
            bOutlined = m_font.isOutlined();
        }

        if (bOutlined) {
            canvas->drawTextOutlined(item.strText.c_str(), item.strText.size(),
                rcItem, clrText, clrOutlined, DT_VCENTER | DT_CENTER | DT_PREFIX_TEXT | DT_SINGLELINE);
        } else {
            canvas->setTextColor(clrText);
            canvas->drawText(item.strText.c_str(), item.strText.size(),
                rcItem, DT_VCENTER | DT_CENTER | DT_PREFIX_TEXT | DT_SINGLELINE);
        }

        x += item.nWidth;
    }
}


bool CSkinMenuBar::onLButtonDown(uint32_t nFlags, CPoint point) {
    if (!m_rcObj.ptInRect(point)) {
        return false;
    }

    m_nSelSubMenu = hitTestSubMenu(point);
    if (m_nSelSubMenu == -1) {
        return false;
    }

    m_bLBtDown = true;

    if (m_bHover) {
        m_bHover = false;
    } else {
        m_pSkin->setCaptureMouse(this);
    }

    invalidate();

    // popup sel menu item
    if (m_vSubMenus[m_nSelSubMenu].bHasSubMenu) {
        trackPopupMenu();
        m_pSkin->releaseCaptureMouse(this);
    } else {
        m_pSkin->onCommand(m_vSubMenus[m_nSelSubMenu].nID, 0);
    }

    return true;
}


bool CSkinMenuBar::onLButtonUp(uint32_t nFlags, CPoint point) {
    // m_pSkin->releaseCaptureMouse(this);

    if (!m_bLBtDown) {
        return true;
    }

    m_bLBtDown = false;

    invalidate();

    return true;
}


bool CSkinMenuBar::onMouseMove(CPoint point) {
    int nSelSubMenuNew;

    // filter repeated mouse message when set message hook.
    if (point == m_ptLast) {
        return true;
    }
    m_ptLast = point;

    nSelSubMenuNew = hitTestSubMenu(point);
    if (nSelSubMenuNew == -1 && !m_bLBtDown && !m_bHover && !m_wndMsgReceiver.isMenuPopuped()) {
        return false;
    }

    if (isPtIn(point)) {
        if (m_wndMsgReceiver.isMenuPopuped()) {
            if (nSelSubMenuNew != m_nSelSubMenu && nSelSubMenuNew != -1) {
                m_nSelSubMenu = nSelSubMenuNew;
                invalidate();
                if (m_nSelSubMenu != -1 && m_vSubMenus[m_nSelSubMenu].bHasSubMenu) {
                    m_wndMsgReceiver.postTrackPopupMenu();
                }
            }
            return true;
        }

        if (!m_bLBtDown && !m_bHover) {
            m_bHover = true;

            m_pSkin->setCaptureMouse(this);

            m_nSelSubMenu = nSelSubMenuNew;

            invalidate();
        } else if (m_bHover || m_bLBtDown) {
            if (m_nSelSubMenu != nSelSubMenuNew) {
                m_nSelSubMenu = nSelSubMenuNew;
                invalidate();
            }
        }
    } else {
        if (m_bHover) {
            m_bHover = false;

            m_nSelSubMenu = nSelSubMenuNew;

            invalidate();
        }
    }

    return true;
}


void CSkinMenuBar::onKeyDown(uint32_t nChar, uint32_t nFlags) {
    if (m_nSelSubMenu == -1) {
        return;
    }

    switch (nChar) {
    case VK_LEFT:
        {
            if (m_nSelSubMenu > 0) {
                m_nSelSubMenu--;
            } else {
                m_nSelSubMenu = (int)m_vSubMenus.size() - 1;
            }

            if (m_nSelSubMenu >= 0 && m_nSelSubMenu < (int)m_vSubMenus.size()) {
                invalidate();
                if (m_wndMsgReceiver.isMenuPopuped()) {
                    m_wndMsgReceiver.postTrackPopupMenu();
                }
            }
        }
        break;
    case VK_RIGHT:
        {
            if (m_nSelSubMenu >= (int)m_vSubMenus.size() - 1) {
                m_nSelSubMenu = 0;
            } else {
                m_nSelSubMenu++;
            }

            if (m_nSelSubMenu >= 0 && m_nSelSubMenu < (int)m_vSubMenus.size()) {
                invalidate();
                if (m_wndMsgReceiver.isMenuPopuped()) {
                    m_wndMsgReceiver.postTrackPopupMenu();
                }
            }
        }
        break;
    }
}


bool CSkinMenuBar::onMenuKey(uint32_t nChar, uint32_t nFlags) {
    for (int i = 0; i < (int)m_vSubMenus.size(); i++) {
        if (toUpper(m_vSubMenus[i].chMenuKey) == toUpper(nChar)) {
            m_nSelSubMenu = i;
            if (m_vSubMenus[i].bHasSubMenu) {
                m_wndMsgReceiver.postTrackPopupMenu();
            } else {
                m_pSkin->onCommand(m_vSubMenus[m_nSelSubMenu].nID, 0);
            }
            return true;
        }
    }

    return false;
}


void CSkinMenuBar::onSetFocus() {
    // DBG_LOG0("onSetFocus");
}


void CSkinMenuBar::onKillFocus() {
    // DBG_LOG0("onKillFocus");

    if (m_bLBtDown) {
        m_bLBtDown = false;
        invalidate();
    }
}


bool CSkinMenuBar::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, "Menu")) {
        m_strMenu = szValue;
    } else if (m_font.setProperty(szProperty, szValue)) {
        return true;
    } else if (isPropertyName(szProperty, "TextColorHover")) {
        getColorValue(m_clrHover, szValue);
    } else if (isPropertyName(szProperty, "TextOutlinedColorHover")) {
        getColorValue(m_clrOutlinedHover, szValue);
        m_bOutlinedHover = true;
    } else if (isPropertyName(szProperty, "TextColorPressed")) {
        getColorValue(m_clrPressed, szValue);
    } else if (isPropertyName(szProperty, "TextOutlinedColorPressed")) {
        getColorValue(m_clrOutlinedPressed, szValue);
        m_bOutlinedPressed = true;
    } else if (isPropertyName(szProperty, "BgColorPressed")) {
        getColorValue(m_clrBgPressed, szValue);
    } else if (isPropertyName(szProperty, "BgPressedAlpha")) {
        m_clrBgPressed.setAlpha(atoi(szValue));
    } else {
        return false;
    }

    return true;
}


#ifdef _SKIN_EDITOR_
void CSkinMenuBar::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);
}
#endif // _SKIN_EDITOR_


void CSkinMenuBar::onLanguageChanged() {
    if (m_pMenu) {
        delete m_pMenu;
        m_pMenu = nullptr;
    }
    m_vSubMenus.clear();

    if (m_strMenu.size()) {
        m_pSkin->getSkinFactory()->loadMenu(m_pSkin, &m_pMenu, m_strMenu.c_str());
    } else if (!isEmptyString(m_pSkin->getMenuName())) {
        m_pSkin->getSkinFactory()->loadMenu(m_pSkin, &m_pMenu,
            m_pSkin->getMenuName());
    }

    if (m_pMenu) {
        int count = m_pMenu->getItemCount();
        for (int i = 0; i < count; i++) {
            SubMenu item;

            if (m_pMenu->getMenuItemInfo(i, item.strText, item.nID, item.bHasSubMenu, true)) {
                item.chMenuKey = getMenuKey(item.strText.c_str());
                m_vSubMenus.push_back(item);
            }
        }
    }
}

int CSkinMenuBar::hitTestSubMenu(const CPoint point) {
    int x = m_rcObj.left;

    if (point.x < m_rcObj.left) {
        return -1;
    }

    for (int i = 0; i < (int)m_vSubMenus.size(); i++) {
        x += m_vSubMenus[i].nWidth;
        if (x > m_rcObj.right) {
            return -1;
        }

        if (point.x < x) {
            return i;
        }
    }

    return -1;
}


int CSkinMenuBar::getItemX(int n) {
    int x = m_rcObj.left;

    if (n >= (int)m_vSubMenus.size()) {
        n = (int)m_vSubMenus.size() - 1;
    }
    for (int i = 0; i < n; i++) {
        x += m_vSubMenus[i].nWidth;
        if (x > m_rcObj.right) {
            return x;
        }
    }

    return x;
}


void CSkinMenuBar::trackPopupMenu() {
    if (m_nSelSubMenu == -1 || m_pMenu == nullptr) {
        return;
    }

    if (m_pSkin->getCaptureMouse() != this) {
        m_pSkin->setCaptureMouse(this);
    }

    CPoint pt;
    CRect rc, rcMonitor;

    pt.x = getItemX(m_nSelSubMenu);
    pt.y = m_rcObj.bottom;

    rc = m_rcObj;
    rc.left = pt.x;
    rc.right = rc.left + m_vSubMenus[m_nSelSubMenu].nWidth;
    m_pSkin->clientToScreen(rc);

    m_pSkin->clientToScreen(pt);

    getMonitorRestrictRect(rc, rcMonitor);
    if (rc.left < rcMonitor.left) {
        rc.right = rcMonitor.left + rc.width();
        pt.x = rc.left = rcMonitor.left;
    }

    m_bLBtDown = false;

    m_wndMsgReceiver.trackPopupSubMenu(pt.x, pt.y, m_pMenu, m_nSelSubMenu, m_pSkin, &rc);
}
