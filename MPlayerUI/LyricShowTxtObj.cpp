/********************************************************************
    Created  :    2002年12月2日 10:48:18
    FileName :    LyricShowTxtObj.cpp
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#include "MPlayerApp.h"
#include "LyricShowTxtObj.h"

#define SCROLL_ANIMATION_DURATION    300
#define SCROLL_ANIMATION_TIMEOUT    50

UIOBJECT_CLASS_NAME_IMP(CLyricShowTxtObj, "LyricShowTxt")

CLyricShowTxtObj::CLyricShowTxtObj()
{
    m_nCurLine = 0;

    m_msgNeed |= UO_MSG_WANT_MOUSEWHEEL;

    m_nDarkenTopArea = float(0.5);
    m_nDarkenBottomArea = float(0.5);

    m_pScrollBar = nullptr;
    m_pObjScrollBar = nullptr;

    m_bInScrollingToNewLine = false;
    m_nCurLineNew = 0;
    m_yCurLineLatest = 0, m_nCurLineLatest = 0;
    m_nBeginScrollTime = 0;
    m_bInVerticalScrollingMode = false;

    m_bRecordScrollingActionsEnabled = true;
    m_bReplayScrollingActionsEnabled = true;
    m_bReplayDisabledTemp = false;
}

CLyricShowTxtObj::~CLyricShowTxtObj()
{
}

void CLyricShowTxtObj::onKeyDown(uint32_t nChar, uint32_t nFlags)
{
    if (g_LyricData.getLyrContentType() != LCT_TXT && !m_bReplayScrollingActionsEnabled)
    {
        CLyricShowMultiRowObj::onKeyDown(nChar, nFlags);
        return;
    }

    bool    bProcessed = false;
    int        nCurLineNew = m_nCurLineNew;

    int        nMax = m_lyrLines.size() - 1;
    if (m_pScrollBar)
        nMax = m_pScrollBar->getMax();

    switch (nChar)
    {
    case VK_UP:
        // 跳到前一行歌词
        nCurLineNew--;
        if (nCurLineNew < 0)
            nCurLineNew = 0;
        bProcessed = true;
        break;
    case VK_DOWN:
        nCurLineNew++;
        if (nCurLineNew > nMax)
            nCurLineNew--;
        bProcessed = true;
        break;
    case VK_HOME:
        nCurLineNew = 0;
        bProcessed = true;
        break;
    case VK_END:
        nCurLineNew = nMax;
        bProcessed = true;
        break;
    case VK_PRIOR:
        nCurLineNew -= getLinesPerPage() - 1;
        bProcessed = true;
        break;
    case VK_NEXT:
        nCurLineNew += getLinesPerPage() - 1;
        if (nCurLineNew > nMax)
            nCurLineNew = nMax;
        bProcessed = true;
        break;
    }

    if (bProcessed)
    {
        if (!m_bRecordScrollingActionsEnabled)
            m_bReplayDisabledTemp = true;

        if (nCurLineNew < 0)
            nCurLineNew = 0;

        if (nCurLineNew != m_nCurLine)
        {
            if (m_pScrollBar)
                m_pScrollBar->setScrollPos(nCurLineNew);
            startScrollAnimation(nCurLineNew);
        }
    }
    else
    {
        CLyricShowMultiRowObj::onKeyDown(nChar, nFlags);
    }
}

void CLyricShowTxtObj::fastDraw(CRawGraph *canvas, CRect *prcUpdate/* = nullptr*/)
{
    if (!m_pMLData->hasLyricsOpened() || m_lyrLines.size() == 0)
    {
        fastDrawMediaInfo(canvas, prcUpdate);
        return;
    }

    int nCurLine = m_pMLData->getCurPlayLine(m_lyrLines);
    if (nCurLine < 0 || nCurLine >= (int)m_lyrLines.size())
        return;

    if (m_bReplayScrollingActionsEnabled && !m_bReplayDisabledTemp
        && m_lyrLines[nCurLine]->nEndTime != CLyrScrollActionRecorder::MAX_TIME)
    {
        // in Scrolling mode
        m_bInVerticalScrollingMode = true;
        if (m_nCurLine != nCurLine)
        {
            m_nCurLineNew = m_nCurLine = nCurLine;
            if (m_pScrollBar)
                m_pScrollBar->setScrollPos(m_nCurLine);
        }

        CDispOptRecover        dispOptRecover(this);
        m_LyricsDisplayOpt = DO_FADEOUT_LOWCOLOR;

        CLyricShowMultiRowObj::fastDraw(canvas, prcUpdate);
        return;
    }

    if (m_bInVerticalScrollingMode)
    {
        // Switched from vertical scrolling mode.
        m_nCurLine = m_nCurLineNew = nCurLine;
        m_bInVerticalScrollingMode = false;
    }
    else
    {
        if (prcUpdate && !m_bInScrollingToNewLine)
        {
            // Fast draw, don't update anything
            prcUpdate->setEmpty();
            return;
        }
    }

    if (prcUpdate)
        *prcUpdate = m_rcObj;

    if (m_lyrLines.size() == 0)
    {
        updateLyricDrawBufferBackground(canvas, m_rcObj);
        return;
    }

    int        nLineHeight = getLineHeight();
    int        yCurLine = getLineVertAlignPos();

    if (m_bInScrollingToNewLine)
    {
        uint32_t nTimeCur = getTickCount();
        if (nTimeCur < m_nBeginScrollTime
            || (nTimeCur - m_nBeginScrollTime) >= SCROLL_ANIMATION_DURATION)
        {
            m_bInScrollingToNewLine = false;
            m_nCurLine = m_nCurLineNew;
            m_pSkin->unregisterTimerObject(this);
        }
        else
        {
            int    distance = (m_nCurLine - m_nCurLineNew) * nLineHeight
                - m_yOffsetScroll;
            yCurLine = yCurLine + m_yOffsetScroll + (distance * (int)(nTimeCur - m_nBeginScrollTime) / SCROLL_ANIMATION_DURATION);
        }

        if (prcUpdate
            && yCurLine == m_yCurLineLatest
            && m_nCurLine == m_nCurLineLatest)
        {
            prcUpdate->setEmpty();
            return;
        }
        m_nCurLineLatest = m_nCurLine;
        m_yCurLineLatest = yCurLine;
    }

    if (m_nCurLine >= (int)m_lyrLines.size() || m_nCurLine < 0)
        m_nCurLine = 0;

    // clear background
    updateLyricDrawBufferBackground(canvas, m_rcObj);

    CRect    rcClip;

    rcClip.setLTRB(m_rcObj.left + m_nXMargin, 
                        m_rcObj.top + m_nYMargin,
                        m_rcObj.right - m_nXMargin,
                        m_rcObj.bottom - m_nYMargin);

    CRawGraph::CClipBoxAutoRecovery    autoCBR(canvas);
    canvas->setClipBoundBox(rcClip);

    CDispOptRecover        dispOptRecover(this);
    m_LyricsDisplayOpt = DO_FADEOUT_LOWCOLOR;

    canvas->setFont(&m_font);

    // draw lines before m_nCurLine
    for (int i = m_nCurLine - 1, y = yCurLine;
        i >= 0 && y > rcClip.top; i--)
    {
        y -= nLineHeight;
        drawRow(canvas, m_lyrLines[i], AUTO_CAL_X, y, LP_ABOVE_CUR_LINE);
    }

    // draw current line.
    drawRow(canvas, m_lyrLines[m_nCurLine], AUTO_CAL_X, yCurLine, LP_CUR_LINE);

    // draw lines from m_nCurLine + 1
    for (int i = m_nCurLine + 1, y = yCurLine + nLineHeight;
        i < (int)m_lyrLines.size() && y < rcClip.bottom; i++, y += nLineHeight)
    {
        if (y + nLineHeight > rcClip.top)
            drawRow(canvas, m_lyrLines[i], AUTO_CAL_X, y, LP_BELOW_CUR_LINE);
    }

    fadeOutVertBorder(canvas, rcClip.top, rcClip.bottom);
}

