#include "SkinTypes.h"
#include "Skin.h"
#include "PopupSkinWnd.h"


CPopupSkinWnd::CPopupSkinWnd() {
    m_pPopupSkinWndNotify = nullptr;
}

CPopupSkinWnd::~CPopupSkinWnd() {

}

int CPopupSkinWnd::create(CSkinFactory *pSkinFactory, cstr_t szSkinWndName,
    Window *pWndParent, IPopupSkinWndNotify *pNotify, const CRect &rc) {
    m_rcBoundBox = rc;

    SkinWndStartupInfo skinWndStartupInfo("ZPMenu", "menu", szSkinWndName, pWndParent);
    int nRet = CSkinWnd::create(skinWndStartupInfo, pSkinFactory, true, true);
    if (nRet != ERR_OK) {
        return nRet;
    }

    m_pPopupSkinWndNotify = pNotify;
    return ERR_OK;
}

void CPopupSkinWnd::onKillFocus() {
    //    if (m_pPopupSkinWndNotify)
    //        m_pPopupSkinWndNotify->popupSkinWndOnDestroy();

    postDestroy();
}

bool CPopupSkinWnd::onKeyDown(uint32_t nChar, uint32_t nFlags) {
    CUIObject *pObjFocus = getFocusUIObj();

    // let focus uiobject to process key message
    if (pObjFocus && pObjFocus->needMsgKey()) {
        if (pObjFocus->onKeyDown(nChar, nFlags)) {
            return true;
        }
    }

    if (nChar == VK_ESCAPE) {
        postDestroy();
        return true;
    }

    return CSkinWnd::onKeyDown(nChar, nFlags);
}
