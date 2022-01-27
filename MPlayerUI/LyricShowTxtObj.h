/********************************************************************
    Created  :    2002/01/04    21:40
    FileName :    LyricShowTxtObj.h
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#if !defined(AFX_LYRICSHOWTXTOBJ_H__18145744_7DB0_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_LYRICSHOWTXTOBJ_H__18145744_7DB0_11D5_9E04_02608CAD9330__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricShowMultiRowObj.h"
#include "LyricShowTxtContainer.h"

#define SZ_TXT_LYR_CONTAINER CLyricShowTxtContainer::className()

class CLyricShowTxtObj : public CLyricShowMultiRowObj, IScrollNotify
{
    UIOBJECT_CLASS_NAME_DECLARE(CLyricShowMultiRowObj)
public:
    CLyricShowTxtObj();
    virtual ~CLyricShowTxtObj();

public:
    void onKeyDown(uint32_t nChar, uint32_t nFlags);
    void onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt);
    void onLyricsChanged();

    void onCreate();
    void onEvent(const IEvent *pEvent);
    void onSize();

    void onTimer(int nId);

    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar);
    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) { }

    void onPlayTimeChangedUpdate();

    bool isRecordScrollingActionsEnabled()
        const { return m_bRecordScrollingActionsEnabled; };
    void enableRecordScrollingActions(bool bEnable);

    bool isReplayScrollingActionsEnabled()
        const { return m_bReplayScrollingActionsEnabled && !m_bReplayDisabledTemp; };
    void enableReplayScrollingActions(bool bEnable);

protected:
    void onLyrDrawContextChanged();

    void fastDraw(CRawGraph *canvas, CRect *prcUpdate = nullptr);

    void setScrollInfo();

    bool setProperty(cstr_t szProperty, cstr_t szValue);

    int getLinesPerPage();

    void startScrollAnimation(int nCurLineNew);

protected:
    IScrollBar                *m_pScrollBar;
    CUIObject                *m_pObjScrollBar;

    int                        m_nCurLine;

    bool                    m_bInScrollingToNewLine;
    int                        m_nCurLineNew;
    int                        m_yCurLineLatest, m_nCurLineLatest;
    int                        m_yOffsetScroll;
    uint32_t                    m_nBeginScrollTime;

    bool                    m_bInVerticalScrollingMode;

    bool                    m_bRecordScrollingActionsEnabled;
    bool                    m_bReplayScrollingActionsEnabled;

    bool                    m_bReplayDisabledTemp;

};

#endif // !defined(AFX_LYRICSHOWTXTOBJ_H__18145744_7DB0_11D5_9E04_02608CAD9330__INCLUDED_)
