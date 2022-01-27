// SESkinWnd.cpp: implementation of the CSESkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "MPSkinEditor.h"
#include "SESkinWnd.h"
#include "UIEditObject.h"
#include "SEPropertyListCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSESkinWnd::CSESkinWnd()
{
    m_WndDrag.enableDrag(false);

    m_hMenuContext = loadMenu(getAppInstance(), MAKEINTRESOURCE(IDR_CONTEXT));

    m_dbScale = 1.0;

    m_pNotify = nullptr;

    m_bEnableVolumeSwitch = false;
    m_bIndividualProperties = false;

    m_bInEditorMode = true;

    m_bEnableTransparentFeatureEditing = true;
    m_bTranslucencyLayeredEditing = false;
}

CSESkinWnd::~CSESkinWnd()
{

}

int CSESkinWnd::create(cstr_t szClassName, cstr_t szCaption, CSkinFactory *pSkinFactory, cstr_t szSkinWndName, Window *pWndParent)
{
    CRect        rcWnd;

    m_pSkinFactory = pSkinFactory;
    m_strSkinWndName = szSkinWndName;

    if (!createEx(szClassName, szCaption,
        rcWnd.left, rcWnd.top, rcWnd.width(), rcWnd.height(),
        pWndParent,
        WS_CHILD | DS_NOIDLEMSG | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_THICKFRAME | WS_MAXIMIZEBOX))
        return ERR_FALSE;

    return ERR_OK;
}

void CSESkinWnd::newSkin(cstr_t szSkinWndName)
{
    closeSkin();

    m_strSkinWndName = szSkinWndName;
}

int CSESkinWnd::openSkin(cstr_t szSkinWndName)
{
    return CSkinWnd::openSkin(szSkinWndName);
}

int CSESkinWnd::copyFrom(cstr_t szSkinWndName, CSESkinWnd *pSkinWndSrc)
{
    CUIObjProperties    properties;
    VecStrings                vStr;
    int                    i;

    closeSkin();

    // copy properties
    pSkinWndSrc->enumProperties(properties);
    properties.toUIObjProperties(vStr);
    for (i = 0; i < (int)vStr.size(); i += 2)
        setProperty(vStr[i].c_str(), vStr[i + 1].c_str());

    m_strSkinWndName = szSkinWndName;

    RecalculateSizePos();

    invalidateRect();

    return ERR_OK;
}

// COMMENT:
//        当加载完SKIN后，调用此函数进行一些设置工作
//        1)    设置子控件的焦点
void CSESkinWnd::onSkinLoaded()
{
    CSkinWnd::onSkinLoaded();

    ::moveWindow(m_hWnd, 5, 5, int(m_rcReal.width()), int(m_rcReal.height()), true);
// 
//     if (m_SizeObj.GetEnableResize())
//     {
//         moveWindow(m_rcReal);
//     }
}

bool containerIsUIObjExist(CSkinContainer *pContainer, CUIObject *pObj)
{
    CUIObject    *p;
    int            n;

    n = pContainer->getChildrenCount();
    for (int i = 0; i < n; i++)
    {
        p = pContainer->getChildByIndex(i);
        if (p == pObj)
            return true;
        else if (p->isContainer())
        {
            if (containerIsUIObjExist(p->getContainerIf(), pObj))
                return true;
        }
    }

    return false;
}

bool CSESkinWnd::isUIObjectExist(CUIObject *pObj)
{
    return containerIsUIObjExist(&m_rootConainter, pObj);
}

int CSESkinWnd::insertUIObject(CUIObject *pObj)
{
    CSkinContainer        *pContainer;
    CUIObject            *pObjFocus;

    if (!pObj)
        return ERR_FALSE;

    pObjFocus = getFocusUIObj();
    if (pObjFocus)
        pContainer = pObjFocus->m_pContainer;
    else
        pContainer = &m_rootConainter;

    if (isEmptyString(pObj->m_formLeft.getFormula()))
    {
        pObj->setProperty(SZ_PN_LEFT, "10");
        pObj->setProperty(SZ_PN_TOP, "10");
        pObj->setProperty(SZ_PN_WIDTH, "30");
        pObj->setProperty(SZ_PN_HEIGHT, "15");
        pObj->m_id = 0;
    }
    pObj->m_pContainer = pContainer;
    pObj->m_pSkin = this;

    pContainer->addUIObject(pObj);

    RecalculateSizePos();

    pObj->setFocus();

    invalidateRect(nullptr, true);

    m_pNotify->onUIObjectListChanged();
    m_pNotify->onUIObjFocusChanged();

    return ERR_OK;
}


