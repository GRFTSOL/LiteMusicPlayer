// SESkinContainer.cpp: implementation of the CSESkinContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "SESkinWnd.h"
#include "SESkinContainer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
/*

class CSESkinContainer : public CSkinContainer  
{
public:
    CSESkinContainer();
    virtual ~CSESkinContainer();

    void onCreate();

    void onKillFocus();
    void onSetFocus();

    void draw(IMLGraphicsBase *canvas);

    bool setProperty(cstr_t szProperty, cstr_t szValue);

    void setNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }

    //
    // CSkinContainer API
    //
    virtual CUIObject *getFocusChild();
    virtual void setFocusChild(CUIObject *pObjChild);

    // if no ctrl can be set focused or reached to the end of ctrl, returns false.
    virtual bool focusNextChild();
    virtual bool focusPrevChild();

protected:
    void switchChildFocus(CUIObject *pUIObjFocusOld, CUIObject *pUIObjFocusNew);


    ISESkinNotify    *m_pNotify;

};


#define INDEX_FOCUS_SELF        -2
#define SIZE_MARGIN                0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSESkinContainer::CSESkinContainer()
{
    m_pNotify = nullptr;
}

CSESkinContainer::~CSESkinContainer()
{

}

void CSESkinContainer::onCreate()
{
    CSkinContainer::onCreate();
}

void CSESkinContainer::onKillFocus()
{
    m_pSkin->invalidateRect(nullptr, true);
}

void CSESkinContainer::onSetFocus()
{
    invalidate();
}

void CSESkinContainer::draw(IMLGraphicsBase *canvas)
{
    HDC        hdc = canvas->getHandle();

    CRect    rc;
    rc = m_rcObj;
    rc.left += SIZE_MARGIN;
    rc.top += SIZE_MARGIN;
    rc.right -= SIZE_MARGIN;
    rc.bottom -= SIZE_MARGIN;

    CSkinContainer::draw(canvas);

    if (isOnFocus() && m_nFocusChild == INDEX_FOCUS_SELF)
    {
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

CUIObject *CSESkinContainer::getFocusChild()
{
    if (m_nFocusChild == INDEX_FOCUS_SELF)
        return this;
    else
        return CSkinContainer::getFocusChild();
}

void CSESkinContainer::setFocusChild(CUIObject *pObjChild)
{
    /// no equal..........???????
    if (pObjChild == this)
    {
        m_nFocusChild = INDEX_FOCUS_SELF;
        m_pSkin->invalidateRect();
    }
    else
        CSkinContainer::setFocusChild(pObjChild);
}

// if no ctrl can be set focused or reached to the end of ctrl, returns false.
bool CSESkinContainer::focusNextChild()
{
    if (m_nFocusChild == INDEX_FOCUS_SELF)
    {
        m_nFocusChild = -1;
        return CSkinContainer::focusNextChild();
    }
    else if (m_nFocusChild == -1)
    {
        // set focus to self
        CUIObject    *pObjFocusOld = m_pSkin->getFocusUIObj();

        m_nFocusChild = INDEX_FOCUS_SELF;
        switchChildFocus(pObjFocusOld, this);

        m_pSkin->invalidateRect();
        return true;
    }
    else
        return CSkinContainer::focusNextChild();
}

bool CSESkinContainer::focusPrevChild()
{
    if (m_nFocusChild == INDEX_FOCUS_SELF)
    {
        m_nFocusChild = -1;
        invalidate();
        return false;
    }
    else
    {
        if (CSkinContainer::focusPrevChild())
            return true;
        else
        {
            CUIObject    *pObjFocusOld = m_pSkin->getFocusUIObj();

            m_nFocusChild = INDEX_FOCUS_SELF;
            switchChildFocus(pObjFocusOld, this);
            m_pSkin->invalidateRect();
            return true;
        }
    }
}

void CSESkinContainer::switchChildFocus(CUIObject *pUIObjFocusOld, CUIObject *pUIObjFocusNew)
{
    if (pUIObjFocusOld != pUIObjFocusNew)
    {
        if (pUIObjFocusOld)
            pUIObjFocusOld->onKillFocus();

        if (pUIObjFocusNew)
            pUIObjFocusNew->onSetFocus();
    }
}

bool CSESkinContainer::setProperty(cstr_t szProperty, cstr_t szValue)
{
    bool    bRet = CSkinContainer::setProperty(szProperty, szValue);

    if (strcasecmp(szProperty, SZ_PN_NAME) == 0)
    {
        if (m_pNotify)
            m_pNotify->onUIObjNameChanged(this);

        return true;
    }

    return bRet;
}
*/