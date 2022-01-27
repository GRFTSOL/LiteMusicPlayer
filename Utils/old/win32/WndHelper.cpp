// WndHelper.cpp: implementation of the CWndHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "WndHelper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWndHelper::CWndHelper()
{
    m_hWndParent = nullptr;
}

CWndHelper::~CWndHelper()
{

}

void CWndHelper::add(HWND hWnd, bool bAlignRight, bool bAlignBottom, bool bSizeXFix, bool bSizeYFix, HWND hWndRelation)
{
    ChildWndProp        childWnd;

    childWnd.hWnd = hWnd;
    childWnd.bAlignBottom = bAlignBottom;
    childWnd.bAlignRight = bAlignRight;
    childWnd.bSizeXFix = bSizeXFix;
    childWnd.bSizeYFix = bSizeYFix;
    childWnd.hWndRelation = hWndRelation;

    CRect    rc, rcRalation;

    getWindowRect(hWnd, &rc);

    if (hWndRelation)
    {
        getWindowRect(hWndRelation, &rcRalation);
    }
    else
    {
        getWindowRect(m_hWndParent, &rcRalation);
        rcRalation.left = rcRalation.right;
        rcRalation.top = rcRalation.bottom;
    }

    if (bAlignRight)
    {
        childWnd.nMarginX = rcRalation.right - rc.right;
    }

    if (bAlignBottom)
        childWnd.nMarginY = rcRalation.bottom - rc.bottom;

    m_vChildWnd.push_back(childWnd);
}

void CWndHelper::addAlignCenter(HWND hWnd, bool bAlignVert)
{
    ChildAlignCenter    align;
    CRect                rc, rcChild;

    align.hWnd = hWnd;
    align.bVert = bAlignVert;

    getWindowRect(m_hWndParent, &rc);
    getWindowRect(hWnd, &rcChild);
    if (bAlignVert)
        align.nOffsetToCenter = (rc.top + rc.bottom) / 2 - rcChild.top;
    else
        align.nOffsetToCenter = (rc.left + rc.right) / 2 - rcChild.left;

    m_vAlignCenterWnds.push_back(align);
}

void CWndHelper::add(int nChildID, bool bAlignRight, bool bAlignBottom, bool bSizeXFix, bool bSizeYFix, HWND hWndRelation)
{
    add(::getDlgItem(m_hWndParent, nChildID), bAlignRight, bAlignBottom, bSizeXFix, bSizeYFix, hWndRelation);
}

void CWndHelper::addAlignCenter(int nChildID, bool bAlignVert)
{
    addAlignCenter(::getDlgItem(m_hWndParent, nChildID), bAlignVert);
}

void CWndHelper::addAlignRight(int nChildID, bool bSizeXFixed)
{
    add(::getDlgItem(m_hWndParent, nChildID), true, false, bSizeXFixed, true, nullptr);
}

void CWndHelper::addAlignBottom(int nChildID, bool bSizeYFixed)
{
    add(::getDlgItem(m_hWndParent, nChildID), false, true, true, bSizeYFixed, nullptr);
}

void CWndHelper::addAlignRightBottom(int nChildID, bool bSizeXFixed, bool bSizeYFixed)
{
    add(::getDlgItem(m_hWndParent, nChildID), true, true, bSizeXFixed, bSizeYFixed, nullptr);
}

void CWndHelper::onResize()
{
    uint32_t        i;

    for (i = 0; i < m_vChildWnd.size(); i++)
    {
        ChildWndProp        &childWnd = m_vChildWnd[i];

        CRect    rc, rcRalation;

        getWindowRect(childWnd.hWnd, &rc);

        if (childWnd.hWndRelation)
        {
            getWindowRect(childWnd.hWndRelation, &rcRalation);
        }
        else
        {
            getWindowRect(m_hWndParent, &rcRalation);
            rcRalation.left = rcRalation.right;
            rcRalation.top = rcRalation.bottom;
        }

        if (childWnd.bAlignRight)
        {
            if (!childWnd.bSizeXFix)
                rc.right = rcRalation.right - childWnd.nMarginX;
            else
            {
                int        nCx;
                nCx = rc.right - rc.left;
                rc.right = rcRalation.right - childWnd.nMarginX;
                rc.left = rc.right - nCx;
            }
        }

        if (childWnd.bAlignBottom)
        {
            if (!childWnd.bSizeYFix)
                rc.bottom = rcRalation.bottom - childWnd.nMarginY;
            else
            {
                int        nCy;
                nCy = rc.bottom - rc.top;
                rc.bottom = rcRalation.bottom - childWnd.nMarginY;
                rc.top = rc.bottom - nCy;
            }
        }
        
        screenToClient(m_hWndParent, (LPPOINT)&(rc.left));
        screenToClient(m_hWndParent, (LPPOINT)&(rc.right));
        moveWindow(childWnd.hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
    }

    for (i = 0; i < (int)m_vAlignCenterWnds.size(); i++)
    {
        CRect                    rc, rcParent;
        ChildAlignCenter        &align = m_vAlignCenterWnds[i];
        getWindowRect(m_hWndParent, &rcParent);
        getWindowRect(align.hWnd, &rc);
        screenToClient(m_hWndParent, (LPPOINT)&rc.left);
        screenToClient(m_hWndParent, (LPPOINT)&rc.right);
        if (align.bVert)
        {
            int            h = rc.bottom - rc.top;
            rc.top = (rcParent.right - rcParent.left) / 2 - align.nOffsetToCenter;
            rc.bottom = rc.top + h;
        }
        else
        {
            int            w = rc.right - rc.left;
            rc.left = (rcParent.right - rcParent.left) / 2 - align.nOffsetToCenter;
            rc.right = rc.left + w;
        }
        moveWindow(align.hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
    }
}
