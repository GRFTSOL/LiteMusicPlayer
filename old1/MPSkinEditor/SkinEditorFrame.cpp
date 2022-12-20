// SkinEditorFrame.cpp: implementation of the CSkinEditorFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "MPSkinEditor.h"
#include "MainFrm.h"
#include "SkinEditorFrame.h"

#include "PropertyBar.h"
#include "ObjListBar.h"

CSkinEditorFrame        *g_pActiveFrameOld = nullptr;

class CUIObjClipboard
{
public:
    CUIObjClipboard()
    {
    }

    void setClippedObj(CUIObject *pObj)
    {
        m_objProperties.clear();
        if (pObj)
        {
            pObj->enumProperties(m_objProperties);
            m_strClassName = pObj->getClassName();
        }
        else
        {
            m_strClassName.resize(0);
        }
    }

    bool isEmpty() { return m_strClassName.empty(); }

    CUIObject *fromClipboard(CSkinWnd *pSkin)
    {
        CUIObject    *pObj;
        VecStrings        vStr;

        pObj = pSkin->getSkinFactory()->createUIObject(pSkin, m_strClassName.c_str());
        if (pObj)
        {
            m_objProperties.toUIObjProperties(vStr);

            pObj->setProperties(vStr);
        }
        return pObj;
    }

protected:
    string                m_strClassName;
    CUIObjProperties    m_objProperties;

};

CUIObjClipboard        g_skinObjClipboard;

/////////////////////////////////////////////////////////////////////////////
// CSkinEditorFrame

IMPLEMENT_DYNCREATE(CSkinEditorFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CSkinEditorFrame, CMDIChildWnd)
    //{{AFX_MSG_MAP(CSkinEditorFrame)
    ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
    ON_WM_SETFOCUS()
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_DESTROY()
    ON_COMMAND(IDC_INSERT_BTN, OnInsertBtn)
    ON_COMMAND(IDC_PROPERTY, OnProperty)
    ON_COMMAND(IDC_DELETE, OnDelete)
    ON_COMMAND(IDC_INSERT_FOCUS_BTN, OnInsertFocusBtn)
    ON_COMMAND(IDC_INSERT_IMG, OnInsertImg)
    ON_COMMAND(IDC_INSERT_FOCUS_IMG, OnInsertFocusImg)
    ON_COMMAND(IDC_INSERT_XSCALE_IMG, OnInsertXscaleImg)
    ON_COMMAND(IDC_INSERT_YSCALE_IMG, OnInsertYscaleImg)
    ON_COMMAND(IDC_INSERT_VSCROLLBAR, OnInsertVscrollbar)
    ON_WM_ERASEBKGND()
    ON_COMMAND(IDC_PROPERTY_SAVE, OnPropertySave)
    ON_WM_LBUTTONDOWN()
    ON_COMMAND(IDC_ROOM_IN, OnRoomIn)
    ON_COMMAND(IDC_ROOM_OUT, OnRoomOut)
    ON_WM_KILLFOCUS()
    ON_COMMAND(IDC_UPDATE_SKIN_OBJ_LIST, OnUpdateSkinObjList)
    ON_COMMAND(IDC_SKIN_OBJ_LIST_SEL_CHANGED, OnSkinObjListSelChanged)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_CUT, OnEditCut)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
    ON_COMMAND(IDC_REDO, OnRedo)
    ON_WM_SYSCOMMAND()
    ON_WM_CHILDACTIVATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkinEditorFrame construction/destruction

CSkinEditorFrame::CSkinEditorFrame()
{
    // TODO: add member initialization code here
    
}

CSkinEditorFrame::~CSkinEditorFrame()
{
}


void CSkinEditorFrame::onSkinWndNameChanged()
{
    setTitle(m_wndSkin.getSkinWndName());
    AfxSetWindowText(m_hWnd, m_wndSkin.getSkinWndName());

    CMainFrame    *pMainFrm = (CMainFrame *)AfxGetMainWnd();

    pMainFrm->getMDITab()->update();
}

HTREEITEM findItemInSkinUIObjTreeCtrl(CTreeCtrl *pTree, HTREEITEM hParent, uint32_t dwItemData)
{
    HTREEITEM    hChild;

    hChild = pTree->GetChildItem(hParent);
    while (hChild)
    {
        if (pTree->getItemData(hChild) == dwItemData)
            return hChild;
        if (pTree->GetChildItem(hChild))
        {
            HTREEITEM    hTemp;
            hTemp = findItemInSkinUIObjTreeCtrl(pTree, hChild, dwItemData);
            if (hTemp)
                return hTemp;
        }

        hChild = pTree->GetNextSiblingItem(hChild);
    }

    return nullptr;
}

