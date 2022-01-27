// DynCtrlFrame.cpp : implementation of the CDynCtrlFrame class
//

#include "MPSkinEditor.h"

#include "DynCtrlFrame.h"

#include "PropertyBar.h"
#include "SEPropertyListCtrl.h"

extern class CSkinEditorFrame        *g_pActiveFrameOld;

/////////////////////////////////////////////////////////////////////////////
// CDynCtrlFrame

IMPLEMENT_DYNCREATE(CDynCtrlFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CDynCtrlFrame, CMDIChildWnd)
    //{{AFX_MSG_MAP(CDynCtrlFrame)
        ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
        ON_WM_SETFOCUS()
        ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_CHILDACTIVATE()
    ON_WM_SYSCOMMAND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynCtrlFrame construction/destruction

CDynCtrlFrame::CDynCtrlFrame()
{
    // TODO: add member initialization code here
    
}

CDynCtrlFrame::~CDynCtrlFrame()
{
}

bool CDynCtrlFrame::preCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    if( !CMDIChildWnd::preCreateWindow(cs) )
        return false;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    
    cs.lpszClass = AfxRegisterWndClass(0);

    return true;
}



/////////////////////////////////////////////////////////////////////////////
// CDynCtrlFrame diagnostics

#ifdef _DEBUG
void CDynCtrlFrame::assertValid() const
{
    CMDIChildWnd::assertValid();
}

void CDynCtrlFrame::dump(CDumpContext& dc) const
{
    CMDIChildWnd::dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDynCtrlFrame message handlers
void CDynCtrlFrame::OnFileClose() 
{
    // To close the frame, just send a WM_CLOSE, which is the equivalent
    // choosing close from the system menu.

    sendMessage(WM_CLOSE);
}

int CDynCtrlFrame::onCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CMDIChildWnd::onCreate(lpCreateStruct) == -1)
        return -1;
    
    CRect    rc;
    getClientRect(&rc);
    // create a view to occupy the client area of the frame
    m_wndEdit.create(ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL,
        rc, this, AFX_WS_DEFAULT_VIEW);
    rc.top += 3;
    rc.bottom -= 3;
    m_wndEdit.setLTRB(&rc);
    m_wndEdit.SetMargins(10, 10);

    SetWindowLong(m_wndEdit.m_hWnd, GWL_EXSTYLE, GetWindowLong(m_wndEdit.m_hWnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

    // g_skinFactory.getDynamicCmds()
    m_wndEdit.setFont(CFont::fromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));

    //
    // set ctrl data
    //
    CDynamicCtrls &dc = g_skinFactory.getDynamicCtrls();
    SXNode        *pNode = dc.getData();
    CXMLWriter    xmlStream(false);
    string    str;

    pNode->toXML(xmlStream);
    utf8ToUCS2(xmlStream.getData(), xmlStream.getLength(), str);
    m_wndEdit.setWindowText(str.c_str());

    g_skinPrj.onCreateSkinDataItem(this);

    return 0;
}

void CDynCtrlFrame::onSetFocus(CWnd* pOldWnd) 
{
    CMDIChildWnd::onSetFocus(pOldWnd);

    m_wndEdit.setFocus();
}

void CDynCtrlFrame::onSave()
{
    CString        str;
    CSimpleXML    xml;
    string    strUtf8;

    m_wndEdit.getWindowText(str);
    str.insert(0, "<?xml version=\"1.0\" encoding='utf-8'?>");
    ucs2ToUtf8(str, str.getLength(), strUtf8);
    if (xml.parseData((void *)strUtf8.c_str(), strUtf8.size()))
    {
        CDynamicCtrls &dc = g_skinFactory.getDynamicCtrls();
        dc.fromXML(xml.m_pRoot);
    }
}

void CDynCtrlFrame::onDestroy() 
{
    g_skinPrj.onDestroySkinDataItem(this);

    CMDIChildWnd::onDestroy();
}

void CDynCtrlFrame::onSize(uint32_t nType, int cx, int cy) 
{
    CMDIChildWnd::onSize(nType, cx, cy);
    
    if (m_wndEdit.m_hWnd)
    {
        CRect    rc;
        getClientRect(&rc);
        m_wndEdit.moveWindow(&rc);
    }
}

void CDynCtrlFrame::OnChildActivate() 
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

void CDynCtrlFrame::OnSysCommand(uint32_t nID, LPARAM lParam) 
{
    if (nID == SC_CLOSE)
        return;

    CMDIChildWnd::OnSysCommand(nID, lParam);
}
