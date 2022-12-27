#include "SkinTypes.h"
#include "Skin.h"
#include "PopupSkinListWnd.h"


CPopupSkinListWnd::CPopupSkinListWnd() {
    m_nCurSel = -1;
    m_pListCtrl = nullptr;
    m_nLineHeight = 0;

    m_pListCtrl = &m_listCtrlData;
}

CPopupSkinListWnd::~CPopupSkinListWnd() {

}

void CPopupSkinListWnd::onLoadWndSizePos() {
}

void CPopupSkinListWnd::onSkinLoaded() {
    CSkinListCtrl *pListCtrl = (CSkinListCtrl *)getUIObjectByClassName(CSkinListCtrl::className());
    assert(pListCtrl);
    if (!pListCtrl) {
        return;
    }

    m_pListCtrl = pListCtrl;

    m_pListCtrl->m_imageListRightSpace = 14;

    m_pListCtrl->m_vHeading = m_listCtrlData.m_vHeading;
    m_pListCtrl->m_vRows = m_listCtrlData.m_vRows;

    m_listCtrlData.m_vHeading.clear();
    m_listCtrlData.m_vRows.clear();

    if (m_nLineHeight > 0) {
        m_pListCtrl->setLineHeight(m_nLineHeight);
    }

    if (m_nCurSel != -1) {
        m_pListCtrl->selectRangeRow(m_nCurSel, m_nCurSel);
    }

    int nContentHeight = m_pListCtrl->getContentHeight();
    CRect rcOrg;
    getWindowRect(&rcOrg);

    CRect rcNew = rcOrg;
    CRect rcMonitor;
    getMonitorRestrictRect(rcOrg, rcMonitor);

    if (m_pListCtrl->m_rcObj.height() != nContentHeight) {
        rcNew.bottom = rcNew.bottom - m_pListCtrl->m_rcObj.height() + nContentHeight;

        if (rcNew.bottom > rcMonitor.bottom) {
            rcNew.top -= rcNew.bottom - rcMonitor.bottom;
            if (rcNew.top < rcMonitor.top) {
                rcNew.top = rcMonitor.top;
            }
            rcNew.bottom = rcMonitor.bottom;
        }
    }

    if (rcNew.right > rcMonitor.right) {
        rcNew.left -= rcNew.right - rcMonitor.right;
        rcNew.right = rcMonitor.right;
    }

    if (rcNew != rcOrg) {
        moveWindow(rcNew, false);
    }

    CPopupSkinWnd::onSkinLoaded();
}

void CPopupSkinListWnd::closeSkin() {
    if (m_bSkinOpened && m_pListCtrl) {
        m_listCtrlData.m_vHeading = m_pListCtrl->m_vHeading;
        m_listCtrlData.m_vRows = m_pListCtrl->m_vRows;

        m_pListCtrl->m_vHeading.clear();
        m_pListCtrl->m_vRows.clear();

        m_pListCtrl = &m_listCtrlData;
    }

    CPopupSkinWnd::closeSkin();
}

void CPopupSkinListWnd::onUIObjNotify(IUIObjNotify *pNotify) {
    if (!m_pPopupSkinWndNotify) {
        return;
    }

    assert(m_pListCtrl);
    assert(pNotify->isKindOf(CSkinListCtrl::className()));

    CSkinListCtrlEventNotify *pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
    if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_CLICK
        || pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_ENTER) {
        setCurSel(m_pListCtrl->getNextSelectedItem());
        m_pPopupSkinWndNotify->popupSkinWndOnSelected();
        postDestroy();
    }
}

int CPopupSkinListWnd::getCurSel() {
    return m_nCurSel;
}

int CPopupSkinListWnd::getItemCount() const {
    if (m_pListCtrl) {
        return m_pListCtrl->getItemCount();
    } else {
        assert(0);
        return 0;
    }
}

void CPopupSkinListWnd::setCurSel(int nIndex) {
    if (nIndex >= (int)getItemCount()) {
        return;
    }

    assert(m_pListCtrl);
    if (!m_pListCtrl) {
        return;
    }

    if (m_nCurSel != -1) {
        m_pListCtrl->setItemImageIndex(m_nCurSel, 0);
    }

    m_nCurSel = nIndex;

    if (m_nCurSel != -1) {
        m_pListCtrl->setItemImageIndex(m_nCurSel, 1);
    }
}

bool CPopupSkinListWnd::deleteItem(int nItem) {
    assert(m_pListCtrl);
    if (m_pListCtrl) {
        return m_pListCtrl->deleteItem(nItem, false);
    } else {
        return false;
    }
}

bool CPopupSkinListWnd::deleteAllItems() {
    assert(m_pListCtrl);
    if (m_pListCtrl) {
        return m_pListCtrl->deleteAllItems(false);
    } else {
        return false;
    }
}

int CPopupSkinListWnd::insertItem(int nItem, cstr_t lpszItem, int nImageIndex, uint32_t nItemData) {
    assert(m_pListCtrl);
    if (m_pListCtrl) {
        return m_pListCtrl->insertItem(nItem, lpszItem, nImageIndex, nItemData, false);
    } else {
        return -1;
    }
}

bool CPopupSkinListWnd::setItemText(int nItem, int nSubItem, cstr_t lpszText) {
    assert(m_pListCtrl);
    if (m_pListCtrl) {
        return m_pListCtrl->setItemText(nItem, nSubItem, lpszText);
    } else {
        return false;
    }
}

bool CPopupSkinListWnd::setItemImage(int nItem, int nSubItem, RawImageData *image) {
    assert(m_pListCtrl);
    if (m_pListCtrl) {
        return m_pListCtrl->setItemImage(nItem, nSubItem, image);
    } else {
        return false;
    }
}

string CPopupSkinListWnd::getItemText(int nItem, int nSubItem) const {
    string str;
    assert(m_pListCtrl);
    if (m_pListCtrl) {
        m_pListCtrl->getItemText(nItem, nSubItem, str);
    }
    return str;
}

void CPopupSkinListWnd::setItemData(int nItem, uint32_t nItemData) {
    assert(m_pListCtrl);
    if (m_pListCtrl) {
        m_pListCtrl->setItemData(nItem, nItemData);
    }
}

uint32_t CPopupSkinListWnd::getItemData(int nItem) {
    assert(m_pListCtrl);
    if (m_pListCtrl) {
        return m_pListCtrl->getItemData(nItem);
    } else {
        return 0;
    }
}
