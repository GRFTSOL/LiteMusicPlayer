#include "SkinTypes.h"
#include "Skin.h"
#include "SkinToggleCtrlContainer.h"


const int DEFAULT_ANIMATION_DURATION = 500;

class CObjAnimationNewer {
public:
    virtual ~CObjAnimationNewer() {}
    virtual CAnimationMotion *newObject(int nId) = 0;

};

template<class _ObjClass>
class _CObjAnimationNewer : public CObjAnimationNewer {
public:
    virtual CAnimationMotion *newObject(int nId) {
        return new _ObjClass(nId);
    }

};

class CAnimateMoveMotion : public CAnimationMotion {
public:
    CAnimateMoveMotion(int nId, AnimateDirection animateDirection)
    : CAnimationMotion(nId), m_animateDirection(animateDirection) { }

protected:
    int                         m_posStart;
    AnimateDirection            m_animateDirection;

};

template<class _ObjClass, AnimateDirection direction>
class _CObjAnimationMoveNewer : public CObjAnimationNewer {
public:
    virtual CAnimationMotion *newObject(int nId) {
        return new _ObjClass(nId, direction);
    }

};

class CAnimateMoveInMotion : public CAnimateMoveMotion {
public:
    CAnimateMoveInMotion(int nId, AnimateDirection animateDirection)
    : CAnimateMoveMotion(nId, animateDirection) { }

    virtual void animate(AnimateState as, int nMotionPercent, CUIObject *pObj) {
        if (as == AS_MOVE || as == AS_DONE) {
            if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT) {
                calculatePosLatest(pObj->m_rcObj.left, nMotionPercent);

                pObj->m_rcObj.right = m_posLatest + pObj->m_rcObj.width();
                pObj->m_rcObj.left = m_posLatest;
            } else {
                calculatePosLatest(pObj->m_rcObj.top, nMotionPercent);

                pObj->m_rcObj.bottom = m_posLatest + pObj->m_rcObj.height();
                pObj->m_rcObj.top = m_posLatest;
            }

            pObj->onSize();
        } else if (as == AS_INIT) {
            pObj->getParent()->recalculateUIObjSizePos(pObj);

            if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT) {
                if (m_animateDirection == AD_LEFT) {
                    initStartPos(0 - pObj->m_rcObj.right, pObj->m_rcObj.left);
                } else {
                    initStartPos(pObj->getSkinWnd()->getBoundBox().width(), pObj->m_rcObj.left);
                }

                pObj->m_rcObj.right = m_posStart + pObj->m_rcObj.width();
                pObj->m_rcObj.left = m_posStart;
            } else {
                if (m_animateDirection == AD_TOP) {
                    initStartPos(0 - pObj->m_rcObj.bottom, pObj->m_rcObj.top);
                } else {
                    initStartPos(pObj->getSkinWnd()->getBoundBox().height(), pObj->m_rcObj.top);
                }

                pObj->m_rcObj.bottom = m_posStart + pObj->m_rcObj.height();
                pObj->m_rcObj.top = m_posStart;
            }
            pObj->onSize();
            pObj->setVisible(true);
        }
    }

    void initStartPos(int posStart, int posTarget) {
        m_posLatest = m_posStart = posStart;
        m_posTarget = posTarget;
    }

    void calculatePosLatest(int pos, int nMotionPercent) {
        if (m_posLatest != pos) {
            m_posTarget = pos;
        }

        // m_posLatest = m_posStart + (m_posTarget - m_posStart) * nMotionPercent / 100;
        m_posLatest = m_posStart + (m_posTarget - m_posStart) * (nMotionPercent * nMotionPercent) / (100 * 100);
    }

    int                         m_posTarget, m_posLatest;

};

class CAnimateMoveOutMotion : public CAnimateMoveMotion {
public:
    CAnimateMoveOutMotion(int nId, AnimateDirection animateDirection)
    : CAnimateMoveMotion(nId, animateDirection) { }

