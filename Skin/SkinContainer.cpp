#include "SkinTypes.h"
#include "Skin.h"
#include "SkinContainer.h"


#define SZ_EX_POOL          "ExPool."
#define LEN_EX_POOL        (CountOf(SZ_EX_POOL) - 1)

//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinContainer, "Container")

CSkinContainer::CSkinContainer() {
    // m_msgNeed = UO_MSG_WANT_ALL;
    m_pMenu = nullptr;
    m_bClipChildren = false;
    m_nFocusUIObj = -1;
}

CSkinContainer::~CSkinContainer() {
    destroy();
}

bool CSkinContainer::onCommand(int nID) {
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];

        if (!pObj->isVisible()) {
            continue;
        }

        if (pObj->isContainer() || pObj->needMsgMenuCmd()) {
            if (pObj->onCommand(nID)) {
                return true;
            }
        }
    }

    return false;
}

bool CSkinContainer::onCustomCommand(int nId) {
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];

        if (pObj->isContainer() || pObj->needMsgCustomCmd()) {
            if (pObj->onCustomCommand(nId)) {
                return true;
            }
        }
    }

    return false;
}

void CSkinContainer::onSwitchTo() {
    if (m_strText.size()) {
        m_pSkin->setCaptionText(_TL(m_strText.c_str()));
    }
}

bool CSkinContainer::onUserMessage(int nMessageID, LPARAM param) {
    for (uint32_t i = 0; i < m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        if (!pObj->isContainer()) {
            continue;
        }
        if (pObj->getContainerIf()->onUserMessage(nMessageID, param)) {
            return true;
        }
    }

    return false;
}

bool CSkinContainer::onClose() {
    for (uint32_t i = 0; i < m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        if (!pObj->isVisible() || !pObj->isContainer()) {
            continue;
        }
        if (!pObj->getContainerIf()->onClose()) {
            return false;
        }
    }

    return true;
}

bool CSkinContainer::onOK() {
    for (uint32_t i = 0; i < m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        if (!pObj->isVisible() || !pObj->isContainer()) {
            continue;
        }
        if (!pObj->getContainerIf()->onOK()) {
            return false;
        }
    }

    return true;
}

bool CSkinContainer::onCancel() {
    for (uint32_t i = 0; i < m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        if (!pObj->isVisible() || !pObj->isContainer()) {
            continue;
        }
        if (!pObj->getContainerIf()->onCancel()) {
            return false;
        }
    }

    return true;
}

void CSkinContainer::dispatchOnCreateMsg() {
    if (m_bCreated) {
        return;
    }
    m_bCreated = true;

    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];

        if (pObj->isContainer()) {
            CSkinContainer *pContainer = pObj->getContainerIf();
            pContainer->dispatchOnCreateMsg();
        } else if (!pObj->m_bCreated) {
            pObj->m_bCreated = true;
            pObj->onCreate();
        }
    }

    onCreate();
}

void CSkinContainer::onInitialUpdate() {
    if (m_strDefaultPageClass.empty()) {
        return;
    }

    CSkinContainer *pToActivate = (CSkinContainer *)getChildByClass(m_strDefaultPageClass.c_str());
    if (pToActivate) {
        PageViewItem pvToActivate(pToActivate, false, 0);
        m_vStackPageView.push(pvToActivate);
        pToActivate->onSwitchTo();
        pToActivate->setVisible(true, false);
    } else {
        ERR_LOG1("Inexistant default Page: %s", m_strDefaultPageClass.c_str());
        return;
    }
}

void CSkinContainer::onDestroy() {
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];

        if (pObj->m_bCreated) {
            pObj->onDestroy();
            pObj->m_bCreated = false;
        }
    }

    destroy();

    m_bCreated = false;
    m_bInitialUpdated = false;
}

void CSkinContainer::onKeyUp(uint32_t nChar, uint32_t nFlags) {
    if (m_nFocusUIObj >= 0 && m_nFocusUIObj < (int)m_vUIObjs.size()) {
        CUIObject *pObj = m_vUIObjs[m_nFocusUIObj];
        pObj->onKeyUp(nChar, nFlags);
    }
}

void CSkinContainer::onKeyDown(uint32_t nChar, uint32_t nFlags) {
    if (m_nFocusUIObj >= 0 && m_nFocusUIObj < (int)m_vUIObjs.size()) {
        CUIObject *pObj = m_vUIObjs[m_nFocusUIObj];
        pObj->onKeyDown(nChar, nFlags);
    }
}

