#pragma once

#include "SkinContainer.h"


//
// This container will toggle sub UIObject with various effect, such as Fade in/Out
//
class CSkinToggleCtrlContainer : public CSkinContainer {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
    CSkinToggleCtrlContainer(void);
    ~CSkinToggleCtrlContainer(void);

    enum TimerID {
        T_ANIMATION                 = 1
    };

    CRawGraph *getMemGraph() override;

    void invalidateUIObject(CUIObject *pObj) override;

    void updateMemGraphicsToScreen(const CRect* lpRect, CUIObject *pObjCallUpdate) override;

    void draw(CRawGraph *canvas) override;

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    virtual void onCreate() override;

    virtual void onTimer(int nId) override;

protected:
    int                         m_nActiveCtrl, m_nToggleToCtrl;
    int                         m_nIDTimerAnimation;
    int64_t                     m_timeBeginAni;
    int                         m_nAnimateDuration;

    CRawGraph                   m_memGraph;

};