    virtual void animate(AnimateState as, int nMotionPercent, CUIObject *pObj) {
        if (as == AS_MOVE) {
            int posTarget, posNew;
            if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT) {
                if (m_animateDirection == AD_LEFT) {
                    posTarget = -pObj->m_rcObj.width();
                } else {
                    posTarget = pObj->getSkinWnd()->getBoundBox().width();
                }
                posNew = calculatePosLatest(posTarget, nMotionPercent);

                pObj->m_rcObj.right = posNew + pObj->m_rcObj.width();
                pObj->m_rcObj.left = posNew;
            } else {
                if (m_animateDirection == AD_TOP) {
                    posTarget = -pObj->m_rcObj.height();
                } else {
                    posTarget = pObj->getSkinWnd()->getBoundBox().height();
                }
                posNew = calculatePosLatest(posTarget, nMotionPercent);

                pObj->m_rcObj.bottom = posNew + pObj->m_rcObj.height();
                pObj->m_rcObj.top = posNew;
            }
            pObj->onSize();
        } else if (as == AS_INIT) {
            if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT) {
                m_posStart = pObj->m_rcObj.left;
            } else {
                m_posStart = pObj->m_rcObj.top;
            }
        } else {
            pObj->setVisible(false);
            if (m_animateDirection == AD_LEFT || m_animateDirection == AD_RIGHT) {
                pObj->m_rcObj.right = m_posStart + pObj->m_rcObj.width();
                pObj->m_rcObj.left = m_posStart;
            } else {
                pObj->m_rcObj.top = m_posStart;
                pObj->m_rcObj.bottom = m_posStart + pObj->m_rcObj.height();
            }
        }
    }

    int calculatePosLatest(int posTarget, int nMotionPercent) {
        // return m_posStart + (posTarget - m_posStart) * nMotionPercent / 100;
        return m_posStart + (posTarget - m_posStart) * (nMotionPercent * nMotionPercent) / (100 * 100);
    }

};

//////////////////////////////////////////////////////////////////////////

class CAnimationFadeOutMotion : public CAnimationMotion {
public:
    CAnimationFadeOutMotion(int nId) : CAnimationMotion(nId) { }

    virtual void animate(AnimateState as, int nMotionPercent, CUIObject *pObj) {
        if (as == AS_MOVE) {
            pObj->m_opacity = 255 - 255 * nMotionPercent / 100;
        } else if (as == AS_INIT) {
            if (pObj->isVisible()) {
                pObj->m_opacity = 255;
            }
        } else {
            pObj->setVisible(false);
        }
    }

};

class CAnimationFadeInMotion : public CAnimationMotion {
public:
    CAnimationFadeInMotion(int nId) : CAnimationMotion(nId) { }

    virtual void animate(AnimateState as, int nMotionPercent, CUIObject *pObj) {
        if (as == AS_MOVE) {
            if (m_bAnimate) {
                pObj->m_opacity = 255 * nMotionPercent / 100;
            }
        } else if (as == AS_INIT) {
            if (!pObj->isVisible()) {
                pObj->setVisible(true);
                pObj->m_opacity = 0;
                m_bAnimate = true;
            } else {
                m_bAnimate = false;
            }
        } else {
            pObj->m_opacity = 255;
        }
    }

    bool                        m_bAnimate;

};

class CAnimationToggleMotion : public CAnimationMotion {
public:
    CAnimationToggleMotion(int nId) : CAnimationMotion(nId) { }

    virtual void animate(AnimateState as, int nMotionPercent, CUIObject *pObj) {
        if (as == AS_INIT) {
            if (!pObj->isVisible()) {
                CUIObject *pObjToggle = pObj->getSkinWnd()->getUIObjectByClassName(CSkinToggleCtrlContainer::className());
                if (pObjToggle) {
                    pObjToggle->setProperty("CmdToggle",
                        pObj->getSkinWnd()->getSkinFactory()->getStringOfID(pObj->getID()).c_str());
                }
            }
        }
    }

};

