#pragma once

#include "../Utils/UtilsTypes.h"


class CSkinAnimationUIObj;
class CUIObject;

extern const int DEFAULT_ANIMATION_DURATION;

enum AnimateState
{
    AS_INIT,
    AS_MOVE,
    AS_DONE
};

class CAnimationMotion
{
public:
    CAnimationMotion(int nId) : m_id(nId) { }
    virtual ~CAnimationMotion() { }

    virtual void animate(AnimateState as, int nMotionPercent, CUIObject *pObj) = 0;

    int            m_id;

};
typedef list<CAnimationMotion *>    ListObjAnimation;


enum AnimateType {
    AT_UNKNOWN,
    AT_FADE,
    AT_MOVE,
};

enum AnimateDirection
{
    AD_LEFT,
    AD_TOP,
    AD_RIGHT,
    AD_BOTTOM
};

AnimateType animateTypeFromString(cstr_t szAnimateType);

CAnimationMotion *newAnimate(AnimateType type, bool bHide, CUIObject *pUIObjTarget, AnimateDirection direction);

//
// Definition the animation process: start, running and stop.
//
class CSkinAnimation
{
public:
    CSkinAnimation(CSkinAnimationUIObj *pObjTarget);    // CSkinAnimationUIObj::onAnimate() to process the animation
    CSkinAnimation(CUIObject *pObjTarget, int nAnimateId, int nDurationTime);    // CUIObject::onAnimate() to process the animation
    CSkinAnimation(CUIObject *pObjTarget, CAnimationMotion *pAnimation, int nDurationTime); // Construct a temporarily Animation
    virtual ~CSkinAnimation();

    int getUIObjectID() const;
    CUIObject *getUIObject() const { return m_pObjTarget; }

    bool onAnimate();

    int getID() const { return m_nAnimateId; }
    AnimateState getState() const { return m_animateState; }

    // return 0 ~ 100
    int getMotionPercent() const { return m_nMotionPercent; }
    int getMotionTimeOffset() const { return m_nMotionTimeOffset; }

protected:
    void initStart();

protected:
    AnimateState        m_animateState;
    uint32_t                m_dwTimeBegin;
    int                    m_nMotionPercent;
    int                    m_nMotionTimeOffset;
    float                m_nOneMotionTime;
    int                    m_nAnimateId;

    CUIObject            *m_pObjTarget;
    CAnimationMotion    *m_pMotion;

};