void CLyricShowTxtObj::onLyricsChanged()
{
    CLyricShowMultiRowObj::onLyricsChanged();

    m_bReplayDisabledTemp = false;

    CSkinToolbar *pToolbarLyrTxt = (CSkinToolbar*)m_pSkin->getUIObjectById(ID_TB_LYR_TXT, CSkinToolbar::className());
    if (pToolbarLyrTxt)
    {
        pToolbarLyrTxt->setCheck(CMD_LYR_SCROLL_ENABLE_RECORD,
            isRecordScrollingActionsEnabled() && g_LyricData.getLyrContentType() == LCT_TXT);
        pToolbarLyrTxt->setCheck(CMD_LYR_SCROLL_ENABLE_REPLAY, isReplayScrollingActionsEnabled());
    }

    m_nCurLine = 0;
    m_nCurLineNew = 0;

    setScrollInfo();

    m_pSkin->invalidateRect();
}

void CLyricShowTxtObj::onLyrDrawContextChanged()
{
    CLyricShowMultiRowObj::onLyrDrawContextChanged();

    setScrollInfo();

    if (g_LyricData.isUsingLyrScrollActionRecorder())
        g_LyricData.updateTimeTagByLyrScrollActions(m_lyrLines);
}

void CLyricShowTxtObj::setScrollInfo()
{
    if (m_pScrollBar)
    {
        if (m_nCurLine < 0 || m_nCurLine >= (int)m_lyrLines.size())
            m_nCurLine = 0;

        m_pScrollBar->setScrollInfo(0, m_lyrLines.size() + getLinesPerPage() - 1, getLinesPerPage(), m_nCurLine);
    }
}