class cAnimationResizeCtrlMotion : public CAnimationMotion {
public:
    cAnimationResizeCtrlMotion(int nId, cstr_t szLeft, cstr_t szTop, cstr_t szWidth, cstr_t szHeight)
    : CAnimationMotion(nId) {
        formLeft.setFormula(szLeft);
        formTop.setFormula(szTop);
        formWidth.setFormula(szWidth);
        formHeight.setFormula(szHeight);
    }

    virtual void animate(AnimateState as, int nMotionPercent, CUIObject *pObj) {
        if (as == AS_MOVE || as == AS_DONE) {
            CRect rc;
            int value;
            if (!formLeft.empty() && pObj->getParent()->recalculateVarValue(formLeft, value)) {
                rc.left = rcObjOrg.left + (value - rcObjOrg.left) * (nMotionPercent * nMotionPercent) / (100 * 100);
            } else {
                rc.left = rcObjOrg.left;
            }

            if (!formTop.empty() && pObj->getParent()->recalculateVarValue(formTop, value)) {
                rc.top = rcObjOrg.top + (value - rcObjOrg.top) * (nMotionPercent * nMotionPercent) / (100 * 100);
            } else {
                rc.top = rcObjOrg.top;
            }

            if (!formWidth.empty() && pObj->getParent()->recalculateVarValue(formWidth, value)) {
                rc.right = rc.left + rcObjOrg.width() + (value - rcObjOrg.width()) * (nMotionPercent * nMotionPercent) / (100 * 100);
            } else {
                rc.right = rcObjOrg.right;
            }

            if (!formHeight.empty() && pObj->getParent()->recalculateVarValue(formHeight, value)) {
                rc.bottom = rc.top + rcObjOrg.height() + (value - rcObjOrg.height()) * (nMotionPercent * nMotionPercent) / (100 * 100);
            } else {
                rc.bottom = rcObjOrg.bottom;
            }

            pObj->m_rcObj = rc;
            pObj->onSize();
        } else if (as == AS_INIT) {
            rcObjOrg = pObj->m_rcObj;
            if (!formLeft.empty()) {
                pObj->m_formLeft.setFormula(formLeft.getFormula());
            }
            if (!formTop.empty()) {
                pObj->m_formTop.setFormula(formTop.getFormula());
            }
            if (!formWidth.empty()) {
                pObj->m_formWidth.setFormula(formWidth.getFormula());
            }
            if (!formHeight.empty()) {
                pObj->m_formHeight.setFormula(formHeight.getFormula());
            }
            // pObj->getParent()->recalculateUIObjSizePos(pObj);
        }
    }

    CRect                       rcObjOrg;
    CFormula                    formLeft, formTop, formWidth, formHeight;

};

//////////////////////////////////////////////////////////////////////////
class CAnimationResizeWndMotion : public CAnimationMotion {
public:
    CAnimationResizeWndMotion(CSkinWnd *pSkinWnd, int nResizeToWidth, int nResizeToHeight)
    : CAnimationMotion(ID_INVALID), m_pSkin(pSkinWnd), m_nResizeToWidth(nResizeToWidth), m_nResizeToHeight(nResizeToHeight) {
    }

    virtual void animate(AnimateState as, int nMotionPercent, CUIObject *pObj) {
        if (as == AS_MOVE || as == AS_DONE) {
            if (m_nResizeToWidth != 0 && m_nResizeToHeight != 0
                && m_nWndWidthOrg != m_nResizeToWidth && m_nWndHeightOrg != m_nResizeToHeight) {
                CRect &rc = m_pSkin->getBoundBox();

                m_pSkin->moveWindow(rc.left, rc.top,
                    m_nWndWidthOrg + (m_nResizeToWidth - m_nWndWidthOrg) * nMotionPercent / 100,
                    m_nWndHeightOrg + (m_nResizeToHeight - m_nWndHeightOrg) * nMotionPercent / 100);
            }

        } else if (as == AS_INIT) {
            m_nWndWidthOrg = m_pSkin->getBoundBox().width();
            m_nWndHeightOrg = m_pSkin->getBoundBox().height();
        }

    }

protected:
    int                         m_nResizeToWidth, m_nResizeToHeight;
    int                         m_nWndWidthOrg, m_nWndHeightOrg;
    CSkinWnd                    *m_pSkin;

};

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinAnimationUIObj, "Animation")

