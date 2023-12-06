#include "Skin.h"
#include "SkinTreeCtrl.h"


void ISkinTreeNode::addChildBack(ISkinTreeNode *pChild) {
    if (m_pLastChild) {
        m_pLastChild->m_pNext = pChild;
    }
    pChild->m_pPrev = m_pLastChild;
    pChild->m_pNext = nullptr;
    pChild->m_pParent = this;
    m_pLastChild = pChild;
    if (m_pFirstChild == nullptr) {
        m_pFirstChild = pChild;
    }
}

void ISkinTreeNode::addChildFront(ISkinTreeNode *pChild) {
    if (m_pFirstChild) {
        m_pFirstChild->m_pPrev = pChild;
    }
    pChild->m_pPrev = nullptr;
    pChild->m_pNext = m_pFirstChild;
    pChild->m_pParent = this;
    m_pFirstChild = pChild;
    if (m_pLastChild == nullptr) {
        m_pLastChild = pChild;
    }
}

void ISkinTreeNode::removeLink() {
    if (m_pPrev) {
        m_pPrev->m_pNext = m_pNext;
    }
    if (m_pNext) {
        m_pNext->m_pPrev = m_pPrev;
    }

    if (m_pParent->m_pFirstChild == this) {
        m_pParent->m_pFirstChild = m_pNext;
    }
    if (m_pParent->m_pLastChild == this) {
        m_pParent->m_pLastChild = m_pPrev;
    }
}

ISkinTreeNode *ISkinTreeNode::getChildByIndex(int nIndex) {
    ISkinTreeNode *pNode = m_pFirstChild;
    for (int i = 0; i < nIndex && pNode; i++) {
        pNode = pNode->m_pNext;
    }

    return pNode;
}

ISkinTreeNode *ISkinTreeNode::getChildByName(cstr_t szName) {
    ISkinTreeNode *pNode = m_pFirstChild;
    while (pNode) {
        if (strcmp(pNode->m_strName.c_str(), szName) == 0) {
            return pNode;
        }
        pNode = pNode->m_pNext;
    }

    return pNode;
}

int ISkinTreeNode::getDepth() {
    int i = -1;
    ISkinTreeNode *pParent = m_pParent;
    while (pParent) {
        i++;
        pParent = pParent->m_pParent;
    }
    return i;
}

bool ISkinTreeNode::isBigBrother() {
    if (!m_pParent) {
        return true;
    }

    return m_pParent->m_pFirstChild == this;
}

void ISkinTreeNode::getPath(SkinTreePath_t &path) {
    ISkinTreeNode *pNode, *pNodeSearch;
    int nIndex;

    path.clear();

    pNodeSearch = this;
    while (pNodeSearch->m_pParent) {
        pNode = pNodeSearch->m_pParent->firstChild();
        for (nIndex = 0; pNode != nullptr; pNode = pNode->nextSibling(), nIndex++) {
            if (pNode == pNodeSearch) {
                break;
            }
        }
        path.insert(path.begin(), nIndex);
        pNodeSearch = pNodeSearch->m_pParent;
    }
}

void ISkinTreeNode::getPath(SkinTreeStrPath_t &path) {
    ISkinTreeNode *pNodeSearch;

    path.clear();

    pNodeSearch = this;
    while (pNodeSearch->m_pParent) {
        path.push_back(pNodeSearch->m_strName);
        pNodeSearch = pNodeSearch->m_pParent;
    }
}

int ISkinTreeNode::getExpandedNodeCount() {
    int n = 1;
    ISkinTreeNode *pNode = m_pFirstChild;
    while (pNode) {
        if (pNode->hasChild() && pNode->isExpanded()) {
            n += pNode->getExpandedNodeCount();
        } else {
            n++;
        }

        pNode = pNode->m_pNext;
    }

    return n;
}

int ISkinTreeNode::getExpandedNodeCountTill(ISkinTreeNode *pNodeTill, bool &bFound) {
    int n = 1;
    ISkinTreeNode *pNode = m_pFirstChild;
    while (pNode) {
        if (pNodeTill == pNode) {
            bFound = true;
            break;
        }
        if (pNode->hasChild() && pNode->isExpanded()) {
            n += pNode->getExpandedNodeCountTill(pNodeTill, bFound);
        } else {
            n++;
        }

        if (bFound) {
            break;
        }
        pNode = pNode->m_pNext;
    }

    return n;
}

