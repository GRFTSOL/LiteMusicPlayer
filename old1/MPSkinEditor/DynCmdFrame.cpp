// ChildFrm.cpp : implementation of the CDynCmdFrame class
//

#include "MPSkinEditor.h"

#include "DynCmdFrame.h"

#include "PropertyBar.h"
#include "SEPropertyListCtrl.h"

extern class CSkinEditorFrame        *g_pActiveFrameOld;

/////////////////////////////////////////////////////////////////////////////
// CDynCmdFrame

IMPLEMENT_DYNCREATE(CDynCmdFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CDynCmdFrame, CMDIChildWnd)
    //{{AFX_MSG_MAP(CDynCmdFrame)
    ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
    ON_WM_SETFOCUS()
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_DESTROY()
    ON_WM_CHILDACTIVATE()
    ON_WM_SYSCOMMAND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynCmdFrame construction/destruction

CDynCmdFrame::CDynCmdFrame()
{
    // TODO: add member initialization code here
    
}

CDynCmdFrame::~CDynCmdFrame()
{
}

bool CDynCmdFrame::preCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    if( !CMDIChildWnd::preCreateWindow(cs) )
        return false;

    // cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.lpszClass = AfxRegisterWndClass(0);

    return true;
}



/////////////////////////////////////////////////////////////////////////////
// CDynCmdFrame diagnostics

#ifdef _DEBUG
void CDynCmdFrame::assertValid() const
{
    CMDIChildWnd::assertValid();
}

void CDynCmdFrame::dump(CDumpContext& dc) const
{
    CMDIChildWnd::dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDynCmdFrame message handlers
void CDynCmdFrame::OnFileClose() 
{
    // To close the frame, just send a WM_CLOSE, which is the equivalent
    // choosing close from the system menu.

    sendMessage(WM_CLOSE);
}

int CDynCmdFrame::onCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CMDIChildWnd::onCreate(lpCreateStruct) == -1)
        return -1;

    // CRect    rc;
    // getClientRect(&rc);
    // m_wndGrid.create(rc, this, 101);
    m_wndGrid.create(CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST);
    m_wndGrid.SetColumnCount(3);
    m_wndGrid.SetRowCount(10);
    m_wndGrid.SetFixedRowCount(1);
    m_wndGrid.setColumnWidth(0, 200);
    m_wndGrid.setColumnWidth(1, 200);
    m_wndGrid.setColumnWidth(2, 200);

    m_wndGrid.setItemText(0, COL_ID, "ID");
    m_wndGrid.setItemText(0, COL_FUNC, "Function");
    m_wndGrid.setItemText(0, COL_PARAM, "Parameter");

    CDynamicCmds &dc = g_skinFactory.getDynamicCmds();
    CDynamicCmds::LIST_DYNCMD &listdc = dc.getDataList();
    CDynamicCmds::LIST_DYNCMD::iterator it, itEnd;
    int        i = 1;
    itEnd = listdc.end();
    for (it = listdc.begin(); it != itEnd; ++it, i++)
    {
        CDynamicCmds::DynamicCmd    &item = *it;
        m_wndGrid.InsertRow("");
        m_wndGrid.setItemText(i, COL_ID, item.strCmdId.c_str());
        m_wndGrid.setItemText(i, COL_FUNC, item.strFunction.c_str());
        m_wndGrid.setItemText(i, COL_PARAM, item.strParam.c_str());
    }

    g_skinPrj.onCreateSkinDataItem(this);

    return 0;
}

void CDynCmdFrame::onSetFocus(CWnd* pOldWnd) 
{
    CMDIChildWnd::onSetFocus(pOldWnd);

    m_wndGrid.setFocus();
}

void CDynCmdFrame::onPaint() 
{
    CPaintDC dc(this); // device context for painting
// 
//     CRect        rc;
// 
//     getClientRect(&rc);
// 
//     dc.FillSolidRect(&rc, RGB(255, 255, 255));
}

void CDynCmdFrame::onSave()
{
    CDynamicCmds &dc = g_skinFactory.getDynamicCmds();
    CDynamicCmds::LIST_DYNCMD &ldc = dc.getDataList();
    CString        str;
    ldc.clear();

    for (int i  = 1; i < m_wndGrid.getRowCount(); i++)
    {
        CDynamicCmds::LIST_DYNCMD::iterator it, itEnd;

        str = m_wndGrid.getItemText(i, COL_ID);
        if (str.isEmpty())
            continue;

        // is this command ()id existed?
        itEnd = ldc.end();
        for (it = ldc.begin(); it != itEnd; ++it)
        {
            CDynamicCmds::DynamicCmd    &item = *it;
            if (strcasecmp(item.strCmdId.c_str(), str) == 0)
                break;
        }

        if (it == itEnd)
        {
            // add it
            CDynamicCmds::DynamicCmd    newitem;
            newitem.strCmdId = str;
            newitem.strFunction = m_wndGrid.getItemText(i, COL_FUNC);
            newitem.strParam = m_wndGrid.getItemText(i, COL_PARAM);
            ldc.push_back(newitem);
        }
    }
}

void CDynCmdFrame::onDestroy() 
{
    g_skinPrj.onDestroySkinDataItem(this);

    CMDIChildWnd::onDestroy();
}

void CDynCmdFrame::OnChildActivate() 
{
    CMDIChildWnd::OnChildActivate();
    
    if (g_seSkinPropertyAdapter.isSrcChanged(nullptr, nullptr))
    {
        CPropertyBar::ms_pwndProperty->resetContent();
    }

    if (g_pActiveFrameOld != nullptr)
    {
        CTreeCtrl        *pTree = getSkinUIObjTreeCtrl();
        pTree->deleteAllItems();
    }
}

void CDynCmdFrame::OnSysCommand(uint32_t nID, LPARAM lParam) 
{
    if (nID == SC_CLOSE)
        return;

    CMDIChildWnd::OnSysCommand(nID, lParam);
}
