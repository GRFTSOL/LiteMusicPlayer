// DlgChooseIDs.cpp : implementation file
//

#include "mpskineditor.h"
#include "DlgChooseIDs.h"
#include "../MPShared/MLCmd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgChooseIDs dialog


CDlgChooseIDs::CDlgChooseIDs(CWnd* pParent /*=nullptr*/)
    : CDialog(CDlgChooseIDs::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDlgChooseIDs)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void CDlgChooseIDs::doDataExchange(CDataExchange* pDX)
{
    CDialog::doDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgChooseIDs)
    DDX_Control(pDX, IDC_LIST, m_listIDs);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgChooseIDs, CDialog)
    //{{AFX_MSG_MAP(CDlgChooseIDs)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgChooseIDs message handlers

void CDlgChooseIDs::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    onOK();
    
    *pResult = 0;
}

void CDlgChooseIDs::onOK() 
{
    char        szText[256] = "";

    POSITION    pos = m_listIDs.GetFirstSelectedItemPosition();
    if (pos)
    {
        int n = m_listIDs.getNextSelectedItem(pos);
        m_listIDs.getItemText(n, 0, szText, CountOf(szText));
        m_strID = szText;
    }
    else
        return;
    
    CDialog::onOK();
}

bool CDlgChooseIDs::onInitDialog() 
{
    CDialog::onInitDialog();
    
    m_listIDs.InsertColumn(0, "ID", LVCFMT_LEFT, 150, 250);
    m_listIDs.InsertColumn(0, "Tool tip", LVCFMT_LEFT, 150, 250);
    m_listIDs.SetExtendedStyle(m_listIDs.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    for (int i =0; g_StringIdMap[i].szId != nullptr; i++)
    {
        int        n = m_listIDs.insertItem(m_listIDs.getItemCount(), g_StringIdMap[i].szId, 0);
        if (g_StringIdMap[i].szToolTip)
            m_listIDs.setItemText(n, 1, g_StringIdMap[i].szToolTip);
        if (strcasecmp(g_StringIdMap[i].szId, m_strID.c_str()) == 0)
            m_listIDs.SetItemState(n, LVIS_SELECTED, LVIS_SELECTED);
    }

    return true;  // return true unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return false
}
