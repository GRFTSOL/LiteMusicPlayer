// DlgImageProperty.cpp : implementation file
//

#include "MPSkinEditor.h"
#include "DlgImageProperty.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgImageProperty dialog


CDlgImageProperty::CDlgImageProperty(CWnd* pParent /*=nullptr*/)
    : CDialog(CDlgImageProperty::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDlgImageProperty)
    m_ncx = 0;
    m_ncy = 0;
    m_nx = 0;
    m_ny = 0;
    m_nStretchStart = 0;
    m_nStretchStart2 = 0;
    m_nStretchEnd = 0;
    m_nStretchEnd2 = 0;
    //}}AFX_DATA_INIT
}


void CDlgImageProperty::doDataExchange(CDataExchange* pDX)
{
    CDialog::doDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgImageProperty)
    DDX_Control(pDX, IDC_IMAGE, m_wndImageRectSel);
    DDX_Control(pDX, IDC_CB_IMAGE, m_cbImage);
    DDX_Text(pDX, IDC_E_CX, m_ncx);
    DDX_Text(pDX, IDC_E_CY, m_ncy);
    DDX_Text(pDX, IDC_E_X, m_nx);
    DDX_Text(pDX, IDC_E_Y, m_ny);
    DDX_Text(pDX, IDC_E_STRETCH_START, m_nStretchStart);
    DDX_Text(pDX, IDC_E_STRETCH_START2, m_nStretchStart2);
    DDX_Text(pDX, IDC_E_STRETCH_END, m_nStretchEnd);
    DDX_Text(pDX, IDC_E_STRETCH_END2, m_nStretchEnd2);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgImageProperty, CDialog)
    //{{AFX_MSG_MAP(CDlgImageProperty)
    ON_CBN_SELCHANGE(IDC_CB_IMAGE, OnSelchangeCbImage)
    ON_BN_CLICKED(IDC_UPDATE_RC, OnUpdateRc)
    ON_EN_CHANGE(IDC_E_CX, OnChangeECx)
    ON_EN_CHANGE(IDC_E_Y, OnChangeEY)
    ON_EN_CHANGE(IDC_E_X, OnChangeEX)
    ON_EN_CHANGE(IDC_E_CY, OnChangeECy)
    ON_BN_CLICKED(IDC_ROOM_IN, OnRoomIn)
    ON_BN_CLICKED(IDC_ROOM_OUT, OnRoomOut)
    ON_WM_DESTROY()
    ON_EN_CHANGE(IDC_E_STRETCH_START, OnChangeEStretchStart)
    ON_EN_CHANGE(IDC_E_STRETCH_END, OnChangeEStretchEnd)
    ON_EN_CHANGE(IDC_E_STRETCH_START2, OnChangeEStretchStart2)
    ON_EN_CHANGE(IDC_E_STRETCH_END2, OnChangeEStretchEnd2)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgImageProperty message handlers