ISkinTreeNode *ISkinTreeNode::getExpandedNodeByPos(int &nPos) {
    ISkinTreeNode *pNodeFound, *pNode;

    if (nPos == 0) {
        return this;
    }
    nPos--;
    if (isExpanded() && m_pFirstChild) {
        pNode = m_pFirstChild;
        while (pNode) {
            pNodeFound = pNode->getExpandedNodeByPos(nPos);
            if (pNodeFound) {
                return pNodeFound;
            }
            pNode = pNode->m_pNext;
        }
    }

    /*    pNode = this->nextSibling();
    while (pNode)
    {
        pNodeFound = pNode->getExpandedNodeByPos(nPos);
        if (pNodeFound)
            return pNodeFound;
        pNode = pNode->nextSibling();
    }*/

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
ISkinTree::ISkinTree() {
    m_root.setExpanded(true);
}

ISkinTree::~ISkinTree() {
    m_root.free();
}

void ISkinTree::getCurPath(SkinTreePath_t &path) {
    path = m_pathSel;
}

void ISkinTree::getCurPath(SkinTreeStrPath_t &path) {
    ISkinTreeNode *pNode;

    path.clear();
    pNode = getSelNode();
    if (pNode) {
        pNode->getPath(path);
    }
}

ISkinTreeNode *ISkinTree::getSelNode() {
    return getNode(m_pathSel);
}

ISkinTreeNode *ISkinTree::getFirstVisibleNode() {
    ISkinTreeNode *pNode = getNode(m_pathFirstVisible);
    if (!pNode) {
        pNode = m_root.firstChild();
    }

    return pNode;
}

int ISkinTree::getAllExpandedNodeCount() {
    return m_root.getExpandedNodeCount() - 1;
}

int ISkinTree::getFirstVisibleNodePos() {
    ISkinTreeNode *pNodeFirstVisible = getFirstVisibleNode();
    bool bFound = false;
    return m_root.getExpandedNodeCountTill(pNodeFirstVisible, bFound) - 1;
}

int ISkinTree::getExpandedNodePos(ISkinTreeNode *pNode) {
    bool bFound = false;
    return m_root.getExpandedNodeCountTill(pNode, bFound) - 1;
}

ISkinTreeNode *ISkinTree::getExpandedNodeByPos(int nPos) {
    nPos++;
    return m_root.getExpandedNodeByPos(nPos);
}

bool ISkinTree::setSelNode(ISkinTreeNode *pNode) {
    ISkinTreeNode *pNodeSelOld = getSelNode();

    if (pNodeSelOld == pNode) {
        return false;
    }

    if (pNodeSelOld) {
        pNodeSelOld->setSelected(false);
    }

    m_pathSel.clear();

    if (pNode) {
        pNode->setSelected(true);
        pNode->getPath(m_pathSel);
    }

    return true;
}

void ISkinTree::setFirstVisibleNode(ISkinTreeNode *pNode) {
    m_pathFirstVisible.clear();

    if (pNode) {
        pNode->getPath(m_pathFirstVisible);
    }
}

ISkinTreeNode *ISkinTree::getNode(SkinTreePath_t &path) {
    ISkinTreeNode *pNode, *pNodeNext;

    pNodeNext = nullptr;
    pNode = &m_root;
    for (int i = 0; i < (int)path.size(); i++) {
        pNodeNext = pNode->getChildByIndex(path[i]);
        if (!pNodeNext) {
            pNodeNext = pNode->lastChild();
            if (!pNodeNext) {
                pNodeNext = pNode;
                break;
            }
        }
        pNode = pNodeNext;
    }

    return pNodeNext;
}

ISkinTreeNode *ISkinTree::getNode(SkinTreeStrPath_t &path) {
    ISkinTreeNode *pNode, *pNodeNext;

    pNodeNext = nullptr;
    pNode = &m_root;
    for (int i = 0; i < (int)path.size(); i++) {
        pNodeNext = pNode->getChildByName(path[i].c_str());
        if (!pNodeNext) {
            pNodeNext = pNode->lastChild();
            if (!pNodeNext) {
                pNodeNext = pNode;
                break;
            }
        }
        pNode = pNodeNext;
    }

    return pNodeNext;
}

// 中序遍历
ISkinTreeNode *ISkinTree::centerOrderNextExpandedOnly(ISkinTreeNode *pNode) {
    if (pNode->firstChild() && pNode->isExpanded()) {
        return pNode->firstChild();        // child
    } else if (pNode->nextSibling()) {
        return pNode->nextSibling();    // next sibling
    } else {
        // parent's next sibling
        while (pNode->m_pParent->m_pParent) {
            pNode = pNode->m_pParent;
            if (pNode->nextSibling()) {
                return pNode->nextSibling();
            }
        }
        return nullptr;
    }
}


//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinTreeCtrl, "TreeCtrl")

CSkinTreeCtrl::CSkinTreeCtrl() {
    m_pTreeData = nullptr;

    m_nXMargin = 2;
    m_nLineIndent = 18;

    m_msgNeed = UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_KEY | UO_MSG_WANT_MOUSEWHEEL;

    // setBkColor(CColor(GetSysColor(COLOR_WINDOW)));
    setBkColor(CColor(RGB(192, 192, 192)));
    setSelRowBkColor(CColor(RGB(128, 128, 128)));
    m_clrSelText.set(RGB(255, 255, 255));

    m_nLineHeight = 14;

    m_pObjScrollBar = nullptr;
    m_pScrollBar = nullptr;

    /*    // test code
    ISkinTree        g_treeData;
    ISkinTreeNode *pRoot = g_treeData.getRoot();
    ISkinTreeNode    *p, *p2;

    for (int i = 0; i < 20; i++)
    {
        p = new ISkinTreeNode;
        p->m_strName = stringPrintf("Node %d", i).c_str();
        p->setExpanded(true);
        pRoot->addChildBack(p);

        for (int k = 0; k < 2; k++)
        {
            p2 = new ISkinTreeNode;
            p2->m_strName = stringPrintf("Node %d-%d", i, k).c_str();
            p->setExpanded(true);
            p->addChildBack(p2);
        }
    }
    setDataSrc(&g_treeData);

    for ( i = 0; i < 40; i++)
    {
        p = m_pTreeData->getExpandedNodeByPos(i);
        int    nRet = m_pTreeData->getExpandedNodePos(p);
        if (i != nRet)
        {
            ERR_LOG0("not equal.");
        }
    }*/
}

CSkinTreeCtrl::~CSkinTreeCtrl() {

}

void CSkinTreeCtrl::draw(CRawGraph *canvas) {
    ISkinTreeNode *pNodeFirstVisible, *pNode, *pNodeNext;
    int nDepth;
    int x, y, xMax, yMax;
    CRect rcNodeText;

    canvas->fillRect(m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), m_clrBk);

    if (!m_pTreeData) {
        return;
    }

    pNodeFirstVisible = m_pTreeData->getFirstVisibleNode();
    if (!pNodeFirstVisible) {
        return;
    }

    CRawGraph::CClipBoxAutoRecovery autoCBR(canvas);
    canvas->setClipBoundBox(m_rcObj);

    xMax = m_rcObj.right - m_nXMargin;
    //if (bVScrollBarVisible && m_pObjScrollBar)
    //    xMax -= m_pObjScrollBar->m_rcObj.width();

    y = m_rcObj.top;
    yMax = m_rcObj.bottom;

    // draw Tree from first visible node
    nDepth = pNodeFirstVisible->getDepth();
    pNode = pNodeFirstVisible;

    if (m_font.isGood()) {
        canvas->setFont(m_font.getFont());
    }

    while (y < yMax && pNode) {
        if (pNode->isSelected()) {
            // draw background
            x = m_rcObj.left;
            canvas->fillRect(x, y, m_rcObj.right - x, m_nLineHeight, m_clrSelRowBk);
        }

        // draw image
        x = m_rcObj.left + m_nXMargin + nDepth * m_nLineIndent;
        if (pNode->isExpanded() && pNode->hasChild() && pNode->m_nExpandedImageIndex != SKIN_TREE_INVALID_IMG_INDEX) {
            m_imageList.draw(canvas, pNode->m_nExpandedImageIndex, x, y);
        } else {
            m_imageList.draw(canvas, pNode->m_nImageIndex, x, y);
        }

        // draw text
        x += m_imageList.getIconCx() + 6;
        rcNodeText.left = x;
        rcNodeText.top = y;
        rcNodeText.right = xMax;
        rcNodeText.bottom = y + m_nLineHeight;

        if (m_font.isOutlined()) {
            canvas->drawTextOutlined(pNode->m_strName.c_str(), (int)pNode->m_strName.size(), rcNodeText,
                pNode->isSelected() ? m_clrSelText : m_font.getTextColor(m_enable), m_font.getColorOutlined());
        } else {
            if (pNode->isSelected()) {
                canvas->setTextColor(m_clrSelText);
            } else {
                canvas->setTextColor(m_font.getTextColor(m_enable));
            }
            canvas->drawText(pNode->m_strName.c_str(), (int)pNode->m_strName.size(), rcNodeText);
        }

        // to next item
        if (pNode->hasChild() && pNode->isExpanded()) {
            // to it's child
            pNodeNext = pNode->firstChild();
            if (pNodeNext) {
                nDepth++;
            }
        } else {
            pNodeNext = nullptr;
        }

        if (!pNodeNext) {
            pNodeNext = pNode->nextSibling();
            if (!pNodeNext) {
                while (!pNodeNext && pNode) {
                    pNode = pNode->m_pParent;
                    nDepth--;
                    if (pNode) {
                        pNodeNext = pNode->nextSibling();
                    } else {
                        pNodeNext = nullptr;
                    }
                }
            }
        }
        pNode = pNodeNext;
        y += m_nLineHeight;
    }
}

