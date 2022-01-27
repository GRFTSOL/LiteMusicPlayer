/********************************************************************
    Created :    2001/12/17    0:49
    FileName:    wnddrag.cpp
    Author  :    xhy
    Purpose :    
*********************************************************************/

#include "WindowTypes.h"
#include "Window.h"
#include "WndDrag.h"
#include "Desktop.h"
#include <vector>

int horzLineCloseToLine(int lsx1, int lsx2, int lsy, int ldx1, int ldx2, int ldy, int nCloseExt)
{
    int nOffy = 0;
    if (abs(lsy - ldy) < nCloseExt)
    {
        int lx, rx, cx;

        if (lsx1 < ldx1)
            lx = lsx1;
        else
            lx = ldx1;

        if (lsx2 > ldx2)
            rx = lsx2;
        else
            rx = ldx2;
        cx = lsx2 - lsx1 + ldx2 - ldx1;

        if (rx - lx <= cx + nCloseExt)
            nOffy = ldy - lsy;
    }
    return nOffy;
}

void vertLineCloseToRect(int srcY1, int srcY2, int srcX, CRect &rcd, int &offx, int nCloseExt)
{
    offx = horzLineCloseToLine(srcY1, srcY2, srcX,
        rcd.top, rcd.bottom, rcd.left, nCloseExt);
    if (offx == 0)
    {
        offx = horzLineCloseToLine(srcY1, srcY2, srcX,
            rcd.top, rcd.bottom, rcd.right, nCloseExt);
    }
}

void horzLineCloseToRect(int srcX1, int srcX2, int srcY, CRect &rcd, int &offy, int nCloseExt)
{
    offy = horzLineCloseToLine(srcX1, srcX2, srcY,
        rcd.left, rcd.right, rcd.top, nCloseExt);
    if (offy == 0)
    {
        offy = horzLineCloseToLine(srcX1, srcX2, srcY,
            rcd.left, rcd.right, rcd.bottom, nCloseExt);
    }
}

void rectCloseToRect(CRect &rcs, CRect &rcd, int &offx, int &offy, int nCloseExt)
{
    offx = 0; offy = 0;
    offy = horzLineCloseToLine(rcs.left, rcs.right, rcs.top,
        rcd.left, rcd.right, rcd.top, nCloseExt);
    if (offy == 0)
    {
        offy = horzLineCloseToLine(rcs.left, rcs.right, rcs.top,
            rcd.left, rcd.right, rcd.bottom, nCloseExt);
        if (offy == 0)
        {
            offy = horzLineCloseToLine(rcs.left, rcs.right, rcs.bottom,
                rcd.left, rcd.right, rcd.bottom, nCloseExt);
            if (offy == 0)
                offy = horzLineCloseToLine(rcs.left, rcs.right, rcs.bottom,
                    rcd.left, rcd.right, rcd.top, nCloseExt);
        }
    }

    offx = horzLineCloseToLine(rcs.top, rcs.bottom, rcs.left,
        rcd.top, rcd.bottom, rcd.left, nCloseExt);
    if (offx == 0)
    {
        offx = horzLineCloseToLine(rcs.top, rcs.bottom, rcs.left,
            rcd.top, rcd.bottom, rcd.right, nCloseExt);
        if (offx == 0)
        {
            offx = horzLineCloseToLine(rcs.top, rcs.bottom, rcs.right,
                rcd.top, rcd.bottom, rcd.right, nCloseExt);
            if (offx == 0)
                offx = horzLineCloseToLine(rcs.top, rcs.bottom, rcs.right,
                    rcd.top, rcd.bottom, rcd.left, nCloseExt);
        }
    }
}