bool CSkinContainer::onMouseDrag(CPoint point) {
    bool bMsgProceed = false;

    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj;
        pObj = m_vUIObjs[i];

        if ((pObj->needMsgMouseMove() || pObj->isContainer()) && pObj->isVisible()
            && pObj->isEnable() && pObj->isPtIn(point)) {
            if (pObj->onMouseDrag(point)) {
                bMsgProceed = true;
                break;
            }
        }
    }

    return bMsgProceed;
}

bool CSkinContainer::onMouseMove(CPoint point) {
    bool bMsgProceed = false;

    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj;
        pObj = m_vUIObjs[i];

        if ((pObj->needMsgMouseMove() || pObj->isContainer()) && pObj->isVisible()
            && pObj->isEnable() && pObj->isPtIn(point)) {
            if (pObj->onMouseMove(point)) {
                bMsgProceed = true;
                break;
            }
        }
    }

    return bMsgProceed;
}

bool CSkinContainer::onLButtonDown(uint32_t nFlags, CPoint point) {
    int nCount;
    bool bMsgProceed = false;

    // send mouse move message to the ui objects that want to process
    nCount = (int)m_vUIObjs.size();
    for (int i = nCount - 1; i >= 0; i--) {
        CUIObject *pObj;
        pObj = m_vUIObjs[i];
        assert(pObj);

        if ((pObj->needMsgLButton() || pObj->isContainer()) && pObj->isVisible()
            && pObj->isEnable() && pObj->isPtIn(point)) {
            if (pObj->needMsgKey()) {
                pObj->setFocus();
            }
            if (pObj->onLButtonDown(nFlags, point)) {
                bMsgProceed = true;
                break;
            }
        }
    }

    return bMsgProceed;
}

bool CSkinContainer::onLButtonDblClk(uint32_t nFlags, CPoint point) {
    int nCount;
    bool bMsgProceed = false;

    // send mouse move message to the ui objects that want to process
    nCount = (int)m_vUIObjs.size();
    for (int i = nCount - 1; i >= 0; i--) {
        CUIObject *pObj;
        pObj = m_vUIObjs[i];
        assert(pObj);

        if ((pObj->needMsgLButton() || pObj->isContainer()) && pObj->isVisible()
            && pObj->isEnable() && pObj->isPtIn(point)) {
            if (pObj->needMsgKey()) {
                pObj->setFocus();
            }
            if (pObj->onLButtonDblClk(nFlags, point)) {
                bMsgProceed = true;
                break;
            }
        }
    }

    return bMsgProceed;
}

bool CSkinContainer::onLButtonUp(uint32_t nFlags, CPoint point) {
    int nCount;
    bool bMsgProceed = false;

    // send mouse move message to the ui objects that want to process
    nCount = (int)m_vUIObjs.size();
    for (int i = nCount - 1; i >= 0; i--) {
        CUIObject *pObj;
        pObj = m_vUIObjs[i];
        //        assert(pObj == nullptr);

        if ((pObj->needMsgLButton() || pObj->isContainer()) && pObj->isVisible()
            && pObj->isEnable() && pObj->isPtIn(point)) {
            if (pObj->onLButtonUp(nFlags, point)) {
                bMsgProceed = true;
                break;
            }
        }
    }

    return bMsgProceed;
}

bool CSkinContainer::onRButtonDown(uint32_t nFlags, CPoint point) {
    int nCount;
    bool bMsgProceed = false;

    // send mouse move message to the ui objects that want to process
    nCount = (int)m_vUIObjs.size();
    for (int i = nCount - 1; i >= 0; i--) {
        CUIObject *pObj;
        pObj = m_vUIObjs[i];
        //        assert(pObj == nullptr);

        if ((pObj->needMsgRButton() || pObj->isContainer()) && pObj->isVisible()
            && pObj->isEnable() && pObj->isPtIn(point)) {
            if (pObj->onRButtonDown(nFlags, point)) {
                bMsgProceed = true;
                break;
            }
        }
    }

    return bMsgProceed;
}

bool CSkinContainer::onRButtonUp(uint32_t nFlags, CPoint point) {
    int nCount;
    bool bMsgProceed = false;

    // send mouse move message to the ui objects that want to process
    nCount = (int)m_vUIObjs.size();
    for (int i = nCount - 1; i >= 0; i--) {
        CUIObject *pObj;
        pObj = m_vUIObjs[i];
        //        assert(pObj == nullptr);

        if ((pObj->needMsgRButton() || pObj->isContainer()) && pObj->isVisible()
            && pObj->isEnable() && pObj->isPtIn(point)) {
            if (pObj->onRButtonUp(nFlags, point)) {
                bMsgProceed = true;
                break;
            }
        }
    }

    if (!bMsgProceed && m_pMenu) {
        CPoint pt = getCursorPos();
        m_pMenu->trackPopupMenu(pt.x, pt.y, m_pSkin);
        return true;
    }

    return bMsgProceed;
}

