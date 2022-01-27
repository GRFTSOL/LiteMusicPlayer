#if !defined(AFX_DLGIMAGEPROPERTY_H__5506FBF2_09FE_4BF1_B17F_05E80B49BE75__INCLUDED_)
#define AFX_DLGIMAGEPROPERTY_H__5506FBF2_09FE_4BF1_B17F_05E80B49BE75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgImageProperty.h : header file
//

#include "ImageRectSel.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgImageProperty dialog

class CDlgImageProperty : public CDialog
{
// Construction
public:
    CDlgImageProperty(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CDlgImageProperty)
    enum { IDD = IDD_IMAGE_PROPERTY };
    CImageRectSelCtrl    m_wndImageRectSel;
    CComboBox    m_cbImage;
    int        m_ncx;
    int        m_ncy;
    int        m_nx;
    int        m_ny;
    int        m_nStretchStart;
    int        m_nStretchStart2;
    int        m_nStretchEnd;
    int        m_nStretchEnd2;
    //}}AFX_DATA

    string                m_strName;
    string                m_strFile;
    vector<string>        m_vNameRects;

    CWndHelper            m_wndHelper;

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDlgImageProperty)
    protected:
    virtual void doDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CDlgImageProperty)
    virtual bool onInitDialog();
    afx_msg void OnSelchangeCbImage();
    afx_msg void OnUpdateRc();
    afx_msg void OnChangeECx();
    afx_msg void OnChangeEY();
    afx_msg void OnChangeEX();
    afx_msg void OnChangeECy();
    afx_msg void OnRoomIn();
    afx_msg void OnRoomOut();
    afx_msg void onDestroy();
    afx_msg void OnChangeEStretchStart();
    afx_msg void OnChangeEStretchEnd();
    afx_msg void OnChangeEStretchStart2();
    afx_msg void OnChangeEStretchEnd2();
    virtual void onOK();
    afx_msg void onSize(uint32_t nType, int cx, int cy);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGIMAGEPROPERTY_H__5506FBF2_09FE_4BF1_B17F_05E80B49BE75__INCLUDED_)
