#if !defined(AFX_DLGCHOOSEIDS_H__AAC1C21B_FE2A_4DA9_887C_82DB76C13AFB__INCLUDED_)
#define AFX_DLGCHOOSEIDS_H__AAC1C21B_FE2A_4DA9_887C_82DB76C13AFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgChooseIDs.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgChooseIDs dialog

class CDlgChooseIDs : public CDialog
{
// Construction
public:
    CDlgChooseIDs(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CDlgChooseIDs)
    enum { IDD = IDD_CHOOSE_ID };
    CListCtrl    m_listIDs;
    //}}AFX_DATA

    string            m_strID;


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDlgChooseIDs)
    protected:
    virtual void doDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CDlgChooseIDs)
    afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
    virtual void onOK();
    virtual bool onInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCHOOSEIDS_H__AAC1C21B_FE2A_4DA9_887C_82DB76C13AFB__INCLUDED_)