void CSkinContainer::onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) {
    int nCount;

    // send mouse move message to every ui objects that want to process
    nCount = (int)m_vUIObjs.size();
    for (int i = nCount - 1; i >= 0; i--) {
        CUIObject *pObj;
        pObj = m_vUIObjs[i];
        assert(pObj);

        if ((pObj->needMsgMouseWheel() || pObj->isContainer()) && pObj->isVisible()
            && pObj->isEnable()) { // && pObj->isPtIn(point))
            pObj->onMouseWheel(nWheelDistance, nMkeys, pt);
            return;
        }
    }
}


bool CSkinContainer::onMenuKey(uint32_t nChar, uint32_t nFlags) {
    for (int i = (int)m_vUIObjs.size() - 1; i >= 0; i--) {
        CUIObject *pObj = m_vUIObjs[i];
        assert(pObj);

        if ((pObj->needMsgMenuKey() || pObj->isContainer()) && pObj->isVisible() && pObj->isEnable()) {
            if (pObj->onMenuKey(nChar, nFlags)) {
                return true;
            }
        }
    }

    return false;
}

void CSkinContainer::onSize() {
    CUIObject::onSize();

    // recalculate all UIObjects' position
    FORMULA_VAR     vars[] = {
        {'w', m_rcObj.width()},
        {'h', m_rcObj.height()},
        {0, 0}
    };

    // recalculate every UIObjects' position on back buffer one by one
    for (uint32_t i = 0; i < m_vUIObjs.size(); i++) {
        m_vUIObjs[i]->onMeasureSizePos(vars);
        m_vUIObjs[i]->onSize();
    }
}

void CSkinContainer::onLanguageChanged() {
    if (m_pMenu) {
        delete m_pMenu;
        m_pMenu = nullptr;

        m_pSkin->getSkinFactory()->loadMenu(m_pSkin, &m_pMenu, m_strContextMenu.c_str());
    }

    // Notify to child container...
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        m_vUIObjs[i]->onLanguageChanged();
    }
}

CUIObject *CSkinContainer::getChildByClass(cstr_t szClassName) {
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        if (pObj->isKindOf(szClassName)) {
            return pObj;
        }
    }

    return nullptr;
}

void CSkinContainer::onAdjustHue(float hue, float saturation, float luminance) {
    m_clrBg = m_clrBgOrg;
    m_pSkin->getSkinFactory()->getAdjustedHueResult(m_clrBg);

    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        pObj->onAdjustHue(hue, saturation, luminance);
    }
}


void CSkinContainer::onSkinFontChanged() {
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        pObj->onSkinFontChanged();
    }
}


CRawGraph *CSkinContainer::getMemGraph() {
    if (m_pContainer) {
        return m_pContainer->getMemGraph();
    } else {
        return m_pSkin->getMemGraphics();
    }
}

void CSkinContainer::draw(CRawGraph *canvas) {
    CRect rc;

    if (m_bClipChildren) {
        canvas->getClipBoundBox(rc);
        canvas->setClipBoundBox(m_rcObj);
    }

    CUIObject::draw(canvas);

    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        if (pObj->isVisible()) {
            CRawGraph::COpacityBlendAutoRecovery opacityAR(canvas, pObj->getOpacity());

            pObj->tryToCallInitialUpdate();
            pObj->draw(canvas);
        }
    }

    if (m_bClipChildren) {
        canvas->resetClipBoundBox(rc);
    }
}

int CSkinContainer::fromXML(SXNode *pXmlNode) {
    CUIObject::fromXML(pXmlNode);

    // recalculate position before create children.
    assert(m_pContainer);
    m_pContainer->recalculateUIObjSizePos(this);

    createChild(pXmlNode);

    return ERR_OK;
}

void CSkinContainer::createChild(SXNode *pXmlNode) {
    // create child CUIObject
    for (SXNode *pNode : pXmlNode->listChildren) {
        if (isPropertyName(pNode->name.c_str(), SZ_PN_PROPERTY)) {
            // CUIObject has handled it.
            continue;
        } else if (isPropertyName(pNode->name.c_str(), "PlaceHolder")) {
            createChild(pNode);
            continue;
        }

        CUIObject *pObj = m_pSkin->getSkinFactory()->createUIObject(m_pSkin, pNode->name.c_str(), this);
        if (pObj) {
            // set its properties
            pObj->fromXML(pNode);

            // add to child list
            addUIObject(pObj);
        } else {
            DBG_LOG1("Failed to create UIObject: %s", pNode->name.c_str());
        }
    }
}


