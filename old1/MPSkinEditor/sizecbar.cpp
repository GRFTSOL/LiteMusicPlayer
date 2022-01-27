/////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998, 1999 by Cristi Posea
// All rights reserved
//
// Use and distribute freely, except: don't remove my name from the
// source or documentation (don't take credit for my work), mark your
// changes (don't get me blamed for your possible bugs), don't alter
// or remove this notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// This class is intended to be used as a base class. Do not simply add
// your code to this file - instead create a new class derived from
// CSizingControlBar and put there what you need.
// Modify this file only to fix bugs, and don't forget to send me a copy.
//
// send bug reports, bug fixes, enhancements, requests, flames, etc.,
// and I'll try to keep a version up to date.  I can be reached at:
//    cristip@dundas.com
//
// More details at MFC Programmer's SourceBook
// http://www.codeguru.com/docking/docking_window.shtml or search
// www.codeguru.com for my name if the article was moved.
//
/////////////////////////////////////////////////////////////////////////
//
// Acknowledgements:
//  o   Thanks to Harlan R. Seymour (harlans@dundas.com) for his continuous
//      support during development of this code.
//  o   Thanks to Dundas Software for the opportunity to test this code
//      on real-life applications.
//      If you don't know who they are, visit them at www.dundas.com .
//      Their award winning components and development suites are
//      a pile of gold.
//  o   Thanks to Chris Maunder (chrism@dundas.com) who came with the
//      simplest way to query "show window content while dragging" system
//      setting.
//  o   Thanks to Zafir Anjum (zafir@codeguru.com) for publishing this
//      code on his cool site (www.codeguru.com).
//  o   Some ideas for the gripper came from the CToolBarEx flat toolbar
//      by Joerg Koenig (Joerg.Koenig@rhein-neckar.de). Also he inspired
//      me on writing this notice:) . Thanks, Joerg!
//  o   Thanks to Jakawan Ratiwanich (jack@alpha.fsec.ucf.edu) and to
//      Udo Schaefer (Udo.Schaefer@vcase.de) for the dwStyle bug fix under
//      VC++ 6.0.
//  o   And, of course, many thanks to all of you who used this code,
//      for the invaluable feedback I received.
//      
/////////////////////////////////////////////////////////////////////////


// sizecbar.cpp : implementation file
//

#include "sizecbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////
// CSizingControlBar

CSCBArray CSizingControlBar::m_arrBars; // static member

IMPLEMENT_DYNAMIC(CSizingControlBar, baseCSizingControlBar);

CSizingControlBar::CSizingControlBar()
{
    m_szMin = cSize(33, 32);
    m_szHorz = cSize(200, 200);
    m_szVert = cSize(200, 200);
    m_szFloat = cSize(200, 200);
    m_bTracking = false;
    m_bKeepSize = false;
    m_bParentSizing = false;
    m_cxEdge = 5;
    m_bDragShowContent = false;
    m_nDockBarID = 0;
    m_dwSCBStyle = 0;
}

CSizingControlBar::~CSizingControlBar()
{
}

BEGIN_MESSAGE_MAP(CSizingControlBar, baseCSizingControlBar)
    //{{AFX_MSG_MAP(CSizingControlBar)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_NCPAINT()
    ON_WM_NCCALCSIZE()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_CAPTURECHANGED()
    ON_WM_SETTINGCHANGE()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_NCLBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_RBUTTONDOWN()
    ON_WM_NCLBUTTONUP()
    ON_WM_NCMOUSEMOVE()
    ON_WM_NCHITTEST()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

bool CSizingControlBar::create(cstr_t lpszWindowName, CWnd* pParentWnd,
                               cSize sizeDefault, bool bHasGripper,
                               uint32_t nID, uint32_t dwStyle)
{
    // must have a parent
    ASSERT_VALID(pParentWnd);
    // cannot be both fixed and dynamic
    // (CBRS_SIZE_DYNAMIC is used for resizng when floating)
    assert (!((dwStyle & CBRS_SIZE_FIXED) &&
              (dwStyle & CBRS_SIZE_DYNAMIC)));

    m_dwStyle = dwStyle & CBRS_ALL; // save the control bar styles

    m_szHorz = sizeDefault; // set the size members
    m_szVert = sizeDefault;
    m_szFloat = sizeDefault;

    m_cyGripper = bHasGripper ? 12 : 0; // set the gripper width

    // register and create the window - skip CControlBar::create()
    CString wndclass = ::AfxRegisterWndClass(CS_DBLCLKS,
        ::loadCursor(nullptr, IDC_ARROW),
        ::GetSysColorBrush(COLOR_BTNFACE), 0);

    dwStyle &= ~CBRS_ALL; // keep only the generic window styles
    dwStyle |= WS_CLIPCHILDREN; // prevents flashing
    if (!CWnd::create(wndclass, lpszWindowName, dwStyle,
        CRect(0, 0, 0, 0), pParentWnd, nID))
        return false;

    return true;
}

/////////////////////////////////////////////////////////////////////////
// CSizingControlBar message handlers