CSkinAnimationUIObj::CSkinAnimationUIObj() {
    m_enable = false;
    m_visible = false;
    m_nDuration = DEFAULT_ANIMATION_DURATION;
}

bool CSkinAnimationUIObj::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    CObjAnimationNewer *pNewer = nullptr;

    if (isPropertyName(szProperty, "MoveFromLeft")) {
        pNewer = new _CObjAnimationMoveNewer<CAnimateMoveInMotion, AD_LEFT>();
    } else if (isPropertyName(szProperty, "MoveFromRight")) {
        pNewer = new _CObjAnimationMoveNewer<CAnimateMoveInMotion, AD_RIGHT>();
    } else if (isPropertyName(szProperty, "MoveFromTop")) {
        pNewer = new _CObjAnimationMoveNewer<CAnimateMoveInMotion, AD_TOP>();
    } else if (isPropertyName(szProperty, "MoveFromBottom")) {
        pNewer = new _CObjAnimationMoveNewer<CAnimateMoveInMotion, AD_BOTTOM>();
    } else if (isPropertyName(szProperty, "MoveToLeft")) {
        pNewer = new _CObjAnimationMoveNewer<CAnimateMoveOutMotion, AD_LEFT>();
    } else if (isPropertyName(szProperty, "MoveToRight")) {
        pNewer = new _CObjAnimationMoveNewer<CAnimateMoveOutMotion, AD_RIGHT>();
    } else if (isPropertyName(szProperty, "MoveToTop")) {
        pNewer = new _CObjAnimationMoveNewer<CAnimateMoveOutMotion, AD_TOP>();
    } else if (isPropertyName(szProperty, "MoveToBottom")) {
        pNewer = new _CObjAnimationMoveNewer<CAnimateMoveOutMotion, AD_BOTTOM>();
    } else if (isPropertyName(szProperty, "FadeOut")) {
        pNewer = new _CObjAnimationNewer<CAnimationFadeOutMotion>();
    } else if (isPropertyName(szProperty, "FadeIn")) {
        pNewer = new _CObjAnimationNewer<CAnimationFadeInMotion>();
    } else if (isPropertyName(szProperty, "Toggle")) {
        pNewer = new _CObjAnimationNewer<CAnimationToggleMotion>();
    } else if (isPropertyName(szProperty, "ResizeCtrl")) {
        // id, left,top,width,height
        VecStrings vParams;
        strSplit(szValue, ',', vParams);
        trimStr(vParams, ' ');
        for (int i = 0; i + 5 <= (int)vParams.size(); i += 5) {
            int nObjId = m_pSkin->getSkinFactory()->getIDByName(vParams[0].c_str());

            m_listObjAnimation.push_back(
                new cAnimationResizeCtrlMotion(nObjId, vParams[1].c_str(), vParams[2].c_str(),
                vParams[3].c_str(), vParams[4].c_str()));
        }
    } else if (isPropertyName(szProperty, "ResizeWnd")) {
        int nResizeToWidth, nResizeToHeight;
        if (scan2IntX(szValue, nResizeToWidth, nResizeToHeight)) {
            m_listObjAnimation.push_back(new CAnimationResizeWndMotion(m_pSkin, nResizeToWidth, nResizeToHeight));
        }
    } else if (isPropertyName(szProperty, "Duration")) {
        m_nDuration = atoi(szValue);
    } else {
        return false;
    }

    if (pNewer) {
        VecStrings vObjects;
        strSplit(szValue, ',', vObjects);
        trimStr(vObjects, ' ');
        for (int i = 0; i < (int)vObjects.size(); i++) {
            if (vObjects[i].size()) {
                m_listObjAnimation.push_back(
                    pNewer->newObject(m_pSkin->getSkinFactory()->getIDByName(vObjects[i].c_str())));
            }
        }

        delete pNewer;
    }

    return true;
}

