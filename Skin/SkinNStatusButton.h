#pragma once

#ifndef Skin_SkinNStatusButton_h
#define Skin_SkinNStatusButton_h


#include "UIObject.h"


class CSkinNStatusButton : public CUIObject {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinNStatusButton();
    virtual ~CSkinNStatusButton();

    void onTimer(int nId) override;

    bool onMouseDrag(CPoint point) override;
    bool onMouseMove(CPoint point) override;
    bool onLButtonUp(uint32_t nFlags, CPoint point) override;
    bool onLButtonDown(uint32_t nFlags, CPoint point) override;

    virtual void onKeyUp(uint32_t nChar, uint32_t nFlags) override;
    virtual void onKeyDown(uint32_t nChar, uint32_t nFlags) override;

    virtual void onSetFocus() override;
    virtual void onKillFocus() override;

    void draw(CRawGraph *canvas) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    int getStatus();
    void setStatus(int nStatus);

protected:
    virtual void buttonDownAction();
    virtual void buttonUpAction();

    // This should be called before status changed.
    void startFadeDrawTimer(bool bPrevButtonDown, bool bPrevHover);

    void fadeInDrawBt(CRawGraph *canvas, CSFImage *pImage, int nAlpha);

    void setMaxStat(int nMaxStat);
    void destroy();

protected:

    struct BtStatImg {
        BtStatImg() : nIDCmd(0) { }
        string                      strBkFile, strSelFile, strHoverFile, strDisabledFile;
        CSFImage                    imgBk, imgSel, imgHover, imgDisabled;
        int                         nIDCmd;
    };

    typedef vector<BtStatImg *>        V_BTSTATIMG;

    V_BTSTATIMG                 m_vBtStatImg;

    int                         m_xExtendStart, m_xExtendEnd;
    bool                        m_bTile;

    int                         m_nCurStatus;

    bool                        m_bEnableHover;
    bool                        m_bHover;
    bool                        m_bLBtDown;

    // Will the cmd be triggered continuous?
    int                         m_nTimerIdContinuous;
    bool                        m_bContinuousCmd;
    bool                        m_bContinuousBegin;

    bool                        m_bFadein;
    int64_t                     m_timeBeginFadein;
    int                         m_nTimerIdFadein;
    CSFImage                    *m_pLastImage;

    BlendPixMode                m_bpm;

};

#endif // !defined(Skin_SkinNStatusButton_h)