int CSizingControlBar::onCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (baseCSizingControlBar::onCreate(lpCreateStruct) == -1)
        return -1;
    
    // querry SPI_GETDRAGFULLWINDOWS system parameter
    // OnSettingChange() will update m_bDragShowContent
    m_bDragShowContent = false;
    ::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0,
        &m_bDragShowContent, 0);

    m_arrBars.add(this);        // register
    
//    m_dwSCBStyle |= SCBS_SHOWEDGES;

    return 0;
}

bool CSizingControlBar::destroyWindow() 
{
    int nPos = findSizingBar(this);
    assert(nPos >= 0);

    m_arrBars.removeAt(nPos);   // unregister

    return baseCSizingControlBar::destroyWindow();
}

const bool CSizingControlBar::isFloating() const
{
    return !isHorzDocked() && !isVertDocked();
}

const bool CSizingControlBar::isHorzDocked() const
{
    return (m_nDockBarID == AFX_IDW_DOCKBAR_TOP ||
        m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);
}

const bool CSizingControlBar::isVertDocked() const
{
    return (m_nDockBarID == AFX_IDW_DOCKBAR_LEFT ||
        m_nDockBarID == AFX_IDW_DOCKBAR_RIGHT);
}

const bool CSizingControlBar::isSideTracking() const
{
    // don't call this when not tracking
    assert(m_bTracking && !isFloating());

    return (m_htEdge == HTLEFT || m_htEdge == HTRIGHT) ?
        isHorzDocked() : isVertDocked();
}

cSize CSizingControlBar::calcFixedLayout(bool bStretch, bool bHorz)
{
    if (bStretch) // the bar is stretched (is not the child of a dockbar)
        if (bHorz)
            return cSize(32767, m_szHorz.cy);
        else
            return cSize(m_szVert.cx, 32767);

    // dirty cast - using CSCBDockBar to access protected CDockBar members
    CSCBDockBar* pDockBar = (CSCBDockBar*) m_pDockBar;

    // force imediate RecalcDelayShow() for all sizing bars on the row
    // with delayShow/delayHide flags set to avoid isVisible() problems
    CSCBArray arrSCBars;
    getRowSizingBars(arrSCBars);
    AFX_SIZEPARENTPARAMS layout;
    layout.hDWP = pDockBar->m_bLayoutQuery ?
        nullptr : ::BeginDeferWindowPos(arrSCBars.getSize());
    for (int i = 0; i < arrSCBars.getSize(); i++)
        arrSCBars[i]->RecalcDelayShow(&layout);
    if (layout.hDWP != nullptr)
        ::EndDeferWindowPos(layout.hDWP);

    // get available length
    CRect rc = pDockBar->m_rectLayout;
    if (rc.empty())
        m_pDockSite->getClientRect(&rc);
    int nLengthAvail = bHorz ? rc.width() + 2 : rc.height() - 2;

    if (isVisible() && !isFloating() &&
        m_bParentSizing && arrSCBars[0] == this)
        if (negociateSpace(nLengthAvail, (bHorz != false)))
            alignControlBars();

    m_bParentSizing = false;
    
    cSize szRet = bHorz ? m_szHorz : m_szVert;
    szRet.cx = max(m_szMin.cx, szRet.cx);
    szRet.cy = max(m_szMin.cy, szRet.cy);

    return szRet;
}

cSize CSizingControlBar::calcDynamicLayout(int nLength, uint32_t dwMode)
{
    if (dwMode & (LM_HORZDOCK | LM_VERTDOCK)) // docked ?
    {
        if (nLength == -1)
            m_bParentSizing = true;

        return baseCSizingControlBar::calcDynamicLayout(nLength, dwMode);
    }

    if (dwMode & LM_MRUWIDTH) return m_szFloat;
    if (dwMode & LM_COMMIT) return m_szFloat; // already committed

    ((dwMode & LM_LENGTHY) ? m_szFloat.cy : m_szFloat.cx) = nLength;

    m_szFloat.cx = max(m_szFloat.cx, m_szMin.cx);
    m_szFloat.cy = max(m_szFloat.cy, m_szMin.cy);

    return m_szFloat;
}

void CSizingControlBar::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
    // force non-client recalc if moved or resized
    lpwndpos->flags |= SWP_FRAMECHANGED;

    baseCSizingControlBar::OnWindowPosChanging(lpwndpos);

    // find on which side are we docked
    uint32_t nOldDockBarID = m_nDockBarID;
    m_nDockBarID = getParent()->GetDlgCtrlID();

    if (!isFloating())
        if (lpwndpos->flags & SWP_SHOWWINDOW)
            m_bKeepSize = true;
}

/////////////////////////////////////////////////////////////////////////
// Mouse Handling
//
void CSizingControlBar::onLButtonDown(uint32_t nFlags, CPoint point) 
{
    if (m_pDockBar != nullptr)
    {
        // start the drag
        assert(m_pDockContext != nullptr);
        clientToScreen(&point);
        m_pDockContext->StartDrag(point);
    }
    else
        CWnd::onLButtonDown(nFlags, point);
}

