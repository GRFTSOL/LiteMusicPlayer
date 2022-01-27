// ObjListBar.cpp : implementation file
//

#include "MPSkinEditor.h"
#include "ObjListBar.h"

#define IDC_SKIN_OBJ_LIST        101

CTreeCtrl    *CObjListBar::ms_pwndTreeObj = nullptr;

CTreeCtrl *getSkinUIObjTreeCtrl()
{
    return CObjListBar::ms_pwndTreeObj;
}


/////////////////////////////////////////////////////////////////////////////
// CObjListBar

CObjListBar::CObjListBar()
{
}

CObjListBar::~CObjListBar()
{
}


BEGIN_MESSAGE_MAP(CObjListBar, CSizingControlBar)
    //{{AFX_MSG_MAP(CObjListBar)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_NOTIFY(TVN_SELCHANGED, IDC_SKIN_OBJ_LIST, OnSelchangeSkinObjList)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CObjListBar message handlers

int CObjListBar::onCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CSizingControlBar::onCreate(lpCreateStruct) == -1)
        return -1;

//    loadState("ObjListBar");
    ms_pwndTreeObj = &m_wndTreeObj;

    CRect    rc;
    getClientRect(&rc);
    m_wndTreeObj.create(TVS_LINESATROOT | TVS_FULLROWSELECT | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER, rc, this, IDC_SKIN_OBJ_LIST);
    m_wndTreeObj.showWindow(SW_SHOW);

    return 0;
}

void CObjListBar::onUpdateCmdUI(CFrameWnd* pTarget, bool bDisableIfNoHndler)
{
    CSizingControlBar::onUpdateCmdUI(pTarget, bDisableIfNoHndler);

    UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

void CObjListBar::onSize(uint32_t nType, int cx, int cy) 
{
    CSizingControlBar::onSize(nType, cx, cy);

    // TODO: add your message handler code here
    CRect rc;
    getClientRect(rc);

    if (m_wndTreeObj.GetSafeHwnd())
        m_wndTreeObj.moveWindow(rc);
}

void CObjListBar::onDestroy() 
{
    CSizingControlBar::onDestroy();
}

void CObjListBar::OnSelchangeSkinObjList(NMHDR* pNMHDR, LRESULT* pResult)
{
    CMDIFrameWnd *pFame = (CMDIFrameWnd *)AfxGetMainWnd();
    CWnd *pChild = pFame->GetActiveFrame();
    if (pChild)
    {
        pChild->sendMessage(WM_COMMAND, IDC_SKIN_OBJ_LIST_SEL_CHANGED);
    }
}
