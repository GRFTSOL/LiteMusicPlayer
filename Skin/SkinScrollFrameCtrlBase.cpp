#include "SkinTypes.h"
#include "Skin.h"
#include "SkinScrollFrameCtrlBase.h"
#include "SkinFrameCtrl.h"
#include "SkinListCtrl.h"

CSkinScrollFrameCtrlBase::CSkinScrollFrameCtrlBase()
{
    m_bHorzScrollBar = m_bVertScrollBar = false;
    m_bEnableBorder = false;

    m_pObjVertScrollBar = nullptr;
    m_pObjHorzScrollBar = nullptr;
    m_nHorzScrollBarId = m_nVertScrollBarId = UID_INVALID;
    m_pVertScrollBar = nullptr;
    m_pHorzScrollBar = nullptr;
    m_pObjFrame = nullptr;

    m_nWidthVertScrollBar = m_nHeightHorzScrollBar = 0;
}

CSkinScrollFrameCtrlBase::~CSkinScrollFrameCtrlBase()
{
    if (m_nVertScrollBarId == UID_INVALID && m_pObjVertScrollBar)
    {
        m_pSkin->removeUIObject(m_pObjVertScrollBar, true);
        m_pObjVertScrollBar = nullptr;
    }
    if (m_nHorzScrollBarId == UID_INVALID && m_pObjHorzScrollBar)
    {
        m_pSkin->removeUIObject(m_pObjHorzScrollBar, true);
        m_pObjHorzScrollBar = nullptr;
    }
}

void CSkinScrollFrameCtrlBase::onCreate()
{
    CSkinContainer::onCreate();

    if (m_bEnableBorder)
        createFrameCtrl();

    if (m_bVertScrollBar)
        createVertScrollbar();
    if (m_bHorzScrollBar)
        createHorzScrollbar();
}

void CSkinScrollFrameCtrlBase::onSize()
{
    CSkinContainer::onSize();

    m_rcContent = m_rcObj;
    m_rcContent.left += m_rcPadding.left;
    m_rcContent.top += m_rcPadding.top;
    m_rcContent.right -= m_rcPadding.right;
    m_rcContent.bottom -= m_rcPadding.bottom;

    m_rcContent.right -= m_nWidthVertScrollBar;
    m_rcContent.bottom -= m_nHeightHorzScrollBar;
}

void CSkinScrollFrameCtrlBase::onKillFocus()
{
    if (m_pObjFrame)
        m_pSkin->invalidateRect();
}

void CSkinScrollFrameCtrlBase::onSetFocus()
{
    if (m_pObjFrame)
        m_pSkin->invalidateRect();
}

void CSkinScrollFrameCtrlBase::onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt)
{
    if (isFlagSet(nMkeys, MK_SHIFT))
    {
        if (m_pObjHorzScrollBar)
            m_pObjHorzScrollBar->onMouseWheel(nWheelDistance, nMkeys, pt);
    }
    else if (m_pObjVertScrollBar)
        m_pObjVertScrollBar->onMouseWheel(nWheelDistance, nMkeys, pt);
}

bool CSkinScrollFrameCtrlBase::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (CSkinContainer::setProperty(szProperty, szValue))
        return true;

    if (isPropertyName(szProperty, "EnableBorder"))
        m_bEnableBorder = isTRUE(szValue);
    else if (isPropertyName(szProperty, "EnableHorzScrollBar"))
        m_bHorzScrollBar = isTRUE(szValue);
    else if (isPropertyName(szProperty, "EnableVertScrollBar"))
        m_bVertScrollBar = isTRUE(szValue);
    else if (isPropertyName(szProperty, "ScrollbarId"))
    {
        VecStrings    vStr;
        strSplit(szValue, ',', vStr);
        trimStr(vStr);
        if (vStr.size() > 0)
            m_nVertScrollBarId = m_pSkin->getSkinFactory()->getIDByName(vStr[0].c_str());
        if (vStr.size() > 1)
            m_nHorzScrollBarId = m_pSkin->getSkinFactory()->getIDByName(vStr[1].c_str());
    }
    else
        return false;

    return true;
}


void CSkinScrollFrameCtrlBase::createFrameCtrl()
{
    m_pObjFrame = m_pSkin->getSkinFactory()->createDynamicCtrl(this,
        isKindOf(CSkinListCtrl::className()) ? "ListCtrlFrame" : "EditCtrlFrame",
        ID_UNDEFINE, "0", "0", "w", "h");

    assert(m_pObjFrame);
    if (m_pObjFrame)
    {
        CSkinFrameCtrl *pFrameCtrl = (CSkinFrameCtrl *)m_pObjFrame;
        int nBorderThick = pFrameCtrl->getBorderThick();
//         m_formLeft.increase(-nBorderThick);
//         m_formTop.increase(-nBorderThick);
//         m_formWidth.increase(nBorderThick * 2);
//         m_formHeight.increase(nBorderThick * 2);

        pFrameCtrl->setFocusIndicator(this);

        m_rcPadding.left = m_rcPadding.top = m_rcPadding.right = m_rcPadding.bottom = nBorderThick;
    }
}

