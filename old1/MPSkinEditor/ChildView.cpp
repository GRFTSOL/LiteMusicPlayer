// ChildView.cpp : implementation of the CChildView class
//

#include "MPSkinEditor.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
    //{{AFX_MSG_MAP(CChildView)
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

bool CChildView::preCreateWindow(CREATESTRUCT& cs) 
{
    if (!CWnd::preCreateWindow(cs))
        return false;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
        ::loadCursor(nullptr, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), nullptr);

    return true;
}

void CChildView::onPaint() 
{
    CPaintDC dc(this); // device context for painting
    
    // TODO: add your message handler code here
    
    // Do not call CWnd::onPaint() for painting messages
}