bool CSkinContainer::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "ContextMenu") == 0) {
        if (m_pMenu) {
            delete m_pMenu;
            m_pMenu = nullptr;
        }

        m_strContextMenu = szValue;
        m_pSkin->getSkinFactory()->loadMenu(m_pSkin, &m_pMenu, szValue);
    } else if (isPropertyName(szProperty, "ClipChildren")) {
        m_bClipChildren = isTRUE(szValue);
    } else if (isPropertyName(szProperty, "DefaultPage")) {
        m_strDefaultPageClass = szValue;
    } else if (startsWith(szProperty, SZ_EX_POOL)) {
        assert(LEN_EX_POOL == strlen(SZ_EX_POOL));
        szProperty += LEN_EX_POOL;
        if (!isEmptyString(szProperty)) {
            m_pSkin->m_mapExchangePool[szProperty] = szValue;
        }
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinContainer::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropStr("ContextMenu", m_strContextMenu.c_str(), !m_strContextMenu.empty());
}
#endif // _SKIN_EDITOR_

void CSkinContainer::destroy() {
    if (m_pMenu) {
        delete m_pMenu;
        m_pMenu = nullptr;
    }

    while (!m_vUIObjs.empty()) {
        CUIObject *pObj = m_vUIObjs.back();
        m_pSkin->unregisterTimerObject(pObj);
        m_pSkin->onRemoveUIObj(*(m_vUIObjs.begin() + m_vUIObjs.size() - 1));
        m_vUIObjs.erase(m_vUIObjs.begin() + m_vUIObjs.size() - 1);
        delete pObj;
    }
    m_vUIObjs.clear();
}

// functions for CSkinWnd
CUIObject *CSkinContainer::getUIObjectByClassName(cstr_t szClassName) {
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        if (pObj->isKindOf(szClassName)) {
            return pObj;
        }
        if (pObj->isContainer()) {
            pObj = pObj->getContainerIf()->getUIObjectByClassName(szClassName);
            if (pObj) {
                return pObj;
            }
        }
    }

    return nullptr;
}

CUIObject *CSkinContainer::getUIObjectById(int nId, cstr_t szClassName) {
    if (nId == UID_INVALID) {
        return nullptr;
    }

    // send mouse move message to every ui objects that want to process
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];
        if (pObj->m_id == nId) {
            if (!szClassName) {
                return pObj;
            } else if (pObj->isKindOf(szClassName)) {
                return pObj;
            } else {
                return nullptr;
            }
        } else if (pObj->isContainer()) {
            pObj = pObj->getContainerIf()->getUIObjectById(nId, szClassName);
            if (pObj) {
                return pObj;
            }
        }
    }

    return nullptr;
}

CUIObject *CSkinContainer::getUIObjectById(cstr_t szId, cstr_t szClassName) {
    return getUIObjectById(getIDByName(szId), szClassName);
}

bool CSkinContainer::enableUIObject(int nId, bool bEnable, bool bRedraw) {
    CUIObject *pObj = getUIObjectById(nId, nullptr);
    if (!pObj) {
        DBG_LOG1("UIObject: %s doesn't exist.", getSkinFactory()->getStringOfID(nId).c_str());
        return false;
    }

    if (pObj->m_enable != bEnable) {
        pObj->m_enable = bEnable;

        if (bRedraw) {
            pObj->invalidate();
        }
    }

    return true;
}

bool CSkinContainer::enableUIObject(cstr_t szId, bool bEnable, bool bRedraw) {
    return enableUIObject(getIDByName(szId), bEnable, bRedraw);
}

bool CSkinContainer::setUIObjectProperty(int nId, cstr_t szProperty, cstr_t szValue) {
    CUIObject *pObj = getUIObjectById(nId, nullptr);
    if (!pObj) {
        DBG_LOG1("UIObject: %s doesn't exist.", getSkinFactory()->getStringOfID(nId).c_str());
        return false;
    }

    return pObj->setProperty(szProperty, szValue);
}

bool CSkinContainer::setUIObjectProperty(cstr_t szId, cstr_t szProperty, cstr_t szValue) {
    return setUIObjectProperty(getSkinFactory()->getIDByName(szId), szProperty, szValue);
}

void CSkinContainer::setUIObjectVisible(int nId, bool bVisible, bool bRedraw) {
    CUIObject *pObj = getUIObjectById(nId, nullptr);
    if (!pObj) {
        DBG_LOG1("UIObject: %s doesn't exist.", getSkinFactory()->getStringOfID(nId).c_str());
        return;
    }

    pObj->setVisible(bVisible, bRedraw);
}

void CSkinContainer::setUIObjectVisible(cstr_t szId, bool bVisible, bool bRedraw) {
    setUIObjectVisible(getSkinFactory()->getIDByName(szId), bVisible, bRedraw);
}

