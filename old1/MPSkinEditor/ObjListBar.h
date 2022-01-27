#if !defined(AFX_OBJLISTBAR_H__E7A9109D_B354_4C83_899A_D65C9F07AD19__INCLUDED_)
#define AFX_OBJLISTBAR_H__E7A9109D_B354_4C83_899A_D65C9F07AD19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjListBar.h : header file
//
#include "sizecbar.h"

/////////////////////////////////////////////////////////////////////////////
// CObjListBar window

class CObjListBar : public CSizingControlBar
{
// Construction
public:
    CObjListBar();

// Attributes
public:
    CTreeCtrl                    m_wndTreeObj;
    static CTreeCtrl            *ms_pwndTreeObj;

// Operations
public:
    virtual void onUpdateCmdUI(CFrameWnd* pTarget, bool bDisableIfNoHndler);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CObjListBar)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CObjListBar();

    // Generated message map functions
protected:
    //{{AFX_MSG(CObjListBar)
    afx_msg int onCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void onSize(uint32_t nType, int cx, int cy);
    afx_msg void onDestroy();
    afx_msg void OnSelchangeSkinObjList(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJLISTBAR_H__E7A9109D_B354_4C83_899A_D65C9F07AD19__INCLUDED_)