void CSizingControlBar::onLButtonDblClk(uint32_t nFlags, CPoint point) 
{
    if (m_pDockBar != nullptr)
    {
        // toggle docking
        assert(m_pDockContext != nullptr);
        m_pDockContext->ToggleDocking();
    }
    else
        CWnd::onLButtonDblClk(nFlags, point);
}

void CSizingControlBar::OnNcLButtonDown(uint32_t nHitTest, CPoint point) 
{
    if (isFloating())
    {
        baseCSizingControlBar::OnNcLButtonDown(nHitTest, point);
        return;
    }

    if (m_bTracking) return;

    if ((nHitTest >= HTSIZEFIRST) && (nHitTest <= HTSIZELAST))
        startTracking(nHitTest); // sizing edge hit
}

void CSizingControlBar::OnNcLButtonUp(uint32_t nHitTest, CPoint point) 
{
    if (nHitTest == HTCLOSE)
        m_pDockSite->ShowControlBar(this, false, false); // hide

    baseCSizingControlBar::OnNcLButtonUp(nHitTest, point);
}

void CSizingControlBar::onLButtonUp(uint32_t nFlags, CPoint point) 
{
    if (m_bTracking)
        stopTracking();

    baseCSizingControlBar::onLButtonUp(nFlags, point);
}

void CSizingControlBar::onRButtonDown(uint32_t nFlags, CPoint point) 
{
    if (m_bTracking)
        stopTracking();
    
    baseCSizingControlBar::onRButtonDown(nFlags, point);
}

void CSizingControlBar::onMouseMove(uint32_t nFlags, CPoint point) 
{
    if (m_bTracking)
        onTrackUpdateSize(point);
    
    baseCSizingControlBar::onMouseMove(nFlags, point);
}

void CSizingControlBar::OnCaptureChanged(CWnd *pWnd) 
{
    if (m_bTracking && (pWnd != this))
        stopTracking();

    baseCSizingControlBar::OnCaptureChanged(pWnd);
}

void CSizingControlBar::OnNcCalcSize(bool bCalcValidRects,
                                     NCCALCSIZE_PARAMS FAR* lpncsp) 
{
    // compute the the client area
    CRect rcClient = lpncsp->rgrc[0];
    rcClient.deflate(5, 5);

    m_dwSCBStyle &= ~SCBS_EDGEALL;

    switch (m_nDockBarID)
    {
    case AFX_IDW_DOCKBAR_TOP:
        m_dwSCBStyle |= SCBS_EDGEBOTTOM;
        rcClient.deflate(m_cyGripper, 0, 0, 0);
        break;
    case AFX_IDW_DOCKBAR_BOTTOM:
        m_dwSCBStyle |= SCBS_EDGETOP;
        rcClient.deflate(m_cyGripper, 0, 0, 0);
        break;
    case AFX_IDW_DOCKBAR_LEFT:
        m_dwSCBStyle |= SCBS_EDGERIGHT;
        rcClient.deflate(0, m_cyGripper, 0, 0);
        break;
    case AFX_IDW_DOCKBAR_RIGHT:
        m_dwSCBStyle |= SCBS_EDGELEFT;
        rcClient.deflate(0, m_cyGripper, 0, 0);
        break;
    default:
        break;
    }

    if (!isFloating() && m_pDockBar != nullptr)
    {
        CSCBArray arrSCBars;
        getRowSizingBars(arrSCBars);

        for (int i = 0; i < arrSCBars.getSize(); i++)
            if (arrSCBars[i] == this)
            {
                if (i > 0)
                    m_dwSCBStyle |= isHorzDocked() ?
                        SCBS_EDGELEFT : SCBS_EDGETOP;
                if (i < arrSCBars.getSize() - 1)
                    m_dwSCBStyle |= isHorzDocked() ?
                        SCBS_EDGERIGHT : SCBS_EDGEBOTTOM;
            }
    }

    // make room for edges only if they will be painted
    if (m_dwSCBStyle & SCBS_SHOWEDGES)
        rcClient.deflate(
            (m_dwSCBStyle & SCBS_EDGELEFT) ? m_cxEdge : 0,
            (m_dwSCBStyle & SCBS_EDGETOP) ? m_cxEdge : 0,
            (m_dwSCBStyle & SCBS_EDGERIGHT) ? m_cxEdge : 0,
            (m_dwSCBStyle & SCBS_EDGEBOTTOM) ? m_cxEdge : 0);

    // "hide" button positioning
    CPoint ptOrgBtn;
    if (isHorzDocked())
        ptOrgBtn = CPoint(rcClient.left - m_cyGripper - 1,
            rcClient.top - 1);
    else
        ptOrgBtn = CPoint(rcClient.right - 11,
            rcClient.top - m_cyGripper - 1);

    m_biHide.move(ptOrgBtn - CRect(lpncsp->rgrc[0]).TopLeft());

    lpncsp->rgrc[0] = rcClient;
}