void CSkinContainer::invalidateUIObject(int nId) {
    CUIObject *pObj = getUIObjectById(nId, nullptr);
    if (!pObj) {
        DBG_LOG1("UIObject: %s doesn't exist.", getSkinFactory()->getStringOfID(nId).c_str());
        return;
    }

    invalidateUIObject(pObj);
}

void CSkinContainer::invalidateUIObject(cstr_t szId) {
    invalidateUIObject(getSkinFactory()->getIDByName(szId));
}

string CSkinContainer::getUIObjectText(int nId) {
    CUIObject *pObj = getUIObjectById(nId, nullptr);
    if (!pObj) {
        DBG_LOG1("UIObject: %s doesn't exist.", getSkinFactory()->getStringOfID(nId).c_str());
        return "";
    }

    return pObj->getText();
}

string CSkinContainer::getUIObjectText(cstr_t szId) {
    return getUIObjectText(getSkinFactory()->getIDByName(szId));
}

bool CSkinContainer::getUIObjectRect(int nId, CRect &rc) {
    CUIObject *p = getUIObjectById(nId, nullptr);
    if (!p) {
        return false;
    }

    rc = p->m_rcObj;
    return true;
}

bool CSkinContainer::getUIObjectRect(cstr_t szId, CRect &rc) {
    return getUIObjectRect(getIDByName(szId), rc);
}

void CSkinContainer::setUIObjectText(int nId, cstr_t szText, bool bRedraw) {
    CUIObject *pObj = getUIObjectById(nId, nullptr);
    if (!pObj) {
        DBG_LOG1("UIObject: %s doesn't exist.", getSkinFactory()->getStringOfID(nId).c_str());
        return;
    }

    pObj->setProperty(SZ_PN_TEXT, szText);
    if (bRedraw) {
        pObj->invalidate();
    }
}

void CSkinContainer::setUIObjectText(cstr_t szId, cstr_t szText, bool bRedraw) {
    setUIObjectText(getSkinFactory()->getIDByName(szId), szText, bRedraw);
}

void CSkinContainer::checkButton(int nId, bool bCheck) {
    CSkinNStatusButton *pButton = (CSkinNStatusButton*)getUIObjectById(nId, CSkinNStatusButton::className());
    if (pButton) {
        pButton->setStatus(bCheck);
    }
}

void CSkinContainer::checkButton(cstr_t szId, bool bCheck) {
    checkButton(getIDByName(szId), bCheck);
}

bool CSkinContainer::isButtonChecked(int nId) {
    CSkinNStatusButton *pButton = (CSkinNStatusButton*)getUIObjectById(nId, CSkinNStatusButton::className());
    if (pButton) {
        return pButton->getStatus();
    }

    return false;
}

bool CSkinContainer::isButtonChecked(cstr_t szId) {
    return isButtonChecked(getIDByName(szId));
}

void CSkinContainer::checkToolbarButton(cstr_t szToolbarId, cstr_t szButtonId, bool bCheck) {
    checkToolbarButton(getIDByName(szToolbarId), getIDByName(szButtonId), bCheck);
}

void CSkinContainer::checkToolbarButton(int nToolbarId, int nButtonId, bool bCheck) {
    CSkinToolbar *pToolbar = (CSkinToolbar*)getUIObjectById(
        nToolbarId, CSkinToolbar::className());
    if (pToolbar) {
        pToolbar->setCheck(nButtonId, bCheck);
    }
}

void CSkinContainer::setFocusUIObject(int nId) {
    CUIObject *pObj = getUIObjectById(nId, nullptr);
    if (pObj) {
        pObj->setFocus();
    }
}

void CSkinContainer::setFocusUIObject(cstr_t szId) {
    setFocusUIObject(getIDByName(szId));
}

CUIObject *CSkinContainer::getFocusUIObject() {
    if (m_nFocusUIObj >= 0 && m_nFocusUIObj < (int)m_vUIObjs.size()) {
        CUIObject *pObj = m_vUIObjs[m_nFocusUIObj];
        if (pObj->getContainerIf()) {
            auto child = pObj->getContainerIf()->getFocusUIObject();
            if (child) {
                return child;
            }
        }

        if (pObj->needMsgKey()) {
            return pObj;
        }
    }

    return nullptr;
}