void CSkinAnimationUIObj::onAnimate(CSkinAnimation *pAnimation) {
    for (CAnimationMotion *pMotion : m_listObjAnimation) {
        if (pMotion->m_id != ID_INVALID) {
            CUIObject *pObj = m_pSkin->getUIObjectById(pMotion->m_id);
            if (pObj) {
                pMotion->animate(pAnimation->getState(), pAnimation->getMotionPercent(), pObj);
            } else {
                DBG_LOG1("Failed to get UIObject: %s", m_pSkin->getSkinFactory()->getStringOfID(pMotion->m_id).c_str());
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////


AnimateType animateTypeFromString(cstr_t szAnimateType) {
    if (isPropertyName("move", szAnimateType)) {
        return AT_MOVE;
    } else if (isPropertyName("fade", szAnimateType)) {
        return AT_FADE;
    } else {
        return AT_UNKNOWN;
    }
}

CAnimationMotion *newAnimate(AnimateType type, bool bHide, CUIObject *pUIObjTarget, AnimateDirection direction) {
    if (type == AT_MOVE) {
        if (bHide) {
            return new CAnimateMoveOutMotion(0, direction);
        } else {
            return new CAnimateMoveInMotion(0, direction);
        }
    } else {
        assert(type == AT_FADE);
        if (bHide) {
            return new CAnimationFadeOutMotion(0);
        } else {
            return new CAnimationFadeInMotion(0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

CSkinAnimation::CSkinAnimation(CSkinAnimationUIObj *pObjTarget) { // CSkinAnimationUIObj::onAnimate() to process the animation
    assert(pObjTarget);
    m_pObjTarget = pObjTarget;
    m_pMotion = nullptr;
    m_nAnimateId = 0;
    m_nOneMotionTime = pObjTarget->getDuration() / (float)100;

    initStart();
}

CSkinAnimation::CSkinAnimation(CUIObject *pObjTarget, int nAnimateId, int nDurationTime) { // CUIObject::onAnimate() to process the animation
    assert(pObjTarget);
    m_pObjTarget = pObjTarget;
    m_pMotion = nullptr;
    m_nAnimateId = nAnimateId;
    m_nOneMotionTime = nDurationTime / (float)100;

    initStart();
}

CSkinAnimation::CSkinAnimation(CUIObject *pObjTarget, CAnimationMotion *pAnimation, int nDurationTime) { // Construct a temporarily Animation
    assert(pObjTarget && pAnimation);
    m_pObjTarget = pObjTarget;
    m_pMotion = pAnimation;
    m_nAnimateId = 0;
    m_nOneMotionTime = nDurationTime / (float)100;

    initStart();
}

CSkinAnimation::~CSkinAnimation() {
    if (m_pMotion) {
        delete m_pMotion;
    }
}

int CSkinAnimation::getUIObjectID() const {
    return m_pObjTarget->getID();
}

void CSkinAnimation::initStart() {
    m_timeBegin = getTickCount();
    m_nMotionPercent = 0;
    m_nMotionTimeOffset = 0;
    m_animateState = AS_INIT;
}

bool CSkinAnimation::onAnimate() {
    assert(m_pObjTarget);

    m_nMotionTimeOffset = int(getTickCount() - m_timeBegin);
    m_nMotionPercent = int(m_nMotionTimeOffset / m_nOneMotionTime);
    if ((m_nMotionPercent > 100 || m_nMotionPercent  < 0) && m_animateState != AS_INIT) {
        m_nMotionPercent = 100;
        m_nMotionTimeOffset = int(m_nOneMotionTime * 100);
        m_animateState = AS_DONE;
    }

    if (m_pMotion) {
        m_pMotion->animate(m_animateState, m_nMotionPercent, m_pObjTarget);
    } else {
        m_pObjTarget->onAnimate(this);
    }

    if (m_animateState == AS_INIT) {
        m_animateState = AS_MOVE;
    }

    return m_animateState != AS_DONE;
}