void CSizingControlBar::OnNcPaint() 
{
    // get window DC that is clipped to the non-client area
    CWindowDC dc(this);

    CRect rcClient, rcBar;
    getClientRect(rcClient);
    clientToScreen(rcClient);
    getWindowRect(rcBar);
    rcClient.offsetRect(-rcBar.TopLeft());
    rcBar.offsetRect(-rcBar.TopLeft());

    // client area is not our bussiness :)
    dc.ExcludeClipRect(rcClient);

    // draw borders in non-client area
    CRect rcDraw = rcBar;
    DrawBorders(&dc, rcDraw);

    // erase parts not drawn
    dc.IntersectClipRect(rcDraw);

    // erase NC background the hard way
    HBRUSH hbr = (HBRUSH)GetClassLong(m_hWnd, GCL_HBRBACKGROUND);
    ::fillRect(dc.m_hDC, rcDraw, hbr);

    if (m_dwSCBStyle & SCBS_SHOWEDGES)
    {
        CRect rcEdge; // paint the sizing edges
        for (int i = 0; i < 4; i++)
            if (getEdgeRect(rcBar, getEdgeHTCode(i), rcEdge))
                dc.Draw3dRect(rcEdge, ::GetSysColor(COLOR_BTNHIGHLIGHT),
                    ::GetSysColor(COLOR_BTNSHADOW));
    }

    if (m_cyGripper && !isFloating())
        ncPaintGripper(&dc, rcClient);

    ReleaseDC(&dc);
}

void CSizingControlBar::ncPaintGripper(CDC* pDC, CRect rcClient)
{
    // paints a simple "two raised lines" gripper
    // override this if you want a more sophisticated gripper
    CRect gripper = rcClient;
    CRect rcbtn = m_biHide.getRect();
    bool bHorz = isHorzDocked();
    
    gripper.deflate(1, 1);
    if (bHorz)
    {   // gripper at left
        gripper.left -= m_cyGripper;
        gripper.right = gripper.left + 3;
        gripper.top = rcbtn.bottom + 3;
    }
    else
    {   // gripper at top
        gripper.top -= m_cyGripper;
        gripper.bottom = gripper.top + 3;
        gripper.right = rcbtn.left - 3;
    }

    pDC->Draw3dRect(gripper, ::GetSysColor(COLOR_BTNHIGHLIGHT),
        ::GetSysColor(COLOR_BTNSHADOW));

    gripper.offsetRect(bHorz ? 3 : 0, bHorz ? 0 : 3);
    
    pDC->Draw3dRect(gripper, ::GetSysColor(COLOR_BTNHIGHLIGHT),
        ::GetSysColor(COLOR_BTNSHADOW));

    m_biHide.paint(pDC);
}

void CSizingControlBar::onPaint()
{
    // overridden to skip border painting based on clientrect
    CPaintDC dc(this);
}

LRESULT CSizingControlBar::OnNcHitTest(CPoint point)
{
    if (isFloating())
        return baseCSizingControlBar::OnNcHitTest(point);

    CRect rcBar, rcEdge;
    getWindowRect(rcBar);

    for (int i = 0; i < 4; i++)
        if (getEdgeRect(rcBar, getEdgeHTCode(i), rcEdge))
            if (rcEdge.ptInRect(point)) return getEdgeHTCode(i);

    CRect rc = m_biHide.getRect();
    rc.offsetRect(rcBar.TopLeft());
    if (rc.ptInRect(point))
        return HTCLOSE;

    return HTCLIENT;
}

void CSizingControlBar::OnSettingChange(uint32_t uFlags, cstr_t lpszSection) 
{
    baseCSizingControlBar::OnSettingChange(uFlags, lpszSection);

    m_bDragShowContent = false;
    ::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0,
        &m_bDragShowContent, 0); // update
}

/////////////////////////////////////////////////////////////////////////
// CSizingControlBar implementation helpers

void CSizingControlBar::startTracking(uint32_t nHitTest)
{
    setCapture();

    // make sure no updates are pending
    RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_UPDATENOW);
    
    bool bHorz = isHorzDocked();

    m_szOld = bHorz ? m_szHorz : m_szVert;

    CRect rc;
    getWindowRect(&rc);
    CRect rcEdge;
    VERIFY(getEdgeRect(rc, nHitTest, rcEdge));
    m_ptOld = rcEdge.centerPoint();

    m_htEdge = nHitTest;
    m_bTracking = true;

    CSCBArray arrSCBars;
    int            i;
    getRowSizingBars(arrSCBars);

    // compute the minsize as the max minsize of the sizing bars on row
    m_szMinT = m_szMin;
    for (i = 0; i < arrSCBars.getSize(); i++)
        if (bHorz)
            m_szMinT.cy = max(m_szMinT.cy, arrSCBars[i]->m_szMin.cy);
        else
            m_szMinT.cx = max(m_szMinT.cx, arrSCBars[i]->m_szMin.cx);

    if (!isSideTracking())
    {
        // the control bar cannot grow with more than the size of 
        // remaining client area of the mainframe
        m_pDockSite->RepositionBars(0, 0xFFFF, AFX_IDW_PANE_FIRST,
            reposQuery, &rc, nullptr, true);
        m_szMaxT = m_szOld + rc.size() - cSize(4, 4);
    }
    else
    {
        // side tracking: max size is the actual size plus the amount
        // the neighbour bar can be decreased to reach its minsize
        for (i = 0; i < arrSCBars.getSize(); i++)
            if (arrSCBars[i] == this) break;

        CSizingControlBar* pBar = arrSCBars[i +
            ((m_htEdge == HTTOP || m_htEdge == HTLEFT) ? -1 : 1)];

        m_szMaxT = m_szOld + (bHorz ? pBar->m_szHorz :
            pBar->m_szVert) - pBar->m_szMin;
    }

    onTrackInvertTracker(); // draw tracker
}

