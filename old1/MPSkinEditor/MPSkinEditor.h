// MPSkinEditor.h : main header file for the MPSKINEDITOR application
//

#if !defined(AFX_MPSKINEDITOR_H__49AEFAE1_773E_4D83_9104_16B4EB463065__INCLUDED_)
#define AFX_MPSKINEDITOR_H__49AEFAE1_773E_4D83_9104_16B4EB463065__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "SESkinPrj.h"

CTreeCtrl *getSkinUIObjTreeCtrl();

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp:
// See MPSkinEditor.cpp for the implementation of this class
//

class CMPSkinEditorApp : public CWinApp
{
public:
    CMPSkinEditorApp();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMPSkinEditorApp)
    public:
    virtual bool initInstance();
    virtual int exitInstance();
    //}}AFX_VIRTUAL

// Implementation
protected:
    HMENU m_hMDIMenu, m_hDynCmdMenu;
    HACCEL m_hMDIAccel;

public:
    //{{AFX_MSG(CMPSkinEditorApp)
    afx_msg void OnAppAbout();
    afx_msg void OnFileNew();
    afx_msg void OnOpenSkin();
    afx_msg void OnSkinWndNew();
    afx_msg void OnSkinWndDup();
    afx_msg void OnSkinWndDel();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MPSKINEDITOR_H__49AEFAE1_773E_4D83_9104_16B4EB463065__INCLUDED_)