// when the UIObject name is changed, update the tree control.
void CSkinEditorFrame::onUIObjNameChanged(CUIObject *pObj)
{
    CTreeCtrl        *pTree = getSkinUIObjTreeCtrl();
    if (!pTree)
        return;

    HTREEITEM    hItem = findItemInSkinUIObjTreeCtrl(pTree, TVI_ROOT, (uint32_t)pObj);
    if (hItem)
    {
        pTree->setItemText(hItem, stringPrintf("%s (%s)", pObj->m_strName.c_str(), pObj->getClassName()).c_str());
    }
}

void CSkinEditorFrame::onUIObjectListChanged()
{
    updateSkinObjList();
}

void CSkinEditorFrame::onUIObjFocusChanged()
{
    updateToPropertyListCtrl();
}

void CSkinEditorFrame::onFocusUIObjPropertyChanged(cstr_t szPropertyName, cstr_t szNewValue)
{
    if (!CPropertyBar::ms_pwndProperty)
        return;

    CUIObject                *pObjFocus;
    pObjFocus = m_wndSkin.getFocusUIObj();

    if (!g_seSkinPropertyAdapter.isSrcChanged(&m_wndSkin, pObjFocus))
    {
        CPropertyBar::ms_pwndProperty->setProperty(szPropertyName, szNewValue);
    }
}

void CSkinEditorFrame::trackPopupMenu(HMENU hMenu, int x, int y)
{
    ::trackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN, x, y, 0, m_hWnd, nullptr);
}

bool CSkinEditorFrame::preCreateWindow(CREATESTRUCT& cs)
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
// CSkinEditorFrame diagnostics

#ifdef _DEBUG
void CSkinEditorFrame::assertValid() const
{
    CMDIChildWnd::assertValid();
}

void CSkinEditorFrame::dump(CDumpContext& dc) const
{
    CMDIChildWnd::dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSkinEditorFrame message handlers
void CSkinEditorFrame::OnFileClose() 
{
    // To close the frame, just send a WM_CLOSE, which is the equivalent
    // choosing close from the system menu.

    sendMessage(WM_CLOSE);
}

int CSkinEditorFrame::onCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CMDIChildWnd::onCreate(lpCreateStruct) == -1)
        return -1;

    SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)loadCursor(nullptr, MAKEINTRESOURCE(IDC_ARROW)));

//    SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)), true);
//    SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)), false);
    SetClassLong(m_hWnd, GCL_HICONSM, (LONG)LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)));
    SetClassLong(m_hWnd, GCL_HICON, (LONG)LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)));

    m_wndSkin.setNotify(this);

    m_wndSkin.create("MPSkin", "MPSkin", &g_skinFactory, "", Window::fromHandle(m_hWnd));

    m_wndSkin.moveWindow(5, 5, 640, 480);

    g_skinPrj.onCreateSkinDataItem(this);

    return 0;
}

void CSkinEditorFrame::onPaint() 
{
    CPaintDC dc(this); // device context for painting

    CRect        rc;

    getClientRect(&rc);

    dc.FillSolidRect(&rc, RGB(255, 255, 255));
}

void CSkinEditorFrame::onDestroy() 
{
    g_skinPrj.onDestroySkinDataItem(this);

    m_wndSkin.destroy();

    CMDIChildWnd::onDestroy();
}

void CSkinEditorFrame::updatePropertyItem(cstr_t szPropName, cstr_t szValue)
{
    if (!CPropertyBar::ms_pwndProperty)
        return;

    CPropertyBar::ms_pwndProperty->setProperty(szPropName, szValue);
}

bool selectItemInPropertyTreeCtrl(CTreeCtrl *pTree, HTREEITEM hParent, CUIObject *pObjFocus)
{
    HTREEITEM    hChild = nullptr;

    hChild = pTree->GetChildItem(hParent);
    while (hChild)
    {
        if (pTree->getItemData(hChild) == (uint32_t)pObjFocus)
        {
            pTree->SelectItem(hChild);

            return true;
        }
        if (selectItemInPropertyTreeCtrl(pTree, hChild, pObjFocus))
            return true;

        hChild = pTree->GetNextSiblingItem(hChild);
    }

    return false;
}