CSkinTreeCtrl::TreeArea CSkinTreeCtrl::hitTest(CPoint point, ISkinTreeNode **ppNode) {
    if (!m_pTreeData) {
        return TA_BLANK;
    }

    ISkinTreeNode *pNodeFirstVisible, *pNode, *pNodeNext;
    int nDepth;
    int x, y, xMax, yMax;
    bool bVScrollBarVisible = true;

    pNodeFirstVisible = m_pTreeData->getFirstVisibleNode();
    if (!pNodeFirstVisible) {
        return TA_BLANK;
    }

    xMax = m_rcObj.right - m_nXMargin;
    if (bVScrollBarVisible && m_pObjScrollBar) {
        xMax -= m_pObjScrollBar->m_rcObj.width();
    }

    y = m_rcObj.top;
    yMax = m_rcObj.bottom;

    // draw Tree from first visible node
    nDepth = pNodeFirstVisible->getDepth();
    pNode = pNodeFirstVisible;

    if (point.y < y || point.x > xMax || point.x < m_rcObj.left + m_nXMargin) {
        return TA_BLANK;
    }

    while (y < yMax && pNode) {
        if (point.y < y + m_nLineHeight) {
            // hit on this line
            if (ppNode) {
                *ppNode = pNode;
            }

            // image area?
            x = m_rcObj.left + m_nXMargin + nDepth * m_nLineIndent;
            if (point.x >= x && point.x <= x + m_imageList.getIconCx()) {
                return TA_EXPAND;
            } else {
                return TA_LINE;
            }
        }

        // to next item
        if (pNode->hasChild() && pNode->isExpanded()) {
            // to it's child
            pNodeNext = pNode->firstChild();
            if (pNodeNext) {
                nDepth++;
            }
        } else {
            pNodeNext = nullptr;
        }

        if (!pNodeNext) {
            pNodeNext = pNode->nextSibling();
            if (!pNodeNext) {
                while (!pNodeNext && pNode) {
                    pNode = pNode->m_pParent;
                    nDepth--;
                    if (pNode) {
                        pNodeNext = pNode->nextSibling();
                    } else {
                        pNodeNext = nullptr;
                    }
                }
            }
        }
        pNode = pNodeNext;
        y += m_nLineHeight;
    }

    return TA_BLANK;
}

