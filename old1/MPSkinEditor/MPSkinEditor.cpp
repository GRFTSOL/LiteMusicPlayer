// MPSkinEditor.cpp : Defines the class behaviors for the application.
//

#include "MPSkinEditor.h"

#include "MainFrm.h"
#include "SkinEditorFrame.h"
#include "DynCmdFrame.h"
#include "DynCtrlFrame.h"
#include "DlgOpenSkin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp

BEGIN_MESSAGE_MAP(CMPSkinEditorApp, CWinApp)
    //{{AFX_MSG_MAP(CMPSkinEditorApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_FILE_NEW, OnFileNew)
    ON_COMMAND(IDC_OPEN_SKIN, OnOpenSkin)
    ON_COMMAND(IDC_SKIN_WND_NEW, OnSkinWndNew)
    ON_COMMAND(IDC_SKIN_WND_DUP, OnSkinWndDup)
    ON_COMMAND(IDC_SKIN_WND_DEL, OnSkinWndDel)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp construction

CMPSkinEditorApp::CMPSkinEditorApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in initInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMPSkinEditorApp object

CMPSkinEditorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp initialization

bool CMPSkinEditorApp::initInstance()
{
    AfxEnableControlContainer();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.

    initBaseFrameWork(AfxGetInstanceHandle(), "ZPSkinEditorLog.txt", "ZPSkinEditor.ini", "ZPSkinEditor");

/*
    {
        CSimpleXML    xml;
        xml.parseFile("c:\\1.txt");
    }
    {
        CSimpleXML    xml;
        xml.parseFile("c:\\2.txt");
    }
    {
        CSimpleXML    xml;
        xml.parseFile("c:\\3.txt");
    }
    {
        CSimpleXML    xml;
        xml.parseFile("c:\\4.txt");
    }
*/

#ifdef _AFXDLL
    Enable3dControls();            // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();    // Call this when linking to MFC statically
#endif

    // Change the registry key under which our settings are stored.
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization.
    SetRegistryKey("Local AppWizard-Generated Applications");


    // To create the main window, this code creates a new frame window
    // object and then sets it as the application's main window object.

    CMDIFrameWnd* pFrame = new CMainFrame;
    m_pMainWnd = pFrame;

    // create main MDI frame window
    if (!pFrame->LoadFrame(IDR_MAINFRAME))
        return false;

    // try to load shared MDI menus and accelerator table
    //TODO: add additional member variables and load calls for
    //    additional menu types your application may need. 

    HINSTANCE hInst = AfxGetResourceHandle();
    m_hMDIMenu  = ::loadMenu(hInst, MAKEINTRESOURCE(IDR_MPSKINTYPE));
    m_hMDIAccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_MPSKINTYPE));
    m_hDynCmdMenu  = ::loadMenu(hInst, MAKEINTRESOURCE(IDR_DYNCMD));



    // The main window has been initialized, so show and update it.
    pFrame->showWindow(m_nCmdShow);
    pFrame->UpdateWindow();

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp message handlers

int CMPSkinEditorApp::exitInstance() 
{
    //TODO: handle additional resources you may have added
    if (m_hMDIMenu != nullptr)
        FreeResource(m_hMDIMenu);
    if (m_hMDIAccel != nullptr)
        FreeResource(m_hMDIAccel);
    if (m_hDynCmdMenu != nullptr)
        FreeResource(m_hDynCmdMenu);

    return CWinApp::exitInstance();
}

void CMPSkinEditorApp::OnFileNew() 
{
}



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App about

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
    protected:
    virtual void doDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
        // No message handlers
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::doDataExchange(CDataExchange* pDX)
{
    CDialog::doDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
        // No message handlers
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CMPSkinEditorApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.doModal();
}

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp message handlers