void CSkinContainer::setFocusChild(CUIObject *pNewFocusObj) {
    CUIObject *pOldFocusObj = nullptr;
    if (m_nFocusUIObj >= 0 && m_nFocusUIObj < (int)m_vUIObjs.size()) {
        pOldFocusObj = m_vUIObjs[m_nFocusUIObj];
    }

    if (pOldFocusObj == pNewFocusObj) {
        return;
    }

    int nNewFocusObj = -1;

    // get new Focus Object
    if (pNewFocusObj != nullptr) {
        for (nNewFocusObj = 0; nNewFocusObj < (int)m_vUIObjs.size(); nNewFocusObj++) {
            if (m_vUIObjs[nNewFocusObj] == pNewFocusObj) {
                break;
            }
        }
        if (nNewFocusObj >= (int)m_vUIObjs.size()) {
            assert(0 && "new focus UIObject is not the child of this container");
            return;
        }
    }

    m_nFocusUIObj = nNewFocusObj;

    if (pOldFocusObj) {
        // Kill Focus for old child.
        if (pOldFocusObj->needMsgKey()) {
            pOldFocusObj->onKillFocus();
        } else if (pOldFocusObj->isContainer()) {
            pOldFocusObj->getContainerIf()->setFocusChild(nullptr);
        } else {
            pOldFocusObj->onKillFocus();
        }
    }

    if (pNewFocusObj) {
        // Let parent container set focus child as "this"
        if (pNewFocusObj->needMsgKey()) {
            pNewFocusObj->onSetFocus();
        }
        if (m_pContainer) {
            m_pContainer->setFocusChild(this);
        }
    }
}

int CSkinContainer::focusToNext() {
    if (m_vUIObjs.empty()) {
        assert(m_nFocusUIObj == -1);
        return false;
    }

    int i = m_nFocusUIObj;
    if (m_nFocusUIObj < 0 || m_nFocusUIObj >= (int)m_vUIObjs.size()) {
        i = 0;
    } else if (m_vUIObjs[m_nFocusUIObj]->needMsgKey()) {
        i++;
    }

    for (int k = 0; k <= (int)m_vUIObjs.size(); k++, i++) {
        CUIObject *pObj = m_vUIObjs[i % m_vUIObjs.size()];

        if (!pObj->isVisible()) {
            continue;
        }

        if (pObj->needMsgKey()) {
            pObj->setFocus();
            return true;
        } else if (pObj->isContainer()) {
            if (pObj->getContainerIf()->focusToNext()) {
                return true;
            }
        }
    }

    if (!m_pContainer) {
        CUIObject *pObj = getFocusUIObject();
        if (pObj) {
            setFocusChild(nullptr);
        }
    }

    return false;
}

int CSkinContainer::focusToPrev() {
    if (m_vUIObjs.empty()) {
        assert(m_nFocusUIObj == -1);
        return false;
    }

    int i = m_nFocusUIObj;
    if (m_nFocusUIObj < 0 || m_nFocusUIObj >= (int)m_vUIObjs.size()) {
        i = 0;
    } else if (m_vUIObjs[m_nFocusUIObj]->needMsgKey()) {
        i--;
    }

    for (int k = 0; k <= (int)m_vUIObjs.size(); k++, i--) {
        CUIObject *pObj = m_vUIObjs[(i + m_vUIObjs.size()) % m_vUIObjs.size()];

        if (!pObj->isVisible()) {
            continue;
        }

        if (pObj->needMsgKey()) {
            pObj->setFocus();
            return true;
        } else if (pObj->isContainer()) {
            if (pObj->getContainerIf()->focusToPrev()) {
                return true;
            }
        }
    }

    if (!m_pContainer) {
        CUIObject *pObj = getFocusUIObject();
        if (pObj) {
            setFocusChild(nullptr);
        }
    }

    return false;
}

bool CSkinContainer::removeUIObject(CUIObject *pObj, bool bFree) {
    for (int i = 0; i < int(m_vUIObjs.size()); i++) {
        CUIObject *p = m_vUIObjs[i];
        if (p == pObj) {
            // found it,
            m_pSkin->unregisterTimerObject(pObj);
            m_pSkin->onRemoveUIObj(pObj);
            m_vUIObjs.erase(m_vUIObjs.begin() + i);
            if (bFree) {
                delete pObj;
            }

            // Change UIObject focus, if possible.
            if (i == m_nFocusUIObj) {
                m_nFocusUIObj = -1;
                m_pSkin->getRootContainer()->focusToNext();
            }

            return true;
        } else if (p->isContainer()) {
            if (p->getContainerIf()->removeUIObject(pObj, bFree)) {
                return true;
            }
        }
    }

    return false;
}

CUIObject *CSkinContainer::removeUIObjectById(int nId) {
    if (nId == UID_INVALID) {
        return nullptr;
    }

    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *pObj = m_vUIObjs[i];

        if (pObj->m_id == nId) {
            m_pSkin->unregisterTimerObject(pObj);
            m_pSkin->onRemoveUIObj(pObj);
            m_vUIObjs.erase(m_vUIObjs.begin() + i);

            // Change UIObject focus, if possible.
            if (i == m_nFocusUIObj) {
                m_nFocusUIObj = -1;
                m_pSkin->getRootContainer()->focusToNext();
            }

            return pObj;
        } else if (pObj->isContainer()) {
            CUIObject *p = pObj->getContainerIf()->removeUIObjectById(nId);
            if (p) {
                return p;
            }
        }
    }

    return nullptr;
}