bool CSkinTreeCtrl::onLButtonDown(uint32_t nFlags, CPoint point) {
    if (!m_pTreeData) {
        return false;
    }

    ISkinTreeNode *pNode = nullptr;

    TreeArea ta = hitTest(point, &pNode);
    if (ta != TA_BLANK && pNode) {
        setSelNode(pNode);

        if (ta == TA_EXPAND && pNode->hasChild()) {
            pNode->setExpanded(!pNode->isExpanded());
            updateVScrollBar();
        }

        invalidate();
    }

    return true;
}

bool CSkinTreeCtrl::onLButtonDblClk(uint32_t nFlags, CPoint point) {
    if (!m_pTreeData) {
        return false;
    }

    ISkinTreeNode *pNode = nullptr;

    TreeArea ta = hitTest(point, &pNode);
    if (ta != TA_BLANK && pNode) {
        setSelNode(pNode);

        if (!pNode->noChild()) {
            pNode->setExpanded(!pNode->isExpanded());
            updateVScrollBar();
        }

        invalidate();

        notifyEvent(CSkinTreeCtrlEventNotify::C_DBL_CLICK);
    }

    return true;
}

bool CSkinTreeCtrl::onKeyDown(uint32_t nChar, uint32_t nFlags) {
    if (!m_pTreeData) {
        return false;
    }

    switch (nChar) {
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
        {
            ISkinTreeNode *pNodeSel;

            pNodeSel = m_pTreeData->getSelNode();
            if (!pNodeSel) {
                pNodeSel = m_pTreeData->getFirstVisibleNode();
                if (pNodeSel) {
                    setSelNode(pNodeSel);
                }
                return true;
            }

            if (nChar == VK_LEFT) {
                if (pNodeSel->isExpanded() && pNodeSel->hasChild()) {
                    pNodeSel->setExpanded(false);
                    updateVScrollBar();
                } else if (pNodeSel->m_pParent != m_pTreeData->getRoot()) {
                    pNodeSel = pNodeSel->m_pParent;
                    setSelNode(pNodeSel);
                }
            } else if (nChar == VK_RIGHT) {
                if (!pNodeSel->isExpanded() && pNodeSel->hasChild()) {
                    pNodeSel->setExpanded(true);
                    updateVScrollBar();
                } else if (pNodeSel->hasChild()) {
                    pNodeSel = pNodeSel->firstChild();
                    setSelNode(pNodeSel);
                }
            } else if (nChar == VK_UP) {
                // move to previous visible node
                if (!pNodeSel->prevSibling()) {
                    // to its parent
                    if (pNodeSel->m_pParent != m_pTreeData->getRoot()) {
                        pNodeSel = pNodeSel->m_pParent;
                        setSelNode(pNodeSel);
                        invalidate();
                    }
                    return true;
                } else {
                    pNodeSel = pNodeSel->prevSibling();
                }
                while (pNodeSel->isExpanded() && pNodeSel->hasChild()) {
                    pNodeSel = pNodeSel->m_pLastChild;
                }
                setSelNode(pNodeSel);
            } else if (nChar == VK_DOWN) {
                // move to next visible node
                if (pNodeSel->isExpanded() && pNodeSel->firstChild()) {
                    pNodeSel = pNodeSel->firstChild();
                    setSelNode(pNodeSel);
                    invalidate();
                    return true;
                }

                if (pNodeSel->nextSibling()) {
                    pNodeSel = pNodeSel->nextSibling();
                    setSelNode(pNodeSel);
                    invalidate();
                    return true;
                }

                // to its parent's next sibling
                while (pNodeSel->m_pParent->m_pParent) {
                    pNodeSel = pNodeSel->m_pParent;
                    if (pNodeSel->nextSibling()) {
                        pNodeSel = pNodeSel->nextSibling();
                        setSelNode(pNodeSel);
                        break;
                    }
                }
            }

            invalidate();
        }
        break;
    case VK_RETURN:
        {
            notifyEvent(CSkinTreeCtrlEventNotify::C_ENTER);
            break;
        }
    case VK_DELETE:
        {
            notifyEvent(CSkinTreeCtrlEventNotify::C_KEY_DELETE);
            break;
        }
    default:
        return false;
    }

    return true;
}

