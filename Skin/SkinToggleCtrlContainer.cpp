#include "SkinTypes.h"
#include "Skin.h"
#include "SkinToggleCtrlContainer.h"


UIOBJECT_CLASS_NAME_IMP(CSkinToggleCtrlContainer, "ToggleCtrlContainer")

CSkinToggleCtrlContainer::CSkinToggleCtrlContainer(void) : m_memGraph(1.0) {
    m_nAnimateDuration = 800;
    m_nToggleToCtrl = UID_INVALID;
    m_nActiveCtrl = UID_INVALID;
    m_nIDTimerAnimation = 0;
    m_timeBeginAni = 0;
}

CSkinToggleCtrlContainer::~CSkinToggleCtrlContainer(void) {
}

CRawGraph *CSkinToggleCtrlContainer::getMemGraph() {
    if (m_nIDTimerAnimation == 0) {
        return CSkinContainer::getMemGraph();
    }

    if (m_memGraph.isValid() && (m_rcObj.width() != m_memGraph.width() || m_rcObj.height() != m_memGraph.height())) {
        m_memGraph.destroy();
    }

    if (!m_memGraph.isValid()) {
        m_memGraph.create(m_rcObj.width(), m_rcObj.height(), m_pSkin->getHandleHolder());
        m_memGraph.resetOrigin(CPoint(-m_rcObj.left, -m_rcObj.top));
    }

    return &m_memGraph;
}

void CSkinToggleCtrlContainer::invalidateUIObject(CUIObject *pObj) {
    if (m_pContainer) {
        m_pContainer->invalidateUIObject(this);
    } else {
        m_pSkin->invalidateUIObject(this);
    }
}

void CSkinToggleCtrlContainer::updateMemGraphicsToScreen(const CRect* lpRect, CUIObject *pObjCallUpdate) {
    if (m_nIDTimerAnimation != 0) {
        invalidateUIObject(this);
    } else {
        CSkinContainer::updateMemGraphicsToScreen(lpRect, pObjCallUpdate);
    }
}

void CSkinToggleCtrlContainer::draw(CRawGraph *canvas) {
    if (m_nIDTimerAnimation) {
        getMemGraph();

        m_memGraph.resetClipBoundBox(m_rcObj);

        // show child controls in animation
        // draw active control
        CUIObject *pObj = getUIObjectById(m_nActiveCtrl, nullptr);
        if (pObj) {
            // m_pSkin->redrawBackground(canvas, m_rcObj);
            pObj->tryToCallInitialUpdate();
            pObj->draw(canvas);
        }

        pObj = getUIObjectById(m_nToggleToCtrl, nullptr);
        if (pObj)
        {
            int opaque = int(getTickCount() - m_timeBeginAni) * 255 / m_nAnimateDuration;
            if (opaque > 255)
                opaque = 255;
            opaque = canvas->setOpacityPainting(opaque);

            // draw toggle to control
            if (isUseParentBg() || pObj->isUseParentBg())
                m_pContainer->redrawBackground(&m_memGraph, m_rcObj);
            CRawGraph::COpacityBlendAutoRecovery opacityAR(&m_memGraph, pObj->getOpacity());
            pObj->tryToCallInitialUpdate();
            pObj->draw(&m_memGraph);
            opacityAR.recover();

            // draw final result
            canvas->bltImage(m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), m_memGraph.getRawBuff(), 0, 0, BPM_COPY);
            canvas->setOpacityPainting(opaque);
        }
    } else {
        CSkinContainer::draw(canvas);
    }
}

bool CSkinToggleCtrlContainer::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinContainer::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, "CmdToggle")) {
        m_nToggleToCtrl = m_pSkin->getSkinFactory()->getIDByName(szValue);
        if (getUIObjectById(m_nToggleToCtrl, nullptr)) {
            m_nIDTimerAnimation = m_pSkin->registerTimerObject(this, 100);

            // Hide all object
            CUIObject *pObj = getUIObjectById(m_nActiveCtrl, nullptr);
            if (pObj) {
                pObj->setVisible(false);
            }

            m_timeBeginAni = getTickCount();
        } else {
            m_nToggleToCtrl = UID_INVALID;
        }
    } else {
        return false;
    }

    return true;
}

void CSkinToggleCtrlContainer::onCreate() {
    CSkinContainer::onCreate();

    m_nActiveCtrl = 0;
}

void CSkinToggleCtrlContainer::onTimer(int nId) {
    if (nId == m_nIDTimerAnimation) {
        if (getTickCount() >= m_timeBeginAni + m_nAnimateDuration) {
            // stop animation timer
            m_pSkin->unregisterTimerObject(this, m_nIDTimerAnimation);
            m_nIDTimerAnimation = 0;

            // Hide old Object
            for (size_t i = 0; i < m_vUIObjs.size(); i++) {
                CUIObject *pObj = m_vUIObjs[i];
                if (pObj->m_id != nId) {
                    pObj->setVisible(false);
                }
            }
            m_nActiveCtrl = UID_INVALID;

            // show new object
            CUIObject *pObj = getUIObjectById(m_nToggleToCtrl, nullptr);
            if (pObj) {
                pObj->setVisible(true);
                m_nActiveCtrl = m_nToggleToCtrl;
            }
        }

        invalidate();
    } else {
        CSkinContainer::onTimer(nId);
    }
}