bool CSkinContainer::addUIObject(CUIObject *pObj) {
    assert(pObj);
    if (pObj == nullptr) {
        return false;
    }

    pObj->m_autoUniID = m_pSkin->allocAutoUniID();
    m_vUIObjs.push_back(pObj);

    onAdd(pObj);

    return true;
}

bool CSkinContainer::insertUIObjectAt(CUIObject *pObjWhere, CUIObject *pObj) {
    assert(pObj);
    if (pObj == nullptr) {
        return false;
    }

    VecUIObjects::iterator it;

    pObj->m_autoUniID = m_pSkin->allocAutoUniID();
    for (it = m_vUIObjs.begin(); it != m_vUIObjs.end(); ++it) {
        if (*it == pObjWhere) {
            break;
        }
    }
    m_vUIObjs.insert(it, pObj);

    onAdd(pObj);

    return true;
}

bool CSkinContainer::recalculateVarValue(CFormula &form, int &nRetValue) {
    FORMULA_VAR     vars[] = {
        {'w', m_rcObj.width()},
        {'h', m_rcObj.height()},
        {0, 0}
    };

    return form.calCualteValue(vars, nRetValue);
}

void CSkinContainer::recalculateUIObjSizePos(CUIObject *pObj) {
    // recalculate all ui objects' position
    FORMULA_VAR     vars[] = {
        {'w', m_rcObj.width()},
        {'h', m_rcObj.height()},
        {0, 0}
    };

    // recalculate every UIObjects' position on back buffer one by one
    pObj->onMeasureSizePos(vars);
    pObj->onSize();
}

bool CSkinContainer::isChild(CUIObject *pObj) {
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *p = m_vUIObjs[i];

        if (pObj == p) {
            return true;
        }

        if (p->isContainer()) {
            if (p->getContainerIf()->isChild(pObj)) {
                return true;
            }
        }
    }

    return false;
}

void CSkinContainer::invalidateUIObject(CUIObject *pObj) {
#ifdef _WIN32
    if (m_pContainer) {
        m_pContainer->invalidateUIObject(pObj);
    } else {
        m_pSkin->invalidateUIObject(pObj);
    }
#else // #ifdef _WIN32
    m_pSkin->invalidateUIObject(pObj);
#endif // #ifdef _WIN32
}

void CSkinContainer::updateMemGraphicsToScreen(const CRect* lpRect, CUIObject *pObjCallUpdate) {
    if (m_pContainer) {
        m_pContainer->updateMemGraphicsToScreen(lpRect, this);
    } else {
        m_pSkin->updateMemGraphicsToScreen(lpRect);
    }
}

void CSkinContainer::switchToPage(cstr_t szPageClass, bool bWaitResultOfNextPage, int nRequestCodeOfNextPage, bool bAnimation) {
    if (m_vStackPageView.size() > 100) {
        DBG_LOG1("Stack of Page View is too large, clear it: %d", m_vStackPageView.size());
        m_vStackPageView.empty();
    }

    PageViewItem pvToHide(nullptr, false, 0);
    if (!m_vStackPageView.empty()) {
        pvToHide = m_vStackPageView.top();
    }

    CSkinContainer *pToActivate = (CSkinContainer *)getChildByClass(szPageClass);
    if (!pToActivate) {
        ERR_LOG1("Switch to an inexistant Page: %s", szPageClass);
        return;
    }

    PageViewItem pvToActivate(pToActivate, bWaitResultOfNextPage, nRequestCodeOfNextPage);
    m_vStackPageView.push(pvToActivate);

    bool bIsNewPageAtLeft = isPageAtLeft(pToActivate, pvToHide.pContainerPage);
    if (pvToHide.pContainerPage) {
        pvToHide.pContainerPage->setVisibleEx(false, bAnimation, bIsNewPageAtLeft ? AD_RIGHT: AD_LEFT);
    }
    pToActivate->onSwitchTo();
    pToActivate->setVisibleEx(true, bAnimation, bIsNewPageAtLeft ? AD_LEFT : AD_RIGHT);
}