bool CSkinTreeCtrl::onLButtonUp(uint32_t nFlags, CPoint point) {
    notifyEvent(CSkinTreeCtrlEventNotify::C_CLICK);

    return true;
}

void CSkinTreeCtrl::onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) {
    if (m_pObjScrollBar) {
        m_pObjScrollBar->onMouseWheel(nWheelDistance, nMkeys, pt);
    }
}

void CSkinTreeCtrl::onCreate() {
    m_font.setParent(m_pSkin);

    CUIObject::onCreate();

    m_nLineHeight = m_font.getHeight() + 2;

    // if in Editor mode, don't create scrollbar
    if (m_pSkin->isInEditorMode()) {
        return;
    }

    // create Scrollbar
    m_pObjScrollBar = m_pSkin->getSkinFactory()->createDynamicCtrl(m_pContainer, CSkinVScrollBar::className(), ID_INVALID, nullptr, nullptr, nullptr, nullptr);
    assert(m_pObjScrollBar);
    if (!m_pObjScrollBar) {
        return;
    }

    char szTemp[MAX_PATH];

    // set left
    snprintf(szTemp, CountOf(szTemp), "%s+%s-%s", m_formLeft.getFormula(), m_formWidth.getFormula(),
        m_pObjScrollBar->m_formWidth.getFormula());
    m_pObjScrollBar->setProperty(SZ_PN_LEFT, szTemp);

    // top
    m_pObjScrollBar->setProperty(SZ_PN_TOP, m_formTop.getFormula());

    // width of this
    snprintf(szTemp, CountOf(szTemp), "%s-%s", m_formWidth.getFormula(), m_pObjScrollBar->m_formWidth.getFormula());
    m_formWidth.setFormula(szTemp);

    // height.
    m_pObjScrollBar->setProperty(SZ_PN_HEIGHT, m_formHeight.getFormula());

    m_pContainer->recalculateUIObjSizePos(m_pObjScrollBar);

    m_pScrollBar = (CSkinVScrollBar*)m_pObjScrollBar;
    m_pScrollBar->setScrollNotify(this);
}