void CSkinEditorFrame::updateToPropertyListCtrl()
{
    if (!CPropertyBar::ms_pwndProperty)
        return;

    CUIObject                *pObjFocus;
    CUIObjProperties        properties;
    CTreeCtrl                *pTree = getSkinUIObjTreeCtrl();

    pObjFocus = m_wndSkin.getFocusUIObj();

    if (pTree)
    {
        // Select focus UIObj in Skin Object tree ctrl
        HTREEITEM    hItem = pTree->GetSelectedItem();
        if (hItem == nullptr || pTree->getItemData(hItem) != (uint32_t)pObjFocus)
        {
            pTree->Select(hItem, 0);

            selectItemInPropertyTreeCtrl(pTree, TVI_ROOT, m_wndSkin.getFocusUIObj());
        }
    }

    g_seSkinPropertyAdapter.setSrc(&m_wndSkin, pObjFocus, m_hWnd);

    if (pObjFocus)
    {
        pObjFocus->enumProperties(properties);
    }
    else
    {
        m_wndSkin.enumProperties(properties);
    }

    CPropertyBar::ms_pwndProperty->resetContent();

    ///////////////////////////////////////
    // add the properties
    CPropertyBar::ms_pwndProperty->addProperties(properties);
}

void CSkinEditorFrame::setPropertiesToUIObj()
{
    if (!CPropertyBar::ms_pwndProperty)
        return;

    CUIObject        *pObjFocus;
    vector<string>    properties;

    pObjFocus = m_wndSkin.getFocusUIObj();

    if (g_seSkinPropertyAdapter.isSrcChanged(&m_wndSkin, pObjFocus))
        return;

    CPropertyBar::ms_pwndProperty->getProperties(properties);

    if (pObjFocus)
    {
        pObjFocus->setProperties(properties);

        m_wndSkin.RecalculateSizePos();

        m_wndSkin.invalidateRect(nullptr, true);
    }
    else
    {
        // set skin's properties.
        for (uint32_t i = 0; i < properties.size(); i += 2)
            m_wndSkin.setProperty(properties[i].c_str(), properties[i + 1].c_str());
    }
}

void CSkinEditorFrame::OnProperty() 
{
    updateToPropertyListCtrl();
}

void CSkinEditorFrame::insertUIObj(cstr_t szClassName)
{
    CUIObject *pObj = g_skinFactory.createUIObject(&m_wndSkin, szClassName);
    m_wndSkin.insertUIObject(pObj);
}

void CSkinEditorFrame::OnInsertBtn() 
{
    insertUIObj(CSkinButton::className());
}

void CSkinEditorFrame::OnInsertFocusBtn() 
{
    insertUIObj(CSkinActiveButton::className());
}

void CSkinEditorFrame::OnInsertImg() 
{
    insertUIObj(CSkinImage::className());
}

void CSkinEditorFrame::OnInsertFocusImg() 
{
    insertUIObj(CSkinActiveImage::className());
}

void CSkinEditorFrame::OnInsertXscaleImg() 
{
    insertUIObj(CSkinXScaleImage::className());
}

void CSkinEditorFrame::OnInsertYscaleImg() 
{
    insertUIObj(CSkinYScaleImage::className());
}

void CSkinEditorFrame::OnInsertVscrollbar() 
{
    insertUIObj(CSkinVScrollBar::className());
}

void CSkinEditorFrame::onSetFocus(CWnd* pOldWnd) 
{
    ::setFocus(m_wndSkin.getHandle());
}


bool CSkinEditorFrame::OnEraseBkgnd(CDC* pDC) 
{
    CRect    rc;
    getClientRect(&rc);
    pDC->FillSolidRect(0, 0, rc.width(), rc.height(), GetSysColor(COLOR_3DSHADOW));
    
    return true;
    // return CMDIChildWnd::OnEraseBkgnd(pDC);
}

void CSkinEditorFrame::OnPropertySave() 
{
    setPropertiesToUIObj();
}

void CSkinEditorFrame::onLButtonDown(uint32_t nFlags, CPoint point) 
{
    // setFocus();
    m_wndSkin.setNOFocusUIObj();

    OnProperty();
    
    CMDIChildWnd::onLButtonDown(nFlags, point);
}

LRESULT CSkinEditorFrame::windowProc(uint32_t message, WPARAM wParam, LPARAM lParam) 
{
    if (message == SNM_UPDATEPROPERTYITEM)
    {
        updatePropertyItem((cstr_t)wParam, (cstr_t)lParam);
    }
    
    return CMDIChildWnd::windowProc(message, wParam, lParam);
}

void CSkinEditorFrame::OnRoomIn() 
{
    m_wndSkin.zoomIn();
}

void CSkinEditorFrame::OnRoomOut() 
{
    m_wndSkin.zoomOut();
}