void CMPSkinEditorApp::OnOpenSkin() 
{
    string        strSkinFile;
    int            nRet;
/*    CFileDialog ofdlg(true, 0, g_profile.getString("LastOpenSknFile", ""), OFN_FILEMUSTEXIST, "Skin files(*.xml)|*.xml|All files (*.*)|*.*||", m_pMainWnd);

    if (ofdlg.doModal() != IDOK)
        return;

    strSkinFile = ofdlg.GetPathName();
    g_profile.writeString("LastOpenSknFile", strSkinFile.c_str());*/

    CDlgOpenSkin    dlg;

    if (dlg.doModal() != IDOK)
        return;

    CSkinFileXML        *pskinFile;
    vector<string>        vSkinWndNames;
    pskinFile = g_skinFactory.getSkinFile();
    CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);

    // close opened skins.
    CMDIChildWnd *pChild = pFrame->MDIGetActive();
    while (pChild)
    {
        pChild->destroyWindow();
        pChild = pFrame->MDIGetActive();
    }
    g_skinPrj.close();

    nRet = g_skinFactory.OpenSkinEx(dlg.m_strSelSkin.c_str());
    if (nRet != ERR_OK)
    {
        AfxMessageBox(Error2Str(nRet));
        return;
    }

    pskinFile->enumSkinWnd(vSkinWndNames);

    for (int i = 0; i < vSkinWndNames.size(); i++)
    {
        // create a new MDI child window
        CSkinEditorFrame *pSEFrame = (CSkinEditorFrame*)pFrame->CreateNewChild(
            RUNTIME_CLASS(CSkinEditorFrame), IDR_MPSKINTYPE, m_hMDIMenu, m_hMDIAccel);

        pSEFrame->m_wndSkin.openSkin(vSkinWndNames[i].c_str());
        pSEFrame->m_wndSkin.setMainAppWnd(false);
        pSEFrame->setTitle(vSkinWndNames[i].c_str());
        AfxSetWindowText(pSEFrame->m_hWnd, vSkinWndNames[i].c_str());
        pSEFrame->showWindow(SW_MAXIMIZE);
        pSEFrame->onUIObjFocusChanged();
        pSEFrame->updateSkinObjList();
    }

    // open Dynamic command window
    pFrame->CreateNewChild(
            RUNTIME_CLASS(CDynCmdFrame), IDR_DYNCMD, m_hDynCmdMenu, m_hMDIAccel);

    // open Dynamic control window
    pFrame->CreateNewChild(
            RUNTIME_CLASS(CDynCtrlFrame), IDR_DYNCTRL, m_hDynCmdMenu, m_hMDIAccel);

    pFrame->getMDITab()->update();
}

void CMPSkinEditorApp::OnSkinWndNew() 
{
    string        name;
    string        strNamePrefix = "Untitled Window";
    CSkinFileXML *pSkinFile = g_skinFactory.getSkinFile();
    CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);

    name = strNamePrefix;
    for (int i = 1; !pSkinFile->canUseSkinWndName(name.c_str()); i++)
        name = strNamePrefix + stringPrintf(" %d", i).c_str();

    CSkinEditorFrame *pSEFrame = (CSkinEditorFrame*)pFrame->CreateNewChild(
        RUNTIME_CLASS(CSkinEditorFrame), IDR_MPSKINTYPE, m_hMDIMenu, m_hMDIAccel);

    pSEFrame->m_wndSkin.newSkin(name.c_str());
    pSEFrame->m_wndSkin.setMainAppWnd(false);
    pSEFrame->setTitle(name.c_str());
    AfxSetWindowText(pSEFrame->m_hWnd, name.c_str());
    pSEFrame->showWindow(SW_MAXIMIZE);
    pSEFrame->onUIObjFocusChanged();
    pSEFrame->updateSkinObjList();

    pSkinFile->newSkinWndNode(name.c_str());

    pFrame->getMDITab()->update();
}

void CMPSkinEditorApp::OnSkinWndDup() 
{
    string        name;
    string        strNamePrefix = "copy Of ";
    CSkinFileXML *pSkinFile = g_skinFactory.getSkinFile();
    CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
    CMDIChildWnd *child = pFrame->MDIGetActive();

    if (child->isKindOf(&CSkinEditorFrame::classCSkinEditorFrame))
    {
        CSkinEditorFrame *pSEFrameSrc = (CSkinEditorFrame*)child;

        name = strNamePrefix += pSEFrameSrc->m_wndSkin.getSkinWndName();
        for (int i = 1; !pSkinFile->canUseSkinWndName(name.c_str()); i++)
            name = strNamePrefix + stringPrintf(" %d", i).c_str();

        CSkinEditorFrame *pSEFrame = (CSkinEditorFrame*)pFrame->CreateNewChild(
            RUNTIME_CLASS(CSkinEditorFrame), IDR_MPSKINTYPE, m_hMDIMenu, m_hMDIAccel);

        pSEFrame->m_wndSkin.copyFrom(name.c_str(), &(pSEFrameSrc->m_wndSkin));
        pSEFrame->m_wndSkin.setMainAppWnd(false);
        pSEFrame->setTitle(name.c_str());
        AfxSetWindowText(pSEFrame->m_hWnd, name.c_str());
        pSEFrame->showWindow(SW_MAXIMIZE);
        pSEFrame->onUIObjFocusChanged();
        pSEFrame->updateSkinObjList();

        pSkinFile->newSkinWndNode(name.c_str());
    }

    pFrame->getMDITab()->update();
}

void CMPSkinEditorApp::OnSkinWndDel() 
{
    CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
    CMDIChildWnd *child = pFrame->MDIGetActive();

    if (child->isKindOf(&CSkinEditorFrame::classCSkinEditorFrame))
    {
        CSkinEditorFrame *pSEFrameSrc = (CSkinEditorFrame*)child;
        string            name;

        name = pSEFrameSrc->m_wndSkin.getSkinWndName();

        pSEFrameSrc->MDIDestroy();

        g_skinFactory.getSkinFile()->removeSkinWnd(name.c_str());

        pFrame->getMDITab()->update();
    }
}