void CSkinTreeCtrl::onSize() {
    CUIObject::onSize();

    updateVScrollBar();
}

void CSkinTreeCtrl::onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) {
    if (!m_pTreeData) {
        return;
    }

    ISkinTreeNode *pNode;
    pNode = m_pTreeData->getExpandedNodeByPos(nPos);
    if (pNode) {
        m_pTreeData->setFirstVisibleNode(pNode);
        invalidate();
    } else if (m_pScrollBar) {
        m_pScrollBar->setScrollPos(m_pTreeData->getFirstVisibleNodePos());
    }
}

void CSkinTreeCtrl::onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) {

}

bool CSkinTreeCtrl::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "BgColor") == 0 ||
        strcasecmp(szProperty, "BkColor") == 0) {
        getColorValue(m_clrBk, szValue);
    } else if (strcasecmp(szProperty, "SelBgColor") == 0 ||
        strcasecmp(szProperty, "SelBkColor") == 0) {
        getColorValue(m_clrSelRowBk, szValue);
    } else if (strcasecmp(szProperty, "SelTextColor") == 0) {
        getColorValue(m_clrSelText, szValue);
    } else if (m_font.setProperty(szProperty, szValue)) {
        return true;
    } else if (strcasecmp(szProperty, "ImageList") == 0) {
        m_strImageList = szValue;
        m_imageList.load(m_pSkin, szValue, 16);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinTreeCtrl::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropColor("BgColor", m_clrBk);
    listProperties.addPropColor("SelBgColor", m_clrSelRowBk);
    listProperties.addPropColor("SelTextColor", m_clrSelText);
    m_font.enumProperties(listProperties);
    listProperties.addPropImageFile("ImageList", m_strImageList.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinTreeCtrl::setBkColor(const CColor &clrBk) {
    m_clrBk = clrBk;
}

void CSkinTreeCtrl::setSelRowBkColor(const CColor &clrBk) {
    m_clrSelRowBk = clrBk;
}

void CSkinTreeCtrl::setTextColor(const CColor &clrText) {
    m_font.setProperty("TextColor", colorToStr(clrText).c_str());
}

void CSkinTreeCtrl::notifyEvent(CSkinTreeCtrlEventNotify::Command cmd) {
    CSkinTreeCtrlEventNotify event(this);

    event.cmd = cmd;
    m_pSkin->dispatchUIObjNotify(&event);
}

void CSkinTreeCtrl::setSelNode(ISkinTreeNode *pNode) {
    if (!m_pTreeData) {
        return;
    }

    if (!pNode->isUpdated()) {
        pNode->setUpdated(true);
        pNode->onUpdate();
    }

    if (m_pTreeData->setSelNode(pNode)) {
        makeSureSelNodeVisible();
        notifyEvent(CSkinTreeCtrlEventNotify::C_SEL_CHANGED);
    }
}

void CSkinTreeCtrl::setFirstVisibleNode(ISkinTreeNode *pNode) {
    if (!m_pTreeData || !m_pScrollBar) {
        return;
    }

    m_pTreeData->setFirstVisibleNode(pNode);
    m_pScrollBar->setScrollPos(m_pTreeData->getFirstVisibleNodePos());
}

void CSkinTreeCtrl::updateVScrollBar() {
    if (!m_pTreeData || !m_pScrollBar) {
        return;
    }

    int nCount = m_pTreeData->getAllExpandedNodeCount();
    int nLinePerPage = m_rcObj.height() / m_nLineHeight;
    if (nLinePerPage == 0) {
        nLinePerPage = 1;
    }

    m_pScrollBar->setScrollInfo(0, nCount, nLinePerPage, m_pTreeData->getFirstVisibleNodePos(), 1);
}

void CSkinTreeCtrl::makeSureSelNodeVisible() {
    if (!m_pTreeData) {
        return;
    }

    ISkinTreeNode *pNodeSel = m_pTreeData->getSelNode();
    int nLinePerPage = m_rcObj.height() / m_nLineHeight;
    int nPosFirstVisibleOld, nPosFirstVisible, nPosCurSel;

    if (!pNodeSel) {
        return;
    }

    if (nLinePerPage == 0) {
        nLinePerPage = 1;
    }

    nPosFirstVisibleOld = nPosFirstVisible = m_pTreeData->getFirstVisibleNodePos();
    nPosCurSel = m_pTreeData->getExpandedNodePos(pNodeSel);

    if (nPosCurSel < nPosFirstVisible) {
        if (nPosCurSel == nPosFirstVisible - 1) {
            nPosFirstVisible = nPosCurSel;
        } else {
            while (nPosCurSel < nPosFirstVisible) {
                nPosFirstVisible -= nLinePerPage;
            }
            nPosFirstVisible += nLinePerPage;
        }
        if (nPosFirstVisible < 0) {
            nPosFirstVisible = 0;
        }
    } else if (nPosCurSel >= nPosFirstVisible + nLinePerPage) {
        if (nPosCurSel == nPosFirstVisible + nLinePerPage) {
            nPosFirstVisible++;
        } else {
            while (nPosCurSel > nPosFirstVisible) {
                nPosFirstVisible += nLinePerPage;
            }
            nPosFirstVisible -= nLinePerPage;

            // nPosFirstVisible = nPosCurSel - nPosCurSel % nLinePerPage + nPosFirstVisible % nLinePerPage;
        }
        if (nPosFirstVisible < 0) {
            nPosFirstVisible = 0;
        }
    } else {
        return;
    }

    if (nPosFirstVisibleOld != nPosFirstVisible) {
        ISkinTreeNode *pNode = m_pTreeData->getExpandedNodeByPos(nPosFirstVisible);
        if (pNode) {
            setFirstVisibleNode(pNode);
            invalidate();
        }
    }

}