void CSizingControlBar::stopTracking()
{
    onTrackInvertTracker(); // erase tracker

    m_bTracking = false;
    releaseCapture();
    
    m_pDockSite->DelayRecalcLayout();
}

void CSizingControlBar::onTrackUpdateSize(CPoint& point)
{
    assert(!isFloating());

    CPoint pt = point;
    clientToScreen(&pt);
    cSize szDelta = pt - m_ptOld;

    cSize sizeNew = m_szOld;
    switch (m_htEdge)
    {
    case HTLEFT:    sizeNew -= cSize(szDelta.cx, 0); break;
    case HTTOP:     sizeNew -= cSize(0, szDelta.cy); break;
    case HTRIGHT:   sizeNew += cSize(szDelta.cx, 0); break;
    case HTBOTTOM:  sizeNew += cSize(0, szDelta.cy); break;
    }

    // enforce the limits
    sizeNew.cx = max(m_szMinT.cx, min(m_szMaxT.cx, sizeNew.cx));
    sizeNew.cy = max(m_szMinT.cy, min(m_szMaxT.cy, sizeNew.cy));

    bool bHorz = isHorzDocked();
    szDelta = sizeNew - (bHorz ? m_szHorz : m_szVert);
    
    if (szDelta == cSize(0, 0)) return; // no size change

    onTrackInvertTracker(); // erase tracker

    (bHorz ? m_szHorz : m_szVert) = sizeNew; // save the new size

    CSCBArray arrSCBars;
    getRowSizingBars(arrSCBars);

    for (int i = 0; i < arrSCBars.getSize(); i++)
        if (!isSideTracking())
        {   // track simultaneously
            CSizingControlBar* pBar = arrSCBars[i];
            (bHorz ? pBar->m_szHorz.cy : pBar->m_szVert.cx) =
                bHorz ? sizeNew.cy : sizeNew.cx;
        }
        else
        {   // adjust the neighbour's size too
            if (arrSCBars[i] != this) continue;

            CSizingControlBar* pBar = arrSCBars[i +
                ((m_htEdge == HTTOP || m_htEdge == HTLEFT) ? -1 : 1)];

            (bHorz ? pBar->m_szHorz.cx : pBar->m_szVert.cy) -=
                bHorz ? szDelta.cx : szDelta.cy;
        }

    onTrackInvertTracker(); // redraw tracker at new pos

    if (m_bDragShowContent)
        m_pDockSite->DelayRecalcLayout();
}

void CSizingControlBar::onTrackInvertTracker()
{
    assert(m_bTracking);

    if (m_bDragShowContent)
        return; // don't show tracker if DragFullWindows is on

    bool bHorz = isHorzDocked();
    CRect rc, rcBar, rcDock, rcFrame;
    getWindowRect(rcBar);
    m_pDockBar->getWindowRect(rcDock);
    m_pDockSite->getWindowRect(rcFrame);
    VERIFY(getEdgeRect(rcBar, m_htEdge, rc));
    if (!isSideTracking())
        rc = bHorz ? 
            CRect(rcDock.left + 1, rc.top, rcDock.right - 1, rc.bottom) :
            CRect(rc.left, rcDock.top + 1, rc.right, rcDock.bottom - 1);

    rc.offsetRect(-rcFrame.TopLeft());

    cSize sizeNew = bHorz ? m_szHorz : m_szVert;
    cSize sizeDelta = sizeNew - m_szOld;
    if (m_nDockBarID == AFX_IDW_DOCKBAR_LEFT && m_htEdge == HTTOP ||
        m_nDockBarID == AFX_IDW_DOCKBAR_RIGHT && m_htEdge != HTBOTTOM ||
        m_nDockBarID == AFX_IDW_DOCKBAR_TOP && m_htEdge == HTLEFT ||
        m_nDockBarID == AFX_IDW_DOCKBAR_BOTTOM && m_htEdge != HTRIGHT)
        sizeDelta = -sizeDelta;
    rc.offsetRect(sizeDelta);

    CDC *pDC = m_pDockSite->GetDCEx(nullptr,
        DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);
    CBrush* pBrush = CDC::GetHalftoneBrush();
    CBrush* pBrushOld = pDC->SelectObject(pBrush);

    pDC->PatBlt(rc.left, rc.top, rc.width(), rc.height(), PATINVERT);
    
    pDC->SelectObject(pBrushOld);
    m_pDockSite->ReleaseDC(pDC);
}