bool CDlgImageProperty::onInitDialog() 
{
    CDialog::onInitDialog();

    CString        str;
    getWindowText(str);
    str += ": ";
    str += m_strName.c_str();
    setWindowText(str);
    
    vector<string>        vFiles;

    vFiles.push_back("");
    g_skinFactory.getResourceMgr()->enumFiles("*.bmp", vFiles, false);
    g_skinFactory.getResourceMgr()->enumFiles("*.png", vFiles, false);

    for (int i = 0; i < (int)vFiles.size(); i++)
    {
        m_cbImage.addString(vFiles[i].c_str());
    }
    m_cbImage.SelectString(-1, m_strFile.c_str());

    m_wndImageRectSel.setImageFile(m_strFile.c_str());
    str.Format("width: %d, height: %d", m_wndImageRectSel.getImageWidth(), m_wndImageRectSel.getImageHeight());
    setDlgItemText(IDC_S_IMG_SIZE, str);
    setDlgItemText(IDC_S_CURSOR, "");

    if (m_vNameRects.size() >= 2)
    {
        // rect image
        CRect        rcObj;
        long        x = 0, y = 0, cx = 0, cy = 0;

        getRectValue(m_vNameRects[1].c_str(), x, y, cx, cy);
        m_nx = x;
        m_ny = y;
        m_ncx = cx;
        m_ncy = cy;
        rcObj.setLTRB(m_nx, m_ny, m_nx + m_ncx, m_ny + m_ncy);

        ::enableDlgItem(m_hWnd, IDC_E_X, true);
        ::enableDlgItem(m_hWnd, IDC_E_Y, true);
        ::enableDlgItem(m_hWnd, IDC_E_CX, true);
        ::enableDlgItem(m_hWnd, IDC_E_CY, true);
        if (m_vNameRects.size() == 2)
        {
            m_wndImageRectSel.initAsRectImg(rcObj);
        }
        else if (m_vNameRects.size() >= 6)
        {
            ::enableDlgItem(m_hWnd, IDC_E_STRETCH_START, true);
            ::enableDlgItem(m_hWnd, IDC_E_STRETCH_END, true);

            m_nStretchStart = atoi(m_vNameRects[3].c_str());
            m_nStretchEnd = atoi(m_vNameRects[5].c_str());

            if (m_vNameRects.size() == 6)
            {
                // horz or vert stretched image
                if (strcasecmp(m_vNameRects[2].c_str(), "StretchStartX") == 0)
                {
                    // horz 
                    m_wndImageRectSel.initAsHorzStretchImg(rcObj, m_nStretchStart, m_nStretchEnd);
                }
                else
                {
                    // vert
                    m_wndImageRectSel.initAsVertStretchImg(rcObj, m_nStretchStart, m_nStretchEnd);
                }
            }
            else if (m_vNameRects.size() == 10)
            {
                // caption image
                ::enableDlgItem(m_hWnd, IDC_E_STRETCH_START2, true);
                ::enableDlgItem(m_hWnd, IDC_E_STRETCH_END2, true);

                m_nStretchStart2 = atoi(m_vNameRects[7].c_str());
                m_nStretchEnd2 = atoi(m_vNameRects[9].c_str());
                m_wndImageRectSel.initAsCaptionImg(rcObj, m_nStretchStart, m_nStretchEnd, m_nStretchStart2, m_nStretchEnd2);
            }
            else
                assert(0);
        }

        UpdateData(false);
    }

    m_wndHelper.init(m_hWnd);
    m_wndHelper.add(IDC_IMAGE, true, true, false, false);
    m_wndHelper.addAlignBottom(IDC_ROOM_OUT, true);
    m_wndHelper.addAlignBottom(IDC_ROOM_IN, true);
    m_wndHelper.addAlignBottom(IDC_S_CURSOR, true);
    m_wndHelper.addAlignBottom(IDC_S_IMG_SIZE, true);
    m_wndHelper.addAlignBottom(IDC_S_FILE, true);
    m_wndHelper.addAlignBottom(IDC_CB_IMAGE, true);
    m_wndHelper.addAlignBottom(IDC_S_X, true);
    m_wndHelper.addAlignBottom(IDC_E_X, true);
    m_wndHelper.addAlignBottom(IDC_S_CX, true);
    m_wndHelper.addAlignBottom(IDC_E_CX, true);
    m_wndHelper.addAlignBottom(IDC_S_Y, true);
    m_wndHelper.addAlignBottom(IDC_E_Y, true);
    m_wndHelper.addAlignBottom(IDC_S_CY, true);
    m_wndHelper.addAlignBottom(IDC_E_CY, true);
    m_wndHelper.addAlignBottom(IDC_S_SSX, true);
    m_wndHelper.addAlignBottom(IDC_E_STRETCH_START, true);
    m_wndHelper.addAlignBottom(IDC_S_SEX, true);
    m_wndHelper.addAlignBottom(IDC_E_STRETCH_END, true);
    m_wndHelper.addAlignBottom(IDC_S_SSX2, true);
    m_wndHelper.addAlignBottom(IDC_E_STRETCH_START2, true);
    m_wndHelper.addAlignBottom(IDC_S_SEX2, true);
    m_wndHelper.addAlignBottom(IDC_E_STRETCH_END2, true);
    m_wndHelper.addAlignBottom(IDOK, true);
    m_wndHelper.addAlignBottom(IDCANCEL, true);

    return true;  // return true unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return false
}

