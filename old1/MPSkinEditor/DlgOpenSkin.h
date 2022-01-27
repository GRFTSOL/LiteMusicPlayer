#if !defined(AFX_DLGOPENSKIN_H__3D7E0FB3_C34F_4C74_B2DB_C4FBB2DC5501__INCLUDED_)
#define AFX_DLGOPENSKIN_H__3D7E0FB3_C34F_4C74_B2DB_C4FBB2DC5501__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgOpenSkin.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgOpenSkin dialog

class CDlgOpenSkin : public CDialog
{
// Construction
public:
    CDlgOpenSkin(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CDlgOpenSkin)
    enum { IDD = IDD_OPEN_SKIN };
    CListBox        m_lbSkins;
    CString            m_strSkinsRootDir;
    CString            m_strInfo;
    //}}AFX_DATA

    string                m_strSelSkin;

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDlgOpenSkin)
    protected:
    virtual void doDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CDlgOpenSkin)
    virtual bool onInitDialog();
    virtual void onOK();
    afx_msg void OnBrSkinRootDir();
    afx_msg void OnDblclkLSkins();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGOPENSKIN_H__3D7E0FB3_C34F_4C74_B2DB_C4FBB2DC5501__INCLUDED_)