bool CSizingControlBar::getEdgeRect(CRect rcWnd, uint32_t nHitTest,
                                    CRect& rcEdge)
{
    rcEdge = rcWnd;
    if (m_dwSCBStyle & SCBS_SHOWEDGES)
        rcEdge.deflate(1, 1);
    bool bHorz = isHorzDocked();

    switch (nHitTest)
    {
    case HTLEFT:
        if (!(m_dwSCBStyle & SCBS_EDGELEFT)) return false;
        rcEdge.right = rcEdge.left + m_cxEdge;
        rcEdge.deflate(0, bHorz ? m_cxEdge: 0);
        break;
    case HTTOP:
        if (!(m_dwSCBStyle & SCBS_EDGETOP)) return false;
        rcEdge.bottom = rcEdge.top + m_cxEdge;
        rcEdge.deflate(bHorz ? 0 : m_cxEdge, 0);
        break;
    case HTRIGHT:
        if (!(m_dwSCBStyle & SCBS_EDGERIGHT)) return false;
        rcEdge.left = rcEdge.right - m_cxEdge;
        rcEdge.deflate(0, bHorz ? m_cxEdge: 0);
        break;
    case HTBOTTOM:
        if (!(m_dwSCBStyle & SCBS_EDGEBOTTOM)) return false;
        rcEdge.top = rcEdge.bottom - m_cxEdge;
        rcEdge.deflate(bHorz ? 0 : m_cxEdge, 0);
        break;
    default:
        assert(false); // invalid hit test code
    }
    return true;
}

uint32_t CSizingControlBar::getEdgeHTCode(int nEdge)
{
    if (nEdge == 0) return HTLEFT;
    if (nEdge == 1) return HTTOP;
    if (nEdge == 2) return HTRIGHT;
    if (nEdge == 3) return HTBOTTOM;
    assert(false); // invalid edge no
    return HTNOWHERE;
}

void CSizingControlBar::getRowInfo(int& nFirst, int& nLast, int& nThis)
{
    ASSERT_VALID(m_pDockBar); // verify bounds

    nThis = m_pDockBar->FindBar(this);
    assert(nThis != -1);

    int i, nBars = m_pDockBar->m_arrBars.getSize();

    // find the first and the last bar in row
    for (nFirst = -1, i = nThis - 1; i >= 0 && nFirst == -1; i--)
        if (m_pDockBar->m_arrBars[i] == nullptr)
            nFirst = i + 1;
    for (nLast = -1, i = nThis + 1; i < nBars && nLast == -1; i++)
        if (m_pDockBar->m_arrBars[i] == nullptr)
            nLast = i - 1;

    assert((nLast != -1) && (nFirst != -1));
}

void CSizingControlBar::getRowSizingBars(CSCBArray& arrSCBars)
{
    arrSCBars.removeAll();

    int nFirst, nLast, nThis;
    getRowInfo(nFirst, nLast, nThis);

    for (int i = nFirst; i <= nLast; i++)
    {
        CControlBar* pBar = (CControlBar*)m_pDockBar->m_arrBars[i];
        if (HIWORD(pBar) == 0) continue; // placeholder
        if (!pBar->isVisible()) continue;
        if (findSizingBar(pBar) >= 0)
            arrSCBars.add((CSizingControlBar*)pBar);
    }
}

const int CSizingControlBar::findSizingBar(CControlBar* pBar) const
{
    for (int nPos = 0; nPos < m_arrBars.getSize(); nPos++)
        if (m_arrBars[nPos] == pBar)
            return nPos; // got it

    return -1; // not found
}