void CDlgImageProperty::OnSelchangeCbImage() 
{
    int        nSel = m_cbImage.getCurSel();
    CString    str;

    m_cbImage.GetLBText(nSel, str);
    m_strFile = str;
    m_wndImageRectSel.setImageFile(m_strFile.c_str());

    str.Format("width: %d, height: %d", m_wndImageRectSel.getImageWidth(), m_wndImageRectSel.getImageHeight());
    setDlgItemText(IDC_S_IMG_SIZE, str);
}

void CDlgImageProperty::OnUpdateRc() 
{
    CRect        rcObj;

    m_wndImageRectSel.getRect(rcObj);
    m_nx = rcObj.left;
    m_ny = rcObj.top;
    m_ncx = rcObj.width();
    m_ncy = rcObj.height();

    m_nStretchStart = m_wndImageRectSel.getStretchStart();
    m_nStretchEnd = m_wndImageRectSel.getStretchEnd();
    m_nStretchStart2 = m_wndImageRectSel.getStretchStart2();
    m_nStretchEnd2 = m_wndImageRectSel.getStretchEnd2();

    UpdateData(false);
}

void CDlgImageProperty::OnChangeEY() 
{
    UpdateData();

    m_wndImageRectSel.setRectY(m_ny);
    m_wndImageRectSel.invalidate();
}

void CDlgImageProperty::OnChangeEX() 
{
    UpdateData();

    m_wndImageRectSel.setRectX(m_nx);
    m_wndImageRectSel.invalidate();
}

void CDlgImageProperty::OnChangeECx() 
{
    UpdateData();

    m_wndImageRectSel.setRectCx(m_ncx);
    m_wndImageRectSel.invalidate();
}

void CDlgImageProperty::OnChangeECy() 
{
    UpdateData();

    m_wndImageRectSel.setRectCy(m_ncy);
    m_wndImageRectSel.invalidate();
}

void CDlgImageProperty::OnChangeEStretchStart() 
{
    UpdateData();

    m_wndImageRectSel.setStretchStart(m_nStretchStart);
    m_wndImageRectSel.invalidate();
}

void CDlgImageProperty::OnChangeEStretchEnd() 
{
    UpdateData();

    m_wndImageRectSel.setStretchStart(m_nStretchEnd);
    m_wndImageRectSel.invalidate();
}

void CDlgImageProperty::OnChangeEStretchStart2() 
{
    UpdateData();

    m_wndImageRectSel.setStretchStart(m_nStretchStart2);
    m_wndImageRectSel.invalidate();
}

void CDlgImageProperty::OnChangeEStretchEnd2() 
{
    UpdateData();

    m_wndImageRectSel.setStretchStart(m_nStretchEnd2);
    m_wndImageRectSel.invalidate();
}

void CDlgImageProperty::OnRoomIn() 
{
    if (m_wndImageRectSel.getScale() < 4)
    {
        m_wndImageRectSel.setScale(m_wndImageRectSel.getScale() + 1);
        m_wndImageRectSel.invalidate();
    }
}

void CDlgImageProperty::OnRoomOut() 
{
    if (m_wndImageRectSel.getScale() > 1)
    {
        m_wndImageRectSel.setScale(m_wndImageRectSel.getScale() - 1);
        m_wndImageRectSel.invalidate();
    }
}

void CDlgImageProperty::onDestroy() 
{
    CDialog::onDestroy();
}

void CDlgImageProperty::onOK() 
{
    char        szValue[256];

    if (m_vNameRects.size() >= 2)
    {
        // rect image
        sprintf(szValue, "%d,%d,%d,%d", m_nx, m_ny, m_ncx, m_ncy);
        m_vNameRects[1] = szValue;

        if (m_vNameRects.size() >= 6)
        {
            sprintf(szValue, "%d", m_nStretchStart);
            m_vNameRects[3] = szValue;

            sprintf(szValue, "%d", m_nStretchEnd);
            m_vNameRects[5] = szValue;

            if (m_vNameRects.size() == 10)
            {
                // caption image
                sprintf(szValue, "%d", m_nStretchStart2);
                m_vNameRects[7] = szValue;

                sprintf(szValue, "%d", m_nStretchEnd2);
                m_vNameRects[9] = szValue;
            }
            else
                assert(0);
        }
    }

    CDialog::onOK();
}

void CDlgImageProperty::onSize(uint32_t nType, int cx, int cy) 
{
    CDialog::onSize(nType, cx, cy);
    
    m_wndHelper.onResize();
}