void CSkinScrollFrameCtrlBase::createVertScrollbar()
{
    if (m_nVertScrollBarId != UID_INVALID)
    {
        m_pObjVertScrollBar = m_pSkin->getUIObjectById(m_nVertScrollBarId, CSkinVScrollBar::className());
        if (m_pObjVertScrollBar)
        {
            m_pVertScrollBar = (CSkinVScrollBar*)m_pObjVertScrollBar;
            m_pVertScrollBar->setScrollNotify(this);
            return;
        }
        else
            ERR_LOG0("Can NOT get the vertical scroll bar of list control.");
    }

    // create Scrollbar
    m_pObjVertScrollBar = m_pSkin->getSkinFactory()->createDynamicCtrl(this, CSkinVScrollBar::className(), UID_INVALID, nullptr, nullptr, nullptr, nullptr);
    assert(m_pObjVertScrollBar);
    if (!m_pObjVertScrollBar)
        return;

    int nWidth = 14;
    m_pObjVertScrollBar->m_formWidth.calCualteValue(nWidth);
    m_nWidthVertScrollBar = nWidth;

    m_pObjVertScrollBar->m_formLeft.setFormula(CStrPrintf("w-%d", m_nWidthVertScrollBar + m_rcPadding.right).c_str());
    m_pObjVertScrollBar->m_formTop.setFormula(m_rcPadding.top);
    m_pObjVertScrollBar->m_formHeight.setFormula(CStrPrintf("h-%d", m_rcPadding.top + m_rcPadding.bottom + m_nHeightHorzScrollBar).c_str());
    if (m_pObjHorzScrollBar)
    {
        // Horizontal scroll bar is created first, decrease its width now.
        m_pObjHorzScrollBar->m_formWidth.increase(-nWidth);
    }

    m_pContainer->recalculateUIObjSizePos(m_pObjVertScrollBar);

    m_pVertScrollBar = (CSkinVScrollBar*)m_pObjVertScrollBar;
    m_pVertScrollBar->setScrollNotify(this);

    m_rcContent.right -= m_nWidthVertScrollBar;
}

void CSkinScrollFrameCtrlBase::createHorzScrollbar()
{
    if (m_nHorzScrollBarId != UID_INVALID)
    {
        m_pObjHorzScrollBar = m_pSkin->getUIObjectById(m_nHorzScrollBarId, CSkinHScrollBar::className());
        if (m_pObjHorzScrollBar)
        {
            m_pHorzScrollBar = (CSkinHScrollBar*)m_pObjHorzScrollBar;
            m_pHorzScrollBar->setScrollNotify(this);
            return;
        }
        else
            ERR_LOG0("Can NOT get the Horzical scroll bar of list control.");
    }

    // create Scrollbar
    m_pObjHorzScrollBar = m_pSkin->getSkinFactory()->createDynamicCtrl(this, CSkinHScrollBar::className(), UID_INVALID, nullptr, nullptr, nullptr, nullptr);
    assert(m_pObjHorzScrollBar);
    if (!m_pObjHorzScrollBar)
        return;

    int nHeight = 14;
    m_pObjHorzScrollBar->m_formHeight.calCualteValue(nHeight);
    m_nHeightHorzScrollBar = nHeight;

    m_pObjHorzScrollBar->m_formLeft.setFormula(m_rcPadding.left);
    m_pObjHorzScrollBar->m_formTop.setFormula(CStrPrintf("h-%d", m_rcPadding.bottom + m_nHeightHorzScrollBar).c_str());
    m_pObjHorzScrollBar->m_formWidth.setFormula(CStrPrintf("w-%d", m_rcPadding.left + m_rcPadding.right + m_nWidthVertScrollBar).c_str());
    if (m_pObjVertScrollBar)
    {
        // Vertical scroll bar is created first, decrease its height now.
        m_pObjVertScrollBar->m_formHeight.increase(-nHeight);
    }

    m_pContainer->recalculateUIObjSizePos(m_pObjHorzScrollBar);

    m_pHorzScrollBar = (CSkinVScrollBar*)m_pObjHorzScrollBar;
    m_pHorzScrollBar->setScrollNotify(this);

    m_rcContent.bottom -= m_nHeightHorzScrollBar;
}
