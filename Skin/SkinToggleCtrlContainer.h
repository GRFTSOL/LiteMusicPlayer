#pragma once

#include "SkinContainer.h"

//
// This container will toggle sub UIObject with various effect, such as Fade in/Out
//
class CSkinToggleCtrlContainer : public CSkinContainer
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
    CSkinToggleCtrlContainer(void);
    ~CSkinToggleCtrlContainer(void);

    enum TimerID
    {
        T_ANIMATION    = 1
    };

    void invalidateUIObject(CUIObject *pObj);

    void updateMemGraphicsToScreen(const CRect* lpRect, CUIObject *pObjCallUpdate);

    void draw(CRawGraph *canvas);

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue);

    virtual void onCreate();

    virtual void onTimer(int nId);

protected:
    int                     m_nActiveCtrl, m_nToggleToCtrl;
    int                     m_nIDTimerAnimation;
    uint32_t                m_nTimeBeginAni;
    int                     m_nAnimateDuration;

    CRawGraph               m_memGraph;

};
