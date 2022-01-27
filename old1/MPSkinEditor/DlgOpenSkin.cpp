// DlgOpenSkin.cpp : implementation file
//

#include "mpskineditor.h"
#include "DlgOpenSkin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgOpenSkin dialog


CDlgOpenSkin::CDlgOpenSkin(CWnd* pParent /*=nullptr*/)
    : CDialog(CDlgOpenSkin::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDlgOpenSkin)
    m_strSkinsRootDir = g_profile.getString("SkinsRootDir", "");
    m_strInfo = "";
    //}}AFX_DATA_INIT
}


void CDlgOpenSkin::doDataExchange(CDataExchange* pDX)
{
    CDialog::doDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgOpenSkin)
    DDX_Control(pDX, IDC_L_SKINS, m_lbSkins);
    DDX_Text(pDX, IDC_E_SKIN_ROOT_DIR, m_strSkinsRootDir);
    DDX_Text(pDX, IDC_E_INFO, m_strInfo);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgOpenSkin, CDialog)
    //{{AFX_MSG_MAP(CDlgOpenSkin)
    ON_BN_CLICKED(IDC_BR_SKIN_ROOT_DIR, OnBrSkinRootDir)
    ON_LBN_DBLCLK(IDC_L_SKINS, OnDblclkLSkins)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgOpenSkin message handlers

bool CDlgOpenSkin::onInitDialog() 
{
    CDialog::onInitDialog();

    if (m_strSkinsRootDir.isEmpty())
    {
        m_strSkinsRootDir = getAppResourceDir();
        m_strSkinsRootDir += "skins\\";
    }
    g_skinFactory.setSkinsRootDir(m_strSkinsRootDir);

    vector<string>        vSkins;

    g_skinFactory.enumAllSkins(vSkins);

    if (vSkins.empty())
    {
        g_skinFactory.setSkinFileName("minilyrics.xml");
        g_skinFactory.enumAllSkins(vSkins);
        if (vSkins.empty())
            g_skinFactory.setSkinFileName("ZikiPlayer.xml");
    }

    for (int i = 0; i < vSkins.size(); i++)
    {
        m_lbSkins.addString(vSkins[i].c_str());
    }
    
    return true;  // return true unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return false
}

void CDlgOpenSkin::onOK() 
{
    if (m_lbSkins.getCount() == 0)
        return;

    int    nSel = m_lbSkins.getCurSel();
    if (nSel == -1)
        return;

    CString        str;
    m_lbSkins.getText(nSel, str);
    m_strSelSkin = (cstr_t)str;

    CDialog::onOK();
}

void CDlgOpenSkin::OnBrSkinRootDir() 
{
    char        szFolder[MAX_PATH];

    strcpy_safe(szFolder, CountOf(szFolder), g_profile.getString("SkinsRootDir", ""));
    if (!browserForFolder(m_hWnd, "Browse skins root dir:", szFolder))
        return;

    m_strSkinsRootDir = szFolder;
    setDlgItemText(IDC_E_SKIN_ROOT_DIR, m_strSkinsRootDir);
    g_profile.writeString("SkinsRootDir", m_strSkinsRootDir);

    g_skinFactory.setSkinsRootDir(m_strSkinsRootDir);

    vector<string>        vSkins;
    g_skinFactory.enumAllSkins(vSkins);

    for (int i = 0; i < vSkins.size(); i++)
    {
        m_lbSkins.addString(vSkins[i].c_str());
    }
}

void CDlgOpenSkin::OnDblclkLSkins() 
{
    onOK();
}
