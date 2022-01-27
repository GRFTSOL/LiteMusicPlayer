// MainFrm.cpp : implementation of the CMainFrame class
//

#include "MPSkinEditor.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_COMMAND(IDC_SAVE_SKIN, OnSaveSkin)
    ON_COMMAND(IDC_VIEW_OBJLIST_BAR, OnViewObjlistBar)
    ON_COMMAND(IDC_VIEW_PROPERTY_BAR, OnViewPropertyBar)
    ON_UPDATE_COMMAND_UI(IDC_MOUSE_POINTER, OnUpdatePaneMousePointer)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

static uint32_t indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_SEPARATOR,
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    m_bTop = true;
    m_bImages = true;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::onCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWnd::onCreate(lpCreateStruct) == -1)
        return -1;
    
    if (!m_wndToolBar.createEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if (!m_wndStatusBar.create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(uint32_t)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    m_wndStatusBar.SetPaneText(1, "X: Y: ");

    // TODO: delete these three lines if you don't want the toolbar to
    //  be dockable
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

    if (!m_wndObjListBar.create("Object List", this, cSize(200, 200),
        true, 123 + 2))//AFX_IDW_CONTROLBAR_FIRST + 33 + i))
    {
        ERR_LOG0("Failed to create m_wndLogBar\n");
        return -1;      // fail to create
    }

    m_wndObjListBar.SetBarStyle(m_wndObjListBar.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
    m_wndObjListBar.EnableDocking(CBRS_ALIGN_LEFT);


    if (!m_wndPropertyBar.create("Property", this, cSize(200, 200),
        true, 123 + 0))//AFX_IDW_CONTROLBAR_FIRST + 33 + i))
    {
        ERR_LOG0("Failed to create mybar\n");
        return -1;      // fail to create
    }

    m_wndPropertyBar.SetBarStyle(m_wndPropertyBar.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
    m_wndPropertyBar.EnableDocking(CBRS_ALIGN_LEFT);


    DockControlBar(&m_wndObjListBar, AFX_IDW_DOCKBAR_LEFT);

    DockControlBar(&m_wndPropertyBar, AFX_IDW_DOCKBAR_LEFT);

    CSizingControlBar::globalLoadState("BarState");
    LoadBarState("BarState");

    // CMDITabs must be createt at last to ensure correct layout!!!
    // ------------------------------------------------------------
    uint32_t dwStyle = (m_bTop ? MT_TOP : 0) | (m_bImages ? MT_IMAGES : 0);
    m_wndMDITabs.create(this, dwStyle);

    showWindow(SW_SHOWMAXIMIZED);

    return 0;
}

void CMainFrame::onUpdateFrameTitle(bool bAddToTitle)
{
  CMDIFrameWnd::onUpdateFrameTitle(bAddToTitle);

  m_wndMDITabs.update(); // sync the mditabctrl with all views
}

bool CMainFrame::preCreateWindow(CREATESTRUCT& cs)
{
    if( !CMDIFrameWnd::preCreateWindow(cs) )
        return false;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::assertValid() const
{
    CMDIFrameWnd::assertValid();
}

void CMainFrame::dump(CDumpContext& dc) const
{
    CMDIFrameWnd::dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::onDestroy() 
{
    CSizingControlBar::globalSaveState("BarState");
    SaveBarState("BarState");

    CMDIFrameWnd::onDestroy();
}

void CMainFrame::OnSaveSkin() 
{
    g_skinPrj.save();
/*    CFileDialog ofdlg(false, 0, g_profile.getString("LastOpenSknFile", ""), OFN_FILEMUSTEXIST, "Skin files(*.xml)|*.xml|All files (*.*)|*.*||", this);
    if (ofdlg.doModal() != IDOK)
        return;

    g_profile.writeString("LastOpenSknFile", ofdlg.GetPathName());

    g_skinPrj.save(ofdlg.GetPathName());*/
}

void CMainFrame::OnViewObjlistBar() 
{
    if (!m_wndObjListBar.isVisible())
        ShowControlBar(&m_wndObjListBar, true, false);
    else
        ShowControlBar(&m_wndObjListBar, false, false);
}

void CMainFrame::OnViewPropertyBar() 
{
    if (!m_wndPropertyBar.isVisible())
        ShowControlBar(&m_wndPropertyBar, true, false);
    else
        ShowControlBar(&m_wndPropertyBar, false, false);
}