void CLyricShowTxtObj::onPlayTimeChangedUpdate()
{
    // No need to update lyrics with time
    if (!m_bInScrollingToNewLine && !m_bInVerticalScrollingMode && (!m_bReplayScrollingActionsEnabled || m_bReplayDisabledTemp))
        return;

    CLyricShowMultiRowObj::onPlayTimeChangedUpdate();
}

void CLyricShowTxtObj::enableRecordScrollingActions(bool bEnable)
{
    if (g_LyricData.getLyrContentType() == LCT_TXT)
    {
        m_bReplayDisabledTemp = false;
        m_bRecordScrollingActionsEnabled = bEnable;
        g_profile.writeInt("LyrRecordSAEnabled", bEnable);
    }
}

void CLyricShowTxtObj::enableReplayScrollingActions(bool bEnable)
{
    if (bEnable)
        m_bReplayDisabledTemp = false;
    m_bReplayScrollingActionsEnabled = bEnable;
    g_profile.writeInt("LyrReplaySAEnabled", bEnable);

    CSkinToolbar *pToolbarLyrTxt = (CSkinToolbar*)m_pSkin->getUIObjectById(ID_TB_LYR_TXT, CSkinToolbar::className());
    if (pToolbarLyrTxt)
        pToolbarLyrTxt->setCheck(CMD_LYR_SCROLL_ENABLE_REPLAY, isReplayScrollingActionsEnabled());
}

void CLyricShowTxtObj::onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar)
{
    int nCurLineNew = pScrollBar->getScrollPos();
    if (nCurLineNew != m_nCurLine)
        startScrollAnimation(nCurLineNew);
}

void CLyricShowTxtObj::onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt)
{
    if (nMkeys == MK_CONTROL)
    {
        CLyricShowMultiRowObj::onMouseWheel(nWheelDistance, nMkeys, pt);
        return;
    }

    if (m_pObjScrollBar)
        m_pObjScrollBar->onMouseWheel(nWheelDistance, nMkeys, pt);
    else
        CLyricShowMultiRowObj::onMouseWheel(nWheelDistance, nMkeys, pt);
}

void CLyricShowTxtObj::onCreate()
{
    CLyricShowMultiRowObj::onCreate();

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_LYRICS_ON_SAVE_EDIT);

    m_bRecordScrollingActionsEnabled = g_profile.getBool("LyrRecordSAEnabled", false);
    m_bReplayScrollingActionsEnabled = g_profile.getBool("LyrReplaySAEnabled", true);

    m_pObjScrollBar = m_pContainer->getUIObjectByClassName(CSkinVScrollBar::className());
    if (m_pObjScrollBar)
        m_pScrollBar = (CSkinVScrollBar*)m_pObjScrollBar;

    if (m_pScrollBar)
        m_pScrollBar->setScrollNotify(this);

    setScrollInfo();
}

void CLyricShowTxtObj::onEvent(const IEvent *pEvent)
{
    if (pEvent->eventType == ET_LYRICS_ON_SAVE_EDIT)
    {
        if (g_LyricData.getLyrContentType() == LCT_TXT)
            g_LyricData.lyrScrollActionsToTag();
    }
    else
        CLyricShowMultiRowObj::onEvent(pEvent);
}

void CLyricShowTxtObj::onSize()
{
    CLyricShowMultiRowObj::onSize();

    setScrollInfo();

    if (g_LyricData.isUsingLyrScrollActionRecorder())
        g_LyricData.updateTimeTagByLyrScrollActions(m_lyrLines);
}

void CLyricShowTxtObj::onTimer(int nId)
{
    onPlayTimeChangedUpdate();
}

bool CLyricShowTxtObj::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (isPropertyName(szProperty, "MarginY"))
    {
        m_nYMargin = atoi(szValue);
    }
    else
        return CLyricShowMultiRowObj::setProperty(szProperty, szValue);

    return true;
}

int CLyricShowTxtObj::getLinesPerPage()
{
    return (m_rcObj.height() - m_nYMargin * 2) / getLineHeight();
}

void CLyricShowTxtObj::startScrollAnimation(int nCurLineNew)
{
    bool bMultipleLineScrolled = (abs(nCurLineNew - m_nCurLine) > 1);

    if (!m_bRecordScrollingActionsEnabled || bMultipleLineScrolled)
        m_bReplayDisabledTemp = true;

    if (g_LyricData.getLyrContentType() == LCT_TXT && m_bRecordScrollingActionsEnabled
        && !bMultipleLineScrolled)
        g_LyricData.lyrScrollToLine(m_lyrLines, nCurLineNew, g_LyricData.getPlayElapsedTime(), true);

    if (m_bInScrollingToNewLine)
        m_yOffsetScroll = m_yCurLineLatest - getLineVertAlignPos();
    else
        m_yOffsetScroll = 0;

    m_nCurLineNew = nCurLineNew;
    m_bInScrollingToNewLine = true;
    m_nBeginScrollTime = getTickCount();

    m_pSkin->registerTimerObject(this, SCROLL_ANIMATION_TIMEOUT);
}
