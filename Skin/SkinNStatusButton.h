// SkinNStatusButton.h: interface for the CSkinNStatusButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINNSTATUSBUTTON_H__DE43DEFF_B449_473B_8C7D_D648AFD8C519__INCLUDED_)
#define AFX_SKINNSTATUSBUTTON_H__DE43DEFF_B449_473B_8C7D_D648AFD8C519__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIObject.h"

class CSkinNStatusButton : public CUIObject  
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinNStatusButton();
    virtual ~CSkinNStatusButton();

    void onTimer(int nId);

    bool onMouseDrag(CPoint point);
    bool onMouseMove(CPoint point);
    bool onLButtonUp(uint32_t nFlags, CPoint point);
    bool onLButtonDown(uint32_t nFlags, CPoint point);

    virtual void onKeyUp(uint32_t nChar, uint32_t nFlags);
    virtual void onKeyDown(uint32_t nChar, uint32_t nFlags);

    virtual void onSetFocus();
    virtual void onKillFocus();

    void draw(CRawGraph *canvas);

    bool setProperty(cstr_t szProperty, cstr_t szValue);
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

    bool isMouseHitBt(CPoint &pt);

protected:

    struct BtStatImg
    {
        BtStatImg() : nIDCmd(0) { }
        string            strBkFile, strSelFile, strHoverFile, strDisabledFile;
        CSFImage        imgBk, imgSel, imgHover, imgDisabled;
        int                nIDCmd;
    };

    typedef vector<BtStatImg *>        V_BTSTATIMG;

    V_BTSTATIMG        m_vBtStatImg;

    int                m_xExtendStart, m_xExtendEnd;
    bool            m_bTile;

    int                m_nCurStatus;

    bool            m_bEnableHover;
    bool            m_bHover;
    bool            m_bLBtDown;

    // Will the cmd be triggered continuous?
    int                m_nTimerIdContinuous;
    bool            m_bContinuousCmd;
    bool            m_bContinuousBegin;

    bool            m_bFadein;
    uint32_t            m_dwBeginFadeinTime;
    int                m_nTimerIdFadein;
    CSFImage        *m_pLastImage;

    BlendPixMode    m_bpm;

};

#endif // !defined(AFX_SKINNSTATUSBUTTON_H__DE43DEFF_B449_473B_8C7D_D648AFD8C519__INCLUDED_)
