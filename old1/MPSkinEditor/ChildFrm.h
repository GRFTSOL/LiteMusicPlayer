// ChildFrm.h : interface of the CChildFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDFRM_H__6F2722A2_9769_4E9C_B7C8_36930FA2B337__INCLUDED_)
#define AFX_CHILDFRM_H__6F2722A2_9769_4E9C_B7C8_36930FA2B337__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildView.h"

class CChildFrame : public CMDIChildWnd
{
    DECLARE_DYNCREATE(CChildFrame)
public:
    CChildFrame();

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CChildFrame)
    public:
    virtual bool preCreateWindow(CREATESTRUCT& cs);
    virtual bool onCmdMsg(uint32_t nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    //}}AFX_VIRTUAL

// Implementation
public:
    // view for the client area of the frame.
    CSESkinWnd        m_wndSkin;
    virtual ~CChildFrame();
#ifdef _DEBUG
    virtual void assertValid() const;
    virtual void dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
    //{{AFX_MSG(CChildFrame)
    afx_msg void OnFileClose();
    afx_msg void onSetFocus(CWnd* pOldWnd);
    afx_msg int onCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void onPaint();
    afx_msg void onDestroy();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__6F2722A2_9769_4E9C_B7C8_36930FA2B337__INCLUDED_)
