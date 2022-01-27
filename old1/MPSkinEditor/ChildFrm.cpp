// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "MPSkinEditor.h"

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
    //{{AFX_MSG_MAP(CChildFrame)
        ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
        ON_WM_SETFOCUS()
        ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
    // TODO: add member initialization code here
    
}

CChildFrame::~CChildFrame()
{
}

bool CChildFrame::preCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    if( !CMDIChildWnd::preCreateWindow(cs) )
        return false;

    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.lpszClass = AfxRegisterWndClass(0);

    return true;
}



/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::assertValid() const
{
    CMDIChildWnd::assertValid();
}

void CChildFrame::dump(CDumpContext& dc) const
{
    CMDIChildWnd::dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers
void CChildFrame::OnFileClose() 
{
    // To close the frame, just send a WM_CLOSE, which is the equivalent
    // choosing close from the system menu.

    sendMessage(WM_CLOSE);
}

int CChildFrame::onCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CMDIChildWnd::onCreate(lpCreateStruct) == -1)
        return -1;
    
    // create a view to occupy the client area of the frame
//     if (!m_wndView.create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, 
//         CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, nullptr))
//     {
//         TRACE0("Failed to create view window\n");
//         return -1;
//     }

    g_skinPrj.OnCreateSkinWnd(&m_wndSkin);

    return 0;
}

void CChildFrame::onSetFocus(CWnd* pOldWnd) 
{
    CMDIChildWnd::onSetFocus(pOldWnd);

//     m_wndView.setFocus();
}

bool CChildFrame::onCmdMsg(uint32_t nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
//     // let the view have first crack at the command
//     if (m_wndView.onCmdMsg(nID, nCode, pExtra, pHandlerInfo))
//         return true;
//     
    // otherwise, do default handling
    return CMDIChildWnd::onCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CChildFrame::onPaint() 
{
    CPaintDC dc(this); // device context for painting

    CRect        rc;

    getClientRect(&rc);

    dc.FillSolidRect(&rc, RGB(255, 255, 255));
}

void CChildFrame::onDestroy() 
{
    CMDIChildWnd::onDestroy();
    
    g_skinPrj.OnDestroySkinWnd(&m_wndSkin);
}
