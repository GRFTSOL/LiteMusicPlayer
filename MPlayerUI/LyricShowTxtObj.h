#pragma once

/********************************************************************
    Created  :    2002/01/04    21:40
    FileName :    LyricShowTxtObj.h
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#ifndef MPlayerUI_LyricShowTxtObj_h
#define MPlayerUI_LyricShowTxtObj_h


#include "LyricShowMultiRowObj.h"
#include "LyricShowTxtContainer.h"


#define SZ_TXT_LYR_CONTAINER CLyricShowTxtContainer::className()

class CLyricShowTxtObj : public CLyricShowMultiRowObj, IScrollNotify {
    UIOBJECT_CLASS_NAME_DECLARE(CLyricShowMultiRowObj)
public:
    CLyricShowTxtObj();
    virtual ~CLyricShowTxtObj();

public:
    bool onKeyDown(uint32_t nChar, uint32_t nFlags) override;
    void onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) override;
    void onLyricsChanged() override;

    void onCreate() override;
    void onEvent(const IEvent *pEvent) override;
    void onSize() override;

    void onTimer(int nId) override;

    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override;
    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override { }

    void onPlayTimeChangedUpdate() override;

    bool isRecordScrollingActionsEnabled()
    const { return m_bRecordScrollingActionsEnabled; };
    void enableRecordScrollingActions(bool bEnable);

    bool isReplayScrollingActionsEnabled()
    const { return m_bReplayScrollingActionsEnabled && !m_bReplayDisabledTemp;  };
    void enableReplayScrollingActions(bool bEnable);

protected:
    void onLyrDrawContextChanged() override;

    void fastDraw(CRawGraph *canvas, CRect *prcUpdate = nullptr) override;

    void setScrollInfo();

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    int getLinesPerPage();

    void startScrollAnimation(int nCurLineNew);

protected:
    IScrollBar                  *m_pScrollBar;
    CUIObject                   *m_pObjScrollBar;

    int                         m_nCurLine;

    bool                        m_bInScrollingToNewLine;
    int                         m_nCurLineNew;
    int                         m_yCurLineLatest, m_nCurLineLatest;
    int                         m_yOffsetScroll;
    int64_t                     m_timeBeginScroll;

    bool                        m_bInVerticalScrollingMode;

    bool                        m_bRecordScrollingActionsEnabled;
    bool                        m_bReplayScrollingActionsEnabled;

    bool                        m_bReplayDisabledTemp;

};

#endif // !defined(MPlayerUI_LyricShowTxtObj_h)