void CSESkinWnd::setNOFocusUIObj()
{
    m_itFocusUIObject.End();
}


bool CSESkinWnd::removeFocusUIObject()
{
    CUIObject    *pObj = getFocusUIObj();
    if (!pObj)
        return false;

    pObj->m_pContainer->removeUIObject(pObj, true);

    m_pNotify->onUIObjectListChanged();

    invalidateRect(nullptr, true);
    return true;
}

void CSESkinWnd::onKeyDown(uint32_t nChar, uint32_t nFlags)
{
    bool        bUpdate = false;
    string        name;
    char        szValue[64];
    CUIObject    *pObjFocus;
    pObjFocus = getFocusUIObj();

    if (nChar == VK_DELETE)
    {
        removeFocusUIObject();

        m_pNotify->onUIObjFocusChanged();
        return;
    }
    else if (nChar == VK_LEFT)
    {
        // 左移焦点控件
        if (pObjFocus)
        {
            sprintf(szValue, "%s-1", pObjFocus->m_formLeft.getFormula());
            name = "left";
            bUpdate = true;
        }
    }
    else if (nChar == VK_RIGHT)
    {
        // 右移焦点控件
        CUIObject    *pObj;
        pObj = getFocusUIObj();
        if (pObj)
        {
            sprintf(szValue, "%s+1", pObj->m_formLeft.getFormula());
            name = "left";
            bUpdate = true;
        }
    }
    else if (nChar == VK_UP)
    {
        // 上移焦点控件
        if (pObjFocus)
        {
            sprintf(szValue, "%s-1", pObjFocus->m_formTop.getFormula());
            name = "top";
            bUpdate = true;
        }
    }
    else if (nChar == VK_DOWN)
    {
        // 上移焦点控件
        if (pObjFocus)
        {
            name = "top";
            sprintf(szValue, "%s+1", pObjFocus->m_formTop.getFormula());
            bUpdate = true;
        }
    }
    if (bUpdate)
    {
        if (pObjFocus)
        {
            pObjFocus->setProperty(name.c_str(), szValue);
            RecalculateSizePos();
            invalidateRect(nullptr, true);
            // 更新
            CFormula    form;
            form.setFormula(szValue);
            m_pNotify->onFocusUIObjPropertyChanged(name.c_str(), form.getFormula());
        }
    }

    CSkinWnd::onKeyDown(nChar, nFlags);

    if (pObjFocus != getFocusUIObj())
    {
        // 更新
        m_pNotify->onUIObjFocusChanged();
    }
}

void CSESkinWnd::onLButtonDown(uint32_t nFlags, CPoint point)
{
    CUIObject    *pObjFocus, *pObjFocusNew;

    pObjFocus = getFocusUIObj();

    CSkinWnd::onLButtonDown(nFlags, point);

    pObjFocusNew = getFocusUIObj();

    point.x = long(point.x / m_dbScale);
    point.y = long(point.y / m_dbScale);

    // 判断鼠标是否击中某个object?
    bool    bHit = false;
    if (pObjFocusNew && pObjFocusNew->isPtIn(point))
    {
        bHit = true;
    }

    if (!bHit && pObjFocus)
    {
        // 未击中，在skin编辑模式下，不设置任何Object为焦点状态
        setNOFocusUIObj();
    }

    if (pObjFocus != pObjFocusNew)
    {
        // 更新
        m_pNotify->onUIObjFocusChanged();
    }
}

void CSESkinWnd::onMouseMove(uint32_t nFlags, CPoint point)
{
    CSkinWnd::onMouseMove(nFlags, point);

    g_seUIAdapter.setMousePointerInfo(int(point.x / m_dbScale), int(point.y / m_dbScale));
}

void CSESkinWnd::onPaint(CRawGraph *canvas, CRect *rcClip)
{
    // // 刷背景色
    // if (m_brBg.isValid())
    //     memCanvas->fillRect(0, 0, m_rcReal.width(), m_rcReal.height(), &m_brBg);

    CSkinWnd::onPaint(canvas, rcClip);
}