void CSkinEditorFrame::onKillFocus(CWnd* pNewWnd) 
{
    CMDIChildWnd::onKillFocus(pNewWnd);

/*    CPropertyBar::ms_pObjFocus = nullptr;
    g_seSkinPropertyAdapter.m_hWndCommand = nullptr;
    CPropertyBar::ms_pwndProperty->resetContent();*/
}

void addSkinObjToTreeCtrl(CTreeCtrl *pTree, HTREEITEM hParent, CSkinContainer *pConainter)
{
    if (!pConainter)
        return;

    HTREEITEM        hChild = nullptr;
    int        nCount = pConainter->getChildrenCount();

    for (int i = 0; i < nCount; i++)
    {
        CUIObject    *pObj = pConainter->getChildByIndex(i);
        if (pObj)
        {
            hChild = pTree->insertItem(stringPrintf("%s (%s)", pObj->m_strName.c_str(), pObj->getClassName()).c_str(), 
                hParent, TVI_LAST);
            pTree->setItemData(hChild, (uint32_t)pObj);

            if (pObj->isContainer())
            {
                addSkinObjToTreeCtrl(pTree, hChild, pObj->getContainerIf());
            }
        }
    }
    pTree->Expand(hParent, TVE_EXPAND);
}

void CSkinEditorFrame::updateSkinObjList()
{
    CTreeCtrl        *pTree = getSkinUIObjTreeCtrl();

    if (!pTree)
        return;

    pTree->deleteAllItems();

    CSkinContainer    *pConainter = m_wndSkin.getRootContainer();

    addSkinObjToTreeCtrl(pTree, TVI_ROOT, pConainter);
}

void CSkinEditorFrame::OnUpdateSkinObjList() 
{
    updateSkinObjList();
}

void CSkinEditorFrame::OnSkinObjListSelChanged() 
{
    CTreeCtrl        *pTree = getSkinUIObjTreeCtrl();
    if (!pTree)
        return;

    HTREEITEM        hSel = pTree->GetSelectedItem();
    uint32_t            dwSelData = pTree->getItemData(hSel);
    CUIObject        *pObj = m_wndSkin.getFocusUIObj();

    if (dwSelData == (uint32_t)pObj)
        return;

    // selected UIObject changed, change the focus to it.
    if (dwSelData && m_wndSkin.isUIObjectExist((CUIObject*)dwSelData))
    {
        m_wndSkin.setNOFocusUIObj();

        pObj = (CUIObject *)dwSelData;
        pObj->setFocus();
    }

    OnProperty();
}

void CSkinEditorFrame::OnEditCopy() 
{
    CUIObject        *pObjFocus;
    pObjFocus = m_wndSkin.getFocusUIObj();
    if (pObjFocus)
        g_skinObjClipboard.setClippedObj(pObjFocus);
}

void CSkinEditorFrame::OnEditCut() 
{
    OnEditCopy();
    OnDelete();
}

void CSkinEditorFrame::OnDelete() 
{
    m_wndSkin.removeFocusUIObject();
}

void CSkinEditorFrame::OnEditPaste() 
{
    CUIObject    *pObj = g_skinObjClipboard.fromClipboard(&m_wndSkin);
    m_wndSkin.insertUIObject(pObj);
}

void CSkinEditorFrame::OnEditUndo() 
{
    // TODO: add your command handler code here
    
}

void CSkinEditorFrame::OnRedo() 
{
    // TODO: add your command handler code here
    
}

void CSkinEditorFrame::onSave()
{
    CXMLWriter        xmlStream;
    CSimpleXML        xml;

    m_wndSkin.toXML(xmlStream);
    if (xml.parseData((void *)xmlStream.getData(), xmlStream.getLength()))
    {
        g_skinFactory.getSkinFile()->setSkinWndNode(m_wndSkin.getSkinWndName(), xml.m_pRoot);
        xml.m_pRoot = nullptr;
    }
    else
    {
        saveDataAsFile("C:\\1.xml", xmlStream.getData(), xmlStream.getLength());
        ERR_LOG0("parse Skin Wnd Node xml FAILED.");
    }
}

void CSkinEditorFrame::OnSysCommand(uint32_t nID, LPARAM lParam) 
{
    if (nID == SC_CLOSE)
        return;
    
    CMDIChildWnd::OnSysCommand(nID, lParam);
}

void CSkinEditorFrame::OnChildActivate() 
{
    CMDIChildWnd::OnChildActivate();

    if (g_seSkinPropertyAdapter.isSrcChanged(&m_wndSkin, m_wndSkin.getFocusUIObj()))
        OnProperty();

    if (g_pActiveFrameOld != this)
    {
        updateSkinObjList();
    }
}
