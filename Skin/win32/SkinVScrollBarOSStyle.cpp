#include "SkinTypes.h"
#include "Skin.h"
#include "SkinVScrollBarOSStyle.h"


//////////////////////////////////////////////////////////////////////////

CSkinScrollBarOSStyleBase::CSkinScrollBarOSStyleBase() {
    m_pScrollNotify = nullptr;
}

CSkinScrollBarOSStyleBase::~CSkinScrollBarOSStyleBase() {
    m_scrollbar.destroy();
}

void CSkinScrollBarOSStyleBase::onSize() {
    CUIObject::onSize();
    m_scrollbar.moveWindow(m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), true);
}

void CSkinScrollBarOSStyleBase::onSetFocus() {
    m_scrollbar.setFocus();
}

void CSkinScrollBarOSStyleBase::onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) {
    if (nMkeys == 0) {
        if (nWheelDistance > 0) {
            m_scrollbar.handleScrollCode(SB_LINEUP, 0);
        } else {
            m_scrollbar.handleScrollCode(SB_LINEDOWN, 0);
        }
    }
}

void CSkinScrollBarOSStyleBase::onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) {
    if (m_pScrollNotify) {
        m_pScrollNotify->onVScroll(nSBCode, getScrollPos(), this);
    } else {
        m_pSkin->onVScroll(nSBCode, getScrollPos(), this);
    }
}

void CSkinScrollBarOSStyleBase::onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) {
    if (m_pScrollNotify) {
        m_pScrollNotify->onVScroll(nSBCode, getScrollPos(), this);
    } else {
        m_pSkin->onVScroll(nSBCode, getScrollPos(), this);
    }
}


UIOBJECT_CLASS_NAME_IMP(CSkinVScrollBarOSStyle, "VScrollBarOSStyle")

CSkinVScrollBarOSStyle::CSkinVScrollBarOSStyle() {
    m_formWidth.setFormula(GetSystemMetrics(SM_CXVSCROLL));
}

void CSkinVScrollBarOSStyle::onCreate() {
    m_scrollbar.create(m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), true, m_pSkin, m_id);
    m_scrollbar.setScrollNotify(this);

    m_scrollbar.showWindow(SW_SHOW);
}

UIOBJECT_CLASS_NAME_IMP(CSkinHScrollBarOSStyle, "HScrollBarOSStyle")

CSkinHScrollBarOSStyle::CSkinHScrollBarOSStyle() {
    m_formHeight.setFormula(GetSystemMetrics(SM_CYHSCROLL));
}

void CSkinHScrollBarOSStyle::onCreate() {
    m_scrollbar.create(m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), false, m_pSkin, m_id);
    m_scrollbar.setScrollNotify(this);

    m_scrollbar.showWindow(SW_SHOW);
}