void CSkinContainer::switchToLastPage(int nResultCodeOfPage, bool bAnimation) {
    if (m_vStackPageView.size() >= 2) {
        PageViewItem pvToHide = m_vStackPageView.top();
        m_vStackPageView.pop();
        PageViewItem pvToActivate = m_vStackPageView.top();

        bool bIsNewPageAtLeft = isPageAtLeft(pvToActivate.pContainerPage, pvToHide.pContainerPage);

        // Hide page from !bIsNewPageAtLeft
        pvToHide.pContainerPage->setVisibleEx(false, bAnimation, bIsNewPageAtLeft ? AD_RIGHT : AD_LEFT);

        if (pvToActivate.bWaitResultOfNextPage) {
            pvToActivate.pContainerPage->onPageViewResult(pvToActivate.nRequestCodeOfNextPage, nResultCodeOfPage);
        }
        pvToActivate.pContainerPage->onSwitchTo();
        pvToActivate.pContainerPage->setVisibleEx(true, bAnimation, bIsNewPageAtLeft ? AD_LEFT : AD_RIGHT);
    }
}

void CSkinContainer::setExPool(cstr_t szProperty, cstr_t szValue) {
    m_pSkin->m_mapExchangePool[szProperty] = szValue;
}

void CSkinContainer::setExPool(cstr_t szProperty, int value) {
    m_pSkin->m_mapExchangePool[szProperty] = stringPrintf("%d", value).c_str();
}

string CSkinContainer::getExPoolStr(cstr_t szProperty, cstr_t szDefault) {
    MapStrings::iterator it = m_pSkin->m_mapExchangePool.find(szProperty);
    if (it != m_pSkin->m_mapExchangePool.end()) {
        return (*it).second;
    } else {
        return szDefault;
    }
}

int CSkinContainer::getExPoolInt(cstr_t szProperty, int nDefault) {
    MapStrings::iterator it = m_pSkin->m_mapExchangePool.find(szProperty);
    if (it != m_pSkin->m_mapExchangePool.end()) {
        return atoi((*it).second.c_str());
    } else {
        return nDefault;
    }
}

bool CSkinContainer::getExPoolBool(cstr_t szProperty, bool bDefault) {
    MapStrings::iterator it = m_pSkin->m_mapExchangePool.find(szProperty);
    if (it != m_pSkin->m_mapExchangePool.end()) {
        return isTRUE((*it).second.c_str());
    } else {
        return bDefault;
    }
}

#ifdef _DEBUG
void CSkinContainer::dumpUIObject(int &nDeep, CPoint *pt) {
    string strDeep;
    strDeep.resize(nDeep * 2, ' ');
    nDeep++;

    for (uint32_t i = 0; i < m_vUIObjs.size(); i++) {
        CUIObject *p = m_vUIObjs[i];
        auto str = stringPrintf("%s id=%s(%d), name=%s visible=%d rc=%d,%d,%d,%d", + p->getClassName(),
            m_pSkin->getSkinFactory()->getStringOfID(p->getID()).c_str(), p->getID(),
            p->m_strName.c_str(), p->m_visible, p->m_rcObj.left, p->m_rcObj.top, p->m_rcObj.width(), p->m_rcObj.height());
        if (pt == nullptr || p->isPtIn(*pt)) {
            DBG_LOG2("%s%s", strDeep.c_str(), str.c_str());
        }
        if (p->isContainer()) {
            p->getContainerIf()->dumpUIObject(nDeep, pt);
        }
    }

    nDeep--;
}
#endif

bool CSkinContainer::isPageAtLeft(CSkinContainer *pPage, CSkinContainer *pPage2) {
    if (pPage2 == nullptr) {
        return false;
    }

    int nPos = -1, nPos2 = (int)m_vUIObjs.size();

    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        if (m_vUIObjs[i] == pPage) {
            nPos = i;
        } else if (m_vUIObjs[i] == pPage2) {
            nPos2 = i;
        }
    }

    return nPos < nPos2;
}

void CSkinContainer::onAdd(CUIObject *pObj) {
    m_pSkin->onAddUIObj(pObj);

    if (!m_bCreated) {
        return;
    }

    // Only when parent is opened, onCreate will be called.

    recalculateUIObjSizePos(pObj);

    if (pObj->isContainer()) {
        pObj->getContainerIf()->dispatchOnCreateMsg();
    } else {
        assert(!pObj->m_bCreated);
        pObj->m_bCreated = true;
        pObj->onCreate();
    }

    if (m_pSkin->getFocusUIObj() == nullptr && pObj->needMsgKey() && pObj->isEnable()
        && pObj->isVisible() && pObj->isParentVisible()) {
        pObj->setFocus();
    }
}

#ifdef _SKIN_EDITOR_
bool CSkinContainer::hasXMLChild() {
    return !m_vUIObjs.empty();
}

void CSkinContainer::onToXMLChild(CXMLWriter &xmlStream) {
    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        m_vUIObjs[i]->toXML(xmlStream);
    }
}
#endif // _SKIN_EDITOR_

UIOBJECT_CLASS_NAME_IMP(CSkinClientArea, "ClientArea")
