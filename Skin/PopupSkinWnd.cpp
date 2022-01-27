#include "SkinTypes.h"
#include "Skin.h"
#include "PopupSkinWnd.h"

CPopupSkinWnd::CPopupSkinWnd()
{
    m_pPopupSkinWndNotify = nullptr;
}

CPopupSkinWnd::~CPopupSkinWnd()
{

}

int CPopupSkinWnd::create(CSkinFactory *pSkinFactory, cstr_t szSkinWndName, 
            Window *pWndParent, IPopupSkinWndNotify *pNotify, const CRect &rc)
{
    m_rcReal = rc;

    SkinWndStartupInfo skinWndStartupInfo("ZPMenu", "menu", szSkinWndName, pWndParent);
    int nRet = CSkinWnd::create(skinWndStartupInfo, pSkinFactory, true, true);
    if (nRet != ERR_OK)
        return nRet;

    m_pPopupSkinWndNotify = pNotify;
    return ERR_OK;
}

void CPopupSkinWnd::onKillFocus()
{
//    if (m_pPopupSkinWndNotify)
//        m_pPopupSkinWndNotify->popupSkinWndOnDestroy();

    postDestroy();
}

void CPopupSkinWnd::onKeyDown(uint32_t nChar, uint32_t nFlags)
{
    CUIObject    *pObjFocus = getFocusUIObj();

    // let focus uiobject to process key message
    if (pObjFocus && pObjFocus->needMsgAllKeys())
    {
        pObjFocus->onKeyDown(nChar, nFlags);
        return;
    }

    if (nChar == VK_ESCAPE)
    {
        postDestroy();
        return;
    }

    CSkinWnd::onKeyDown(nChar, nFlags);
}