bool CSizingControlBar::negociateSpace(int nLengthAvail, bool bHorz)
{
    assert(bHorz == isHorzDocked());

    int nFirst, nLast, nThis, i;
    getRowInfo(nFirst, nLast, nThis);

    // step 1: subtract the visible fixed bars' lengths
    for (i = nFirst; i <= nLast; i++)
    {
        CControlBar* pFBar = (CControlBar*)m_pDockBar->m_arrBars[i];
        if (HIWORD(pFBar) == 0) continue; // placeholder
        if (!pFBar->isVisible() || (findSizingBar(pFBar) >= 0)) continue;

        CRect rcBar;
        pFBar->getWindowRect(&rcBar);

        nLengthAvail -= (bHorz ? rcBar.width() - 2 : rcBar.height() - 2);
    }

    CSCBArray arrSCBars;
    getRowSizingBars(arrSCBars);
    CSizingControlBar* pBar;

    // step 2: compute actual and min lengths; also the common width
    int nActualLength = 0;
    int nMinLength = 2;
    int nWidth = 0;
    for (i = 0; i < arrSCBars.getSize(); i++)
    {
        pBar = arrSCBars[i];
        nActualLength += bHorz ? pBar->m_szHorz.cx - 2 :
            pBar->m_szVert.cy - 2;
        nMinLength += bHorz ? pBar->m_szMin.cx - 2:
            pBar->m_szMin.cy - 2;
        nWidth = max(nWidth, bHorz ? pBar->m_szHorz.cy :
            pBar->m_szVert.cx);
    }
    
    // step 3: pop the bar out of the row if not enough room
    if (nMinLength > nLengthAvail)
    {
        if (nFirst < nThis || nThis < nLast)
        {   // not enough room - create a new row
            m_pDockBar->m_arrBars.insertAt(nLast + 1, this);
            m_pDockBar->m_arrBars.insertAt(nLast + 1, (CControlBar*) nullptr);
            m_pDockBar->m_arrBars.removeAt(nThis);
        }
        return false;
    }

    // step 4: make the bars same width
    for (i = 0; i < arrSCBars.getSize(); i++)
        if (bHorz)
            arrSCBars[i]->m_szHorz.cy = nWidth;
        else
            arrSCBars[i]->m_szVert.cx = nWidth;

    if (nActualLength == nLengthAvail)
        return true; // no change

    // step 5: distribute the difference between the bars, but
    //         don't shrink them below minsize
    int nDelta = nLengthAvail - nActualLength;

    while (nDelta != 0)
    {
        int nDeltaOld = nDelta;
        for (i = 0; i < arrSCBars.getSize(); i++)
        {
            pBar = arrSCBars[i];
            int nLMin = bHorz ? pBar->m_szMin.cx : pBar->m_szMin.cy;
            int nL = bHorz ? pBar->m_szHorz.cx : pBar->m_szVert.cy;
            
            if ((nL == nLMin) && (nDelta < 0) || // already at min length
                pBar->m_bKeepSize) // or wants to keep its size
                continue;
            
            // sign of nDelta
            int nDelta2 = (nDelta == 0) ? 0 : ((nDelta < 0) ? -1 : 1);

            (bHorz ? pBar->m_szHorz.cx : pBar->m_szVert.cy) += nDelta2;
            nDelta -= nDelta2;
            if (nDelta == 0) break;
        }
        // clear m_bKeepSize flags
        if ((nDeltaOld == nDelta) || (nDelta == 0))
            for (i = 0; i < arrSCBars.getSize(); i++)
                arrSCBars[i]->m_bKeepSize = false;
    }

    return true;
}

void CSizingControlBar::alignControlBars()
{
    int nFirst, nLast, nThis;
    getRowInfo(nFirst, nLast, nThis);

    bool bHorz = isHorzDocked();
    bool bNeedRecalc = false;
    int nPos, nAlign = bHorz ? -2 : 0;

    CRect rc, rcDock;
    m_pDockBar->getWindowRect(&rcDock);

    for (int i = nFirst; i <= nLast; i++)
    {
        CControlBar* pBar = (CControlBar*)m_pDockBar->m_arrBars[i];
        if (HIWORD(pBar) == 0) continue; // placeholder
        if (!pBar->isVisible()) continue;

        pBar->getWindowRect(&rc);
        rc.offsetRect(-rcDock.TopLeft());

        if ((nPos = findSizingBar(pBar)) >= 0)
            rc = CRect(rc.TopLeft(), bHorz ?
                m_arrBars[nPos]->m_szHorz : m_arrBars[nPos]->m_szVert);

        if ((bHorz ? rc.left : rc.top) != nAlign)
        {
            if (!bHorz)
                rc.offsetRect(0, nAlign - rc.top - 2);
            else if (m_nDockBarID == AFX_IDW_DOCKBAR_TOP)
                rc.offsetRect(nAlign - rc.left, -2);
            else
                rc.offsetRect(nAlign - rc.left, 0);
            pBar->moveWindow(rc);
            bNeedRecalc = true;
        }
        nAlign += (bHorz ? rc.width() : rc.height()) - 2;
    }

    if (bNeedRecalc)
    {
        m_pDockSite->DelayRecalcLayout();
        TRACE("ccc\n");
    }
}

void CSizingControlBar::onUpdateCmdUI(CFrameWnd* pTarget, bool bDisableIfNoHndler)
{
    bool bNeedPaint = false;

    CPoint pt;
    ::getCursorPos(&pt);
    bool bHit = (OnNcHitTest(pt) == HTCLOSE);
    bool bLButtonDown = (::GetKeyState(VK_LBUTTON) < 0);

    bool bWasPushed = m_biHide.bPushed;
    m_biHide.bPushed = bHit && bLButtonDown;

    bool bWasRaised = m_biHide.bRaised;
    m_biHide.bRaised = bHit && !bLButtonDown;

    bNeedPaint |= (m_biHide.bPushed ^ bWasPushed) || 
                  (m_biHide.bRaised ^ bWasRaised);

    if (bNeedPaint)
        sendMessage(WM_NCPAINT);
}

