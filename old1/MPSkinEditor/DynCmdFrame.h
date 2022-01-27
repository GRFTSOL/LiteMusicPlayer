// ChildFrm.h : interface of the CDynCmdFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ChildView.h"
#include "../GridCtrl/GridCtrl.h"

class CDynCmdFrame : public CMDIChildWnd, public ISESkinDataItem
{
    DECLARE_DYNCREATE(CDynCmdFrame)
public:
    CDynCmdFrame();

    enum
    {
        COL_ID,
        COL_FUNC,
        COL_PARAM,
    };

// Attributes
public:
    CGridCtrl        m_wndGrid;

// Operations
public:
    virtual void onSave();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDynCmdFrame)
    public:
    virtual bool preCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

// Implementation
public:
    // view for the client area of the frame.
    virtual ~CDynCmdFrame();
#ifdef _DEBUG
    virtual void assertValid() const;
    virtual void dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
    //{{AFX_MSG(CDynCmdFrame)
    afx_msg void OnFileClose();
    afx_msg void onSetFocus(CWnd* pOldWnd);
    afx_msg int onCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void onPaint();
    afx_msg void onDestroy();
    afx_msg void OnChildActivate();
    afx_msg void OnSysCommand(uint32_t nID, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
