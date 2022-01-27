// UIEditObject.cpp: implementation of the CUIEditObject class.
//
//////////////////////////////////////////////////////////////////////

#include "resource.h"
#include "UIEditObject.h"
#include "SESkinWnd.h"
#include "SEPropertyListCtrl.h"

#define SIZE_MARGIN        0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CUIEditObject, "SkinEditObject")

CUIEditObject::CUIEditObject()
{
    // m_nMsgMask = UO_MSG_MASK_KEY | UO_MSG_MASK_MOUSEWHEEL | UO_MSG_MASK_RBUTTON;
    m_pUIObject = nullptr;
    m_bLBtDown = false;
    static uint8_t    r = 0, g = 0, b = 0;
    r += 75;
    g += 50;
    b += 25;
    m_hbrBk = CreateSolidBrush(RGB(r, g, b));
    m_msgNeed = UO_MSG_WANT_ALL;
    m_pNotify = nullptr;
}

CUIEditObject::~CUIEditObject()
{
    if (m_pUIObject)
        delete m_pUIObject;
    if (m_hbrBk)
        DeleteObject(m_hbrBk);
}

void CUIEditObject::onCreate()
{
    CUIObject::onCreate();

    m_pUIObject->m_pContainer = m_pContainer;
    m_pUIObject->onCreate();
}

bool CUIEditObject::onLButtonDown(uint32_t nFlags, CPoint point)
{
    assert(m_pSkin);

    m_bLBtDown = true;
    m_ptDragOld = point;

    // 捕捉鼠标输入
    m_pSkin->setCaptureMouse(this);

//    invalidate();
    return true;
}

bool CUIEditObject::onLButtonUp(uint32_t nFlags, CPoint point)
{
    m_bLBtDown = false;

    // 释放鼠标输入
    m_pSkin->releaseCaptureMouse(this);

    //invalidate();
    
    //m_pSkin->onCustomCommand(m_id);
    return true;
}

bool CUIEditObject::onMouseMove(uint32_t nFlags, CPoint point)
{
    assert(m_pSkin);

    if (m_bLBtDown)
    {
        //
        // 移动此控件
        CString        str;
        // int            nNewPos;
        int            nOffset;

        nOffset = point.x - m_ptDragOld.x;
        // nNewPos = m_rcObj.left + point.x - m_ptDragOld.x + SIZE_MARGIN;
        if (nOffset != 0)
        {
            if (nOffset > 0)
                str.Format("%s + %d", m_formLeft.getFormula(), nOffset);
            else
                str.Format("%s%d", m_formLeft.getFormula(), nOffset);
            setProperty("left", str);

            if (m_pNotify)
                m_pNotify->onFocusUIObjPropertyChanged("left", m_formLeft.getFormula());
        }

        nOffset = point.y - m_ptDragOld.y;
        // nNewPos = m_rcObj.top + point.y - m_ptDragOld.y + SIZE_MARGIN;
        if (nOffset != 0)
        {
            if (nOffset > 0)
                str.Format("%s + %d", m_formTop.getFormula(), nOffset);
            else
                str.Format("%s%d", m_formTop.getFormula(), nOffset);
            setProperty("top", str);
            if (m_pNotify)
                m_pNotify->onFocusUIObjPropertyChanged("top", m_formTop.getFormula());
        }

        m_pSkin->RecalculateSizePos();

        m_pSkin->invalidateRect(nullptr, true);

        m_ptDragOld = point;
    }
    return true;
}

void CUIEditObject::draw(CRawGraph *canvas)
{
    HDC        hdc = canvas->getHandle();

    CRect    rc;
    rc = m_rcObj;
    rc.left += SIZE_MARGIN;
    rc.top += SIZE_MARGIN;
    rc.right -= SIZE_MARGIN;
    rc.bottom -= SIZE_MARGIN;
    fillRect(hdc, &rc, m_hbrBk);

    m_pUIObject->draw(canvas);

    if (isOnFocus())
    {
    //    BitBlt(hdc, m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), nullptr, 0, 0, WHITENESS);
        HPEN    hPen, hPenOld;
        COLORREF    clrOld;
        int            nBkModeOld;

        hPen = CreatePen(PS_DOT, 1, RGB(0x31, 0x69, 0xc6));
        hPenOld = (HPEN)SelectObject(hdc, hPen);
        clrOld = setBkColor(hdc, RGB(255, 255, 255));
        nBkModeOld = SetBkMode(hdc, OPAQUE);

        // 画边缘
        // for (int i = 0; i < SIZE_MARGIN; i+=2)
        int    i = 0;
        {
            MoveToEx(hdc, m_rcObj.left + i, m_rcObj.top + i, nullptr);
            LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.top + i);

            MoveToEx(hdc, m_rcObj.left + i, m_rcObj.bottom - i - 1, nullptr);
            LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.bottom - i - 1);

            MoveToEx(hdc, m_rcObj.left + i, m_rcObj.top + i, nullptr);
            LineTo(hdc, m_rcObj.left + i, m_rcObj.bottom - i - 1);

            MoveToEx(hdc, m_rcObj.right - i - 1, m_rcObj.top + i, nullptr);
            LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.bottom - i - 1);
        }

        SetBkMode(hdc, nBkModeOld);
        setBkColor(hdc, clrOld);
        SelectObject(hdc, hPenOld);

        DeleteObject(hPen);
    }
}

bool CUIEditObject::reCalculatePos(FORMULA_VAR vars[])
{
    if (!CUIObject::reCalculatePos(vars))
        return false;

    m_pUIObject->reCalculatePos(vars);

    m_rcObj.left -= SIZE_MARGIN;
    m_rcObj.top -= SIZE_MARGIN;
    m_rcObj.right += SIZE_MARGIN;
    m_rcObj.bottom += SIZE_MARGIN;

    return true;
}


void CUIEditObject::enumProperties(CUIObjProperties &listProperties)
{
    m_pUIObject->enumProperties(listProperties);
}

bool CUIEditObject::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (strcasecmp(szProperty, "visible") != 0)
        CUIObject::setProperty(szProperty, szValue);

    if (strcasecmp(szProperty, SZ_PN_NAME) == 0)
    {
        m_pUIObject->setProperty(szProperty, szValue);

        if (m_pNotify)
            m_pNotify->onUIObjNameChanged(this);

        return true;
    }

    return m_pUIObject->setProperty(szProperty, szValue);
}

/*
int CUIEditObject::fromXML(SXNode *pXmlNode)
{
    for (uint32_t i = 0; i < pXmlNode->properties.size(); i += 2)
    {
        setProperty(pXmlNode->properties[i], pXmlNode->properties[i + 1]);
    }

    return ERR_OK
    // return m_pUIObject->fromXML(pXmlNode);
}*/

int CUIEditObject::fromXML(CSkinContainer *pContainer, SXNode *pXmlNode)
{
    CUIObject::fromXML(pContainer, pXmlNode);
    return m_pUIObject->fromXML(pContainer, pXmlNode);
}

void CUIEditObject::toXML(CXMLWriter &xmlStream)
{
    m_pUIObject->toXML(xmlStream);
}

void CUIEditObject::onKillFocus()
{
    m_pSkin->invalidateRect(nullptr, true);
}

void CUIEditObject::onSetFocus()
{
    invalidate();
}
