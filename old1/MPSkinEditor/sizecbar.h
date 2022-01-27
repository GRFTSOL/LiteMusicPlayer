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

#if !defined(__SIZECBAR_H__)
#define __SIZECBAR_H__

#include <afxpriv.h>    // for CDockContext
#include <afxtempl.h>   // for CArray

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////
// CSCBButton (button info) helper class

class CSCBButton
{
public:
    CSCBButton();

    void move(CPoint ptTo) {ptOrg = ptTo; };
    CRect getRect() { return CRect(ptOrg, cSize(11, 11)); };
    void paint(CDC* pDC);

    bool    bPushed;
    bool    bRaised;

protected:
    CPoint  ptOrg;
};

/////////////////////////////////////////////////////////////////////////
// CSCBDockBar dummy class for access to protected members

class CSCBDockBar : public CDockBar
{
    friend class CSizingControlBar;
};

/////////////////////////////////////////////////////////////////////////
// CSizingControlBar control bar styles

#define SCBS_EDGELEFT       0x00000001
#define SCBS_EDGERIGHT      0x00000002
#define SCBS_EDGETOP        0x00000004
#define SCBS_EDGEBOTTOM     0x00000008
#define SCBS_EDGEALL        0x0000000F
#define SCBS_SHOWEDGES      0x00000010
#define SCBS_GRIPPER        0x00000020

/////////////////////////////////////////////////////////////////////////
// CSizingControlBar control bar

#ifndef baseCSizingControlBar
#define baseCSizingControlBar CControlBar
#endif

class CSizingControlBar;
typedef CTypedPtrArray <CPtrArray, CSizingControlBar*> CSCBArray;

class CSizingControlBar : public baseCSizingControlBar
{
    DECLARE_DYNAMIC(CSizingControlBar);

// Construction
protected:
    CSizingControlBar();

public:
    virtual bool create(cstr_t lpszWindowName, CWnd* pParentWnd,
        cSize sizeDefault, bool bHasGripper, uint32_t nID,
        uint32_t dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP);

// Attributes
public:
    cSize m_szHorz;
    cSize m_szVert;
    cSize m_szFloat;
    const bool isFloating() const;
    const bool isHorzDocked() const;
    const bool isVertDocked() const;
    const bool isSideTracking() const;

// Operations
public:
    virtual void loadState(cstr_t lpszProfileName);
    virtual void saveState(cstr_t lpszProfileName);
    static void globalLoadState(cstr_t lpszProfileName);
    static void globalSaveState(cstr_t lpszProfileName);

    int getDockingBarID() { return m_nDockBarID; }

// Overridables
    virtual void onUpdateCmdUI(CFrameWnd* pTarget, bool bDisableIfNoHndler);

// Overrides
public:
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSizingControlBar)
    public:
    virtual cSize calcFixedLayout(bool bStretch, bool bHorz);
    virtual cSize calcDynamicLayout(int nLength, uint32_t dwMode);
    virtual bool destroyWindow();
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CSizingControlBar();
    
protected:
    // implementation helpers
    uint32_t getEdgeHTCode(int nEdge);
    bool getEdgeRect(CRect rcWnd, uint32_t nHitTest, CRect& rcEdge);
    virtual void startTracking(uint32_t nHitTest);
    virtual void stopTracking();
    virtual void onTrackUpdateSize(CPoint& point);
    virtual void onTrackInvertTracker();
    virtual void ncPaintGripper(CDC* pDC, CRect rcClient);

    virtual void alignControlBars();
    const int findSizingBar(CControlBar* pBar) const;
    void getRowInfo(int& nFirst, int& nLast, int& nThis);
    void getRowSizingBars(CSCBArray& arrSCBars);
    bool negociateSpace(int nLengthAvail, bool bHorz);

protected:
    static CSCBArray    m_arrBars;

    uint32_t   m_dwSCBStyle;
    uint32_t    m_htEdge;

    cSize   m_szMin;
    cSize   m_szMinT;
    cSize   m_szMaxT;
    cSize   m_szOld;
    CPoint  m_ptOld;
    bool    m_bTracking;
    bool    m_bKeepSize;
    bool    m_bParentSizing;
    bool    m_bDragShowContent;
    uint32_t    m_nDockBarID;
    int     m_cxEdge;
    int     m_cyGripper;

    CSCBButton m_biHide;

// Generated message map functions
protected:
    //{{AFX_MSG(CSizingControlBar)
    afx_msg int onCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnNcPaint();
    afx_msg void OnNcCalcSize(bool bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnCaptureChanged(CWnd *pWnd);
    afx_msg void OnSettingChange(uint32_t uFlags, cstr_t lpszSection);
    afx_msg void onLButtonUp(uint32_t nFlags, CPoint point);
    afx_msg void onMouseMove(uint32_t nFlags, CPoint point);
    afx_msg void OnNcLButtonDown(uint32_t nHitTest, CPoint point);
    afx_msg void onLButtonDown(uint32_t nFlags, CPoint point);
    afx_msg void onLButtonDblClk(uint32_t nFlags, CPoint point);
    afx_msg void onRButtonDown(uint32_t nFlags, CPoint point);
    afx_msg void OnNcLButtonUp(uint32_t nHitTest, CPoint point);
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    afx_msg void onPaint();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

#endif // !defined(__SIZECBAR_H__)

