// PropertyBar.cpp : implementation file
//

#include "MPSkinEditor.h"
#include "PropertyBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertyBar

CSEPropertyListCtrl    *CPropertyBar::ms_pwndProperty = nullptr;

CPropertyBar::CPropertyBar()
{
}

CPropertyBar::~CPropertyBar()
{
}


BEGIN_MESSAGE_MAP(CPropertyBar, CSizingControlBar)
    //{{AFX_MSG_MAP(CPropertyBar)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropertyBar message handlers

int CPropertyBar::onCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CSizingControlBar::onCreate(lpCreateStruct) == -1)
        return -1;

//    loadState("PropertyBar");

    ///////////////////////////////////////
    // create the list box font
    m_ListBoxFont.CreateFont( 14, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
                             "arial");    

    ///////////////////////////////////////
    // create the Control
    ms_pwndProperty = new CSEPropertyListCtrl();
    // ms_pwndProperty->SubclassDlgItem(ID_PROPERTYLIST, this);
    CRect    rc;
    getClientRect(&rc);
    ms_pwndProperty->create(WS_CHILD|WS_BORDER|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL|WS_HSCROLL|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED, rc, this, 1000);
    
    ///////////////////////////////////////
    // set some attributes (Completly Optional)
    ms_pwndProperty->setFont(&m_ListBoxFont);
/*    ms_pwndProperty->setBkColor(RGB(214,227,239));
    ms_pwndProperty->setTextColor(RGB(74,109,132));
    ms_pwndProperty->setTextHighlightColor(RGB(80,80,80));
    ms_pwndProperty->setHighlightColor(RGB(246,246,220));
    ms_pwndProperty->setPropertyBkColor(RGB(255,255,255));
    ms_pwndProperty->setPropertyTextColor(RGB(0,0,192));
    ms_pwndProperty->setBoldSelection(true);
    ms_pwndProperty->setLineStyle(RGB(74,109,132), PS_SOLID);*/

/*
    if (!m_wndChild.create("Paste",
        WS_CHILD|WS_VISIBLE,
        CRect(0,0,0,0), this, ID_EDIT_PASTE))
        return -1;
*/
/*
    CRect    rc;
    getClientRect(&rc);
    if (!m_wndChild.create(LBS_HASSTRINGS | LBS_MULTICOLUMN | LBS_NOTIFY | LBS_OWNERDRAWFIXED, rc, this, ID_EDIT_PASTE))
        return -1;
    m_wndChild.showWindow(SW_SHOW);

    // older versions of Windows* (NT 3.51 for instance)
    // fail with DEFAULT_GUI_FONT

    if (!m_font.CreateStockObject(DEFAULT_GUI_FONT))
        if (!m_font.CreatePointFont(80, "MS Sans Serif"))
            return -1;

    m_wndChild.setFont(&m_font);
    ms_pwndProperty = &m_wndChild;
*/

    ms_pwndProperty = ms_pwndProperty;

    return 0;
}

void CPropertyBar::onUpdateCmdUI(CFrameWnd* pTarget, bool bDisableIfNoHndler)
{
    CSizingControlBar::onUpdateCmdUI(pTarget, bDisableIfNoHndler);

    UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

void CPropertyBar::onSize(uint32_t nType, int cx, int cy) 
{
    CSizingControlBar::onSize(nType, cx, cy);

    // TODO: add your message handler code here
    CRect rc;
    getClientRect(rc);

    if (ms_pwndProperty)
        ms_pwndProperty->moveWindow(rc);
//    m_wndChild.moveWindow(rc);
}

void CPropertyBar::onDestroy() 
{
    CSizingControlBar::onDestroy();
    
    ms_pwndProperty = nullptr;
}