void CSESkinWnd::onContexMenu(int xPos, int yPos)
{
    HMENU    hMenuSub = GetSubMenu(m_hMenuContext, 0);
    g_seSkinPropertyAdapter.trackPopupMenu(hMenuSub, xPos, yPos);
}

void CSESkinWnd::onRButtonUp(uint32_t nFlags, CPoint point)
{
    CPoint pt = getCursorPos();
    onContexMenu(pt.x, pt.y);
}

LRESULT CSESkinWnd::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam)
{
    return CSkinWnd::wndProc(message, wParam, lParam);
}

bool CSESkinWnd::setProperty(cstr_t szProperty, cstr_t szValue)
{
    bool    bRet;

    if (strcasecmp(szProperty, SZ_PN_NAME) == 0)
    {
        if (strcmp(szValue, m_strSkinWndName.c_str()) != 0)
        {
            if (m_pSkinFactory->getSkinFile()->changeSkinWndName(m_strSkinWndName.c_str(), szValue) == ERR_OK)
            {
                m_strSkinWndName = szValue;
                m_pNotify->onSkinWndNameChanged();
            }
            else
                m_pNotify->onUIObjFocusChanged();
        }
        return true;
    }
    else if (isPropertyName(szProperty, "EnableTransparentFeature"))
    {
        m_bEnableTransparentFeatureEditing = isTRUE(szValue);
        return true;
    }
    else if (isPropertyName(szProperty, "EnableTranslucency"))
    {
        m_bTranslucencyLayeredEditing = isTRUE(szValue);
        return true;
    }

    bRet = CSkinWnd::setProperty(szProperty, szValue);

    if (strcasecmp(szProperty, SZ_PN_WIDTH) == 0 ||
        strcasecmp(szProperty, SZ_PN_HEIGHT) == 0)
        ::moveWindow(m_hWnd, 5, 5, int(m_nWidth * m_dbScale), int(m_nHeight * m_dbScale), true);
    else if (strcasecmp(szProperty, "PaintBg") == 0 ||
        strcasecmp(szProperty, "BgColor") == 0)
        invalidateRect();
    else if (strcasecmp(szProperty, "Cursor") == 0)
        setCursor(m_cursor);
    else if (strcasecmp(szProperty, "Region") == 0)
        updateSkinProperty();

    if (bRet)
        return true;

    if (strcasecmp(szProperty, "CmdHandler") == 0)
        m_strCmdHandler = szValue;
    else if (strcasecmp(szProperty, "ContextMenu") == 0)
        m_strContextMenu = szValue;
    else if (strcasecmp(szProperty, "EnableVolumeSwitch") == 0)
        m_bEnableVolumeSwitch = isTRUE(szValue);
    else if (strcasecmp(szProperty, "IndividualProperties") == 0)
        m_bIndividualProperties = isTRUE(szValue);
    else
        return false;

    return true;
}

void CSESkinWnd::enumProperties(CUIObjProperties &listProperties)
{
    CSkinWnd::enumProperties(listProperties);

    listProperties.delProp("EnableTransparentFeature");
    listProperties.delProp("EnableTranslucency");

    listProperties.addPropBoolStr("EnableTransparentFeature", m_bEnableTransparentFeatureEditing, !m_bEnableTransparentFeatureEditing);
    listProperties.addPropBoolStr("EnableTranslucency", m_bTranslucencyLayeredEditing, m_bTranslucencyLayeredEditing);

    listProperties.addPropStr("CmdHandler", m_strCmdHandler.c_str(), !m_strCmdHandler.empty());
    listProperties.addPropBoolStr("EnableVolumeSwitch", m_bEnableVolumeSwitch, m_bEnableVolumeSwitch);
}

void CSESkinWnd::zoomIn()
{
    if (m_dbScale < 4.0)
    {
        m_dbScale += 1.0;
        ::moveWindow(m_hWnd, 5, 5, int(m_rcBoundBox.width() * m_dbScale), int(m_rcBoundBox.height() * m_dbScale), true);
        invalidateRect();
    }
}

void CSESkinWnd::zoomOut()
{
    if (m_dbScale > 1.0)
    {
        m_dbScale -= 1.0;
        ::moveWindow(m_hWnd, 5, 5, int(m_rcBoundBox.width() * m_dbScale), int(m_rcBoundBox.height() * m_dbScale), true);
        invalidateRect();
    }
}
