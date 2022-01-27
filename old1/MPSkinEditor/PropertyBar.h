#if !defined(AFX_PROPERTYBAR_H__801970CC_52B6_416F_B371_C7300385DB4D__INCLUDED_)
#define AFX_PROPERTYBAR_H__801970CC_52B6_416F_B371_C7300385DB4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyBar.h : header file
//
#include "sizecbar.h"
#include "SEPropertyListCtrl.h"
#include "UIEditObject.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertyBar window

class CPropertyBar : public CSizingControlBar
{
// Construction
public:
    CPropertyBar();

// Attributes
public:
    static CSEPropertyListCtrl    *ms_pwndProperty;
    CFont   m_font;

    CFont            m_ListBoxFont;

// Operations
public:
    virtual void onUpdateCmdUI(CFrameWnd* pTarget, bool bDisableIfNoHndler);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CPropertyBar)
    protected:
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CPropertyBar();

    // Generated message map functions
protected:
    //{{AFX_MSG(CPropertyBar)
    afx_msg int onCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void onSize(uint32_t nType, int cx, int cy);
    afx_msg void onDestroy();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYBAR_H__801970CC_52B6_416F_B371_C7300385DB4D__INCLUDED_)