void rectCloseToWindow(CRect &rcs, Window *pWnd, int nCloseExt, int &offx, int &offy)
{
    CRect rcd;
    offx = 0;
    offy = 0;
    if (pWnd)
    {
        if (pWnd->isVisible())
        {
            pWnd->getWindowRect(&rcd);
            rectCloseToRect(rcs, rcd, offx, offy, nCloseExt);
        }
    }
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WndDrag::WndDrag()
{
    m_bEnableDrag = true;

    m_bDragWnd = false;

    m_bDragAutoCloseto = true;

    m_pWnd = nullptr;

    // m_arrszWndClassCloseTo = nullptr;
    // m_pvWndClassCloseTo = nullptr;

    m_bSticked = false;
    m_pWndToTrack = nullptr;

    m_ptWndTrack.x = 0;
    m_ptWndTrack.y = 0;
}

WndDrag::~WndDrag()
{

}

void WndDrag::onDrag(uint32_t fwKeys, CPoint pt)
{
    if (!m_bEnableDrag)
        return;

    if (fwKeys & MK_LBUTTON)
    {
        pt = getCursorPos();
        // DBG_LOG2("Drag, cursor x: %d, y: %d", pt.x, pt.y);
        if (m_bDragWnd)
        {
            // dragging
            // move window
            if ( (pt.x - m_ptDragOld.x != 0) || (pt.y - m_ptDragOld.y != 0))
            {
                if (m_bDragAutoCloseto)
                {
                    int nOffx, nOffy;    

                    nOffx = pt.x - m_ptDragOld.x;
                    nOffy = pt.y - m_ptDragOld.y;
                    autoCloseToWindows(nOffx, nOffy);
                    m_ptDragOld.y = pt.y + nOffy;
                    m_ptDragOld.x = pt.x + nOffx;
                }
                else
                {
                    CRect        rcWnd;
                    m_pWnd->getWindowRect(&rcWnd);
                    setWindowPosSafely(rcWnd.left, rcWnd.top, pt.x - m_ptDragOld.x, pt.y - m_ptDragOld.y);

                    m_ptDragOld.y = pt.y;
                    m_ptDragOld.x = pt.x;
                }
            }
        }
        else
        {
            if (m_pWnd->isZoomed())
                return;

            // begin drag
            m_bDragWnd = true;
            m_ptDragOld.x = pt.x;
            m_ptDragOld.y = pt.y;
            m_pWnd->setCapture();
           }
    }
    else if (m_bDragWnd)
    {
        // drag end
        m_bDragWnd = false;
        m_pWnd->releaseCapture();
    }
}

void WndDrag::setWindowPosSafely(int xOld, int yOld, int nOffsetx, int nOffsety)
{
    m_pWnd->setWindowPosSafely(xOld + nOffsetx, yOld + nOffsety);
}

bool WndDrag::autoCloseToWindows(int &nOffx, int &nOffy, bool bMoveWindow /* = true */)
{
    CRect        rcw, rcd;
    Window    *pWnd;

    m_pWnd->getWindowRect(&rcw);

    rcw.offsetRect(nOffx, nOffy);
    nOffx = 0;
    nOffy = 0;

    // move Window close to winamp window and desktop and tray_window
    for (uint32_t i = 0; i < m_vWndCloseTo.size(); i ++)
    {
        if (nOffx == 0 || nOffy == 0)
        {
            int nOffxt, nOffyt;
            // get Window close to rect
            if (m_vWndCloseTo[i].pWnd)
                pWnd = m_vWndCloseTo[i].pWnd;
            else
            {
                const char    *szClass = m_vWndCloseTo[i].strClass.c_str();
                if (isEmptyString(szClass))
                    szClass = nullptr;
                const char    *szWnd = m_vWndCloseTo[i].strWndName.c_str();
                if (isEmptyString(szWnd))
                    szWnd = nullptr;
                pWnd = findWindow(szClass, szWnd);
            }
            if (pWnd && !pWnd->isSameWnd(m_pWnd))
            {
                rectCloseToWindow(rcw, pWnd, 10, nOffxt, nOffyt);
                if (nOffx == 0)
                    nOffx = nOffxt;
                if (nOffy == 0)
                    nOffy = nOffyt;
            }
        }
        else
            break;
    }

    if (nOffx == 0 || nOffy == 0)
    {
        int nOffxt, nOffyt;
        CRect rcd;
        if (getMonitorRestrictRect(rcw, rcd))
        {
            // close to monitor
            rectCloseToRect(rcw, rcd, nOffxt, nOffyt, 10);
            if (nOffx == 0)
                nOffx = nOffxt;
            if (nOffy == 0)
                nOffy = nOffyt;
        }
    }
    // ok 
    rcw.offsetRect(nOffx, nOffy);

    CRect       rc;
    m_pWnd->getWindowRect(&rc);
    if (rc.equal(rcw))
    {
        // no move
    }
    else
    {
        // move to new rect
        if (bMoveWindow)
        {
            setWindowPosSafely(rc.left, rc.top, nOffx, nOffy);
        }

        return true;
    }

    return false;
}

void WndDrag::init(Window *pWnd)
{
    m_pWnd = pWnd;

    m_pWndToTrack = nullptr;
}

void WndDrag::addWndCloseto(Window *pWnd, cstr_t szWndClass, cstr_t szWndName)
{
    WndCloseTo    CloseTo;

    if (pWnd)
    {
        CloseTo.pWnd = pWnd;
        CloseTo.strClass = "";
        CloseTo.strWndName = "";
        for (uint32_t i = 0; i < m_vWndCloseTo.size(); i++)
        {
            if (m_vWndCloseTo[i].pWnd == pWnd)
            {
                return;
            }
        }
    }
    else
    {
        CloseTo.pWnd = nullptr;
        if (szWndClass)
            CloseTo.strClass = szWndClass;
        else
            CloseTo.strClass = "";
        if (szWndName)
            CloseTo.strWndName = szWndName;
        else
            CloseTo.strWndName = "";
        for (uint32_t i = 0; i < m_vWndCloseTo.size(); i++)
        {
            if (strcasecmp(m_vWndCloseTo[i].strWndName.c_str(), CloseTo.strWndName.c_str()) == 0 
                && strcasecmp(m_vWndCloseTo[i].strClass.c_str(), CloseTo.strClass.c_str()) == 0)
            {
                return;
            }
        }
    }

    m_vWndCloseTo.push_back(CloseTo);
}

bool isHorzStickLines(int yA, int xA1, int xA2, int yB, int xB1, int xB2)
{
    return (yA == yB) && ((xA1 <= xB1 && xB1 <= xA2) || (xA1 <= xB2 && xB2 <= xA2) || (xB1 <= xA1 && xB2 >= xA2));
}

bool isVertStickLines(int xA, int yA1, int yA2, int xB, int yB1, int yB2)
{
    return (xA == xB) && ((yA1 <= yB1 && yB1 <= yA2) || (yA1 <= yB2 && yB2 <= yA2) || (yB1 <= yA1 && yB2 >= yA2));
}

bool isStickedWindows(Window *pWnd1, Window *pWnd2)
{
    CRect rcw, rcd;

    pWnd1->getWindowRect(&rcw);

    pWnd2->getWindowRect(&rcd);

    return (isHorzStickLines(rcd.top, rcd.left, rcd.right, rcw.top, rcw.left, rcw.right) ||
        isHorzStickLines(rcd.top, rcd.left, rcd.right, rcw.bottom, rcw.left, rcw.right) ||
        isHorzStickLines(rcd.bottom, rcd.left, rcd.right, rcw.top, rcw.left, rcw.right) ||
        isHorzStickLines(rcd.bottom, rcd.left, rcd.right, rcw.bottom, rcw.left, rcw.right) ||
        isVertStickLines(rcd.left, rcd.top, rcd.bottom, rcw.left, rcw.top, rcw.bottom) || 
        isVertStickLines(rcd.left, rcd.top, rcd.bottom, rcw.right, rcw.top, rcw.bottom) ||
        isVertStickLines(rcd.right, rcd.top, rcd.bottom, rcw.left, rcw.top, rcw.bottom) || 
        isVertStickLines(rcd.right, rcd.top, rcd.bottom, rcw.right, rcw.top, rcw.bottom));
}

void WndDrag::trackMoveWith(Window *pWnd, int x, int y)
{
    if (m_bEnableDrag && m_bSticked && m_pWndToTrack && !m_pWndToTrack->isIconic())
    {
        CRect    rcw;

        m_pWnd->getWindowRect(&rcw);

        setWindowPosSafely(rcw.left, rcw.top, x - m_ptWndTrack.x, y - m_ptWndTrack.y);

        rcw.offsetRect(x - m_ptWndTrack.x, y - m_ptWndTrack.y);

        m_pWndToTrack->getWindowRect(&rcw);

        m_ptWndTrack.x = rcw.left;
        m_ptWndTrack.y = rcw.top;
    }
}

//
// 此函数在目标窗口移动之前进行调用，可以跟随目标窗口移动
//
// hWndChain 是中间的传递窗口;
// nCount 是其中包含的个数
// hWndToTrack 是将要跟随移动的窗口
//
// 如果检查的结果是本窗口靠近hWndToCheck中的任何一个，则
void WndDrag::beforeTrackMoveWith(Window *pWndChain[], int nCount, Window *pWndToTrack)
{
    m_bSticked = false;
    std::vector<Window *>    vIn, vOut;

    for (int i = 0; i < nCount; i++)
    {
        if (pWndChain[i] && pWndChain[i]->isVisible())
            vOut.push_back(pWndChain[i]);
    }

    vIn.push_back(pWndToTrack);

    while (1)
    {
        int        i;
        for (i = 0; i < (int)vIn.size(); i++)
        {
            if (isStickedWindows(m_pWnd, vIn[i]))
            {
                m_bSticked = true;
                goto STICKED_FOUND;
            }
        }
        bool    bFound = false;
        for (i = 0; i < (int)vIn.size(); i++)
        {
            for (int k = 0; k < (int)vOut.size(); k++)
            {
                if (isStickedWindows(vOut[k], vIn[i]))
                {
                    vIn.push_back(vOut[k]);
                    vOut.erase(vOut.begin() + k);
                    bFound = true;
                    goto FIND_CHAIN_OUT;
                }
            }
        }
FIND_CHAIN_OUT:
        if (!bFound)
            break;
    }
    return;

STICKED_FOUND:
    CRect        rc;

    m_pWndToTrack = pWndToTrack;

    m_pWndToTrack->getWindowRect(&rc);

    m_ptWndTrack.x = rc.left;
    m_ptWndTrack.y = rc.top;

    m_bSticked = true;
}
