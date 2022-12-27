#include "SkinTypes.h"
#include "Skin.h"
#include "SkinDecorativeContainer.h"


UIOBJECT_CLASS_NAME_IMP(CSkinDecorativeContainer, "DecorativeContainer")


CSkinDecorativeContainer::CSkinDecorativeContainer() {
}


CSkinDecorativeContainer::~CSkinDecorativeContainer() {
}


void CSkinDecorativeContainer::invalidateUIObject(CUIObject *pObj) {
    // Redraw this whole container.

    if (m_pContainer) {
        m_pContainer->invalidateUIObject(this);
    } else {
        m_pSkin->invalidateUIObject(this);
    }
}

//
// Redraw this whole container.
//
void CSkinDecorativeContainer::updateMemGraphicsToScreen(const CRect* lpRect, CUIObject *pObjCallUpdate) {
    assert(lpRect);

    CRawGraph *pGraphMem = nullptr;
    CRect rcClipBoxOrg;

    // Check for UIObject overlay, and redraw them?
    for (VecUIObjects::iterator it = m_vUIObjs.begin(); it != m_vUIObjs.end(); ++it) {
        CUIObject *pObj = *it;
        CRect rc;

        if (pObj != pObjCallUpdate && pObj->isVisible()
            && rc.intersect(pObj->m_rcObj, *lpRect)) {
            if (!pGraphMem) {
                pGraphMem = m_pContainer->getMemGraph();
                pGraphMem->getClipBoundBox(rcClipBoxOrg);
                pGraphMem->setClipBoundBox(*lpRect);
            }

            CRawGraph::COpacityBlendAutoRecovery opacityAR(pGraphMem, pObj->getOpacity());

            pObj->tryToCallInitialUpdate();
            pObj->draw(pGraphMem);
        }
    }

    if (pGraphMem) {
        pGraphMem->resetClipBoundBox(rcClipBoxOrg);
    }

    if (m_pContainer) {
        m_pContainer->updateMemGraphicsToScreen(lpRect, this);
    } else {
        m_pSkin->updateMemGraphicsToScreen(lpRect);
    }
}