void CSizingControlBar::loadState(cstr_t lpszProfileName)
{
    ASSERT_VALID(this);
    assert(GetSafeHwnd()); // must be called after create()

    char szSection[256];
    wsprintf(szSection, "%s-SCBar-%d", lpszProfileName,
        GetDlgCtrlID());
//
//    m_szHorz.cx = max(m_szMin.cx, (int) pApp->getInt(szSection,
//        "sizeHorzCX", m_szHorz.cx));
//    m_szHorz.cy = max(m_szMin.cy, (int) pApp->getInt(szSection, 
//        "sizeHorzCY", m_szHorz.cy));
//
//    m_szVert.cx = max(m_szMin.cx, (int) pApp->getInt(szSection, 
//        "sizeVertCX", m_szVert.cx));
//    m_szVert.cy = max(m_szMin.cy, (int) pApp->getInt(szSection, 
//        "sizeVertCY", m_szVert.cy));
//
//    m_szFloat.cx = max(m_szMin.cx, (int) pApp->getInt(szSection,
//        "sizeFloatCX", m_szFloat.cx));
//    m_szFloat.cy = max(m_szMin.cy, (int) pApp->getInt(szSection,
//        "sizeFloatCY", m_szFloat.cy));

    m_nDockBarID = max(m_szMin.cx, (int) g_profile.getInt(szSection,
        "DockBarID", AFX_IDW_DOCKBAR_RIGHT));
    m_szHorz.cx = max(m_szMin.cx, (int) g_profile.getInt(szSection,
        "sizeHorzCX", m_szHorz.cx));
    m_szHorz.cy = max(m_szMin.cy, (int) g_profile.getInt(szSection, 
        "sizeHorzCY", m_szHorz.cy));

    m_szVert.cx = max(m_szMin.cx, (int) g_profile.getInt(szSection, 
        "sizeVertCX", m_szVert.cx));
    m_szVert.cy = max(m_szMin.cy, (int) g_profile.getInt(szSection, 
        "sizeVertCY", m_szVert.cy));

    m_szFloat.cx = max(m_szMin.cx, (int) g_profile.getInt(szSection,
        "sizeFloatCX", m_szFloat.cx));
    m_szFloat.cy = max(m_szMin.cy, (int) g_profile.getInt(szSection,
        "sizeFloatCY", m_szFloat.cy));
}

void CSizingControlBar::saveState(cstr_t lpszProfileName)
{
    // place your saveState or globalSaveState call in
    // CMainFrame::destroyWindow(), not in onDestroy()
    ASSERT_VALID(this);
    assert(GetSafeHwnd());

    char szSection[256];
    wsprintf(szSection, "%s-SCBar-%d", lpszProfileName,
        GetDlgCtrlID());

 //    pApp->writeInt(szSection, "sizeVertCX", m_szVert.cx);
    g_profile.writeInt(szSection, "DockBarID", m_nDockBarID);
    g_profile.writeInt(szSection, "sizeHorzCX", m_szHorz.cx);
    g_profile.writeInt(szSection, "sizeHorzCY", m_szHorz.cy);

    g_profile.writeInt(szSection, "sizeVertCX", m_szVert.cx);
    g_profile.writeInt(szSection, "sizeVertCY", m_szVert.cy);

    g_profile.writeInt(szSection, "sizeFloatCX", m_szFloat.cx);
    g_profile.writeInt(szSection, "sizeFloatCY", m_szFloat.cy);
}

void CSizingControlBar::globalLoadState(cstr_t lpszProfileName)
{
    for (int i = 0; i < m_arrBars.getSize(); i++)
        ((CSizingControlBar*) m_arrBars[i])->loadState(lpszProfileName);
}

void CSizingControlBar::globalSaveState(cstr_t lpszProfileName)
{
    for (int i = 0; i < m_arrBars.getSize(); i++)
        ((CSizingControlBar*) m_arrBars[i])->saveState(lpszProfileName);
}

/////////////////////////////////////////////////////////////////////////
// CSCBButton

CSCBButton::CSCBButton()
{
    bRaised = false;
    bPushed = false;
}

void CSCBButton::paint(CDC* pDC)
{
    CRect rc = getRect();

    if (bPushed)
        pDC->Draw3dRect(rc, ::GetSysColor(COLOR_BTNSHADOW),
            ::GetSysColor(COLOR_BTNHIGHLIGHT));
    else
        if (bRaised)
            pDC->Draw3dRect(rc, ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));

    COLORREF clrOldTextColor = pDC->getTextColor();
    pDC->setTextColor(::GetSysColor(COLOR_BTNTEXT));
    int nPrevBkMode = pDC->SetBkMode(TRANSPARENT);
    CFont font;
    int ppi = pDC->GetDeviceCaps(LOGPIXELSX);
    int pointsize = MulDiv(60, 96, ppi); // 6 points at 96 ppi
    font.CreatePointFont(pointsize, "Marlett");
    CFont* oldfont = pDC->SelectObject(&font);

    pDC->textOut(ptOrg.x + 2, ptOrg.y + 2, CString("r")); // x-like
    
    pDC->SelectObject(oldfont);
    pDC->SetBkMode(nPrevBkMode);
    pDC->setTextColor(clrOldTextColor);
}
