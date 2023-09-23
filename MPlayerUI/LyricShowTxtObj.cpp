/********************************************************************
    Created  :    2002年12月2日 10:48:18
    FileName :    LyricShowTxtObj.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "MPlayerApp.h"
#include "LyricShowTxtObj.h"


#define SCROLL_ANIMATION_DURATION   300
#define SCROLL_ANIMATION_TIMEOUT    50

UIOBJECT_CLASS_NAME_IMP(CLyricShowTxtObj, "LyricShowTxt")

CLyricShowTxtObj::CLyricShowTxtObj() {
    m_nCurLine = 0;

    m_msgNeed |= UO_MSG_WANT_MOUSEWHEEL;

    m_nDarkenTopArea = float(0.5);
    m_nDarkenBottomArea = float(0.5);

    m_pScrollBar = nullptr;
    m_pObjScrollBar = nullptr;

    m_bInScrollingToNewLine = false;
    m_nCurLineNew = 0;
    m_yCurLineLatest = 0;
    m_nCurLineLatest = 0;
    m_timeBeginScroll = 0;
    m_bInVerticalScrollingMode = false;
}

CLyricShowTxtObj::~CLyricShowTxtObj() {
}

bool CLyricShowTxtObj::onKeyDown(uint32_t nChar, uint32_t nFlags) {
    if (g_currentLyrics.getLyrContentType() != LCT_TXT) {
        return CLyricShowMultiRowObj::onKeyDown(nChar, nFlags);
    }

    bool bProcessed = false;
    int nCurLineNew = m_nCurLineNew;

    int nMax = (int)m_lyrLines.size() - 1;
    if (m_pScrollBar) {
        nMax = m_pScrollBar->getMax();
    }

    switch (nChar) {
    case VK_UP:
        // 跳到前一行歌词
        nCurLineNew--;
        if (nCurLineNew < 0) {
            nCurLineNew = 0;
        }
        bProcessed = true;
        break;
    case VK_DOWN:
        nCurLineNew++;
        if (nCurLineNew > nMax) {
            nCurLineNew--;
        }
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
        if (nCurLineNew > nMax) {
            nCurLineNew = nMax;
        }
        bProcessed = true;
        break;
    }

    if (bProcessed) {
        if (nCurLineNew < 0) {
            nCurLineNew = 0;
        }

        if (nCurLineNew != m_nCurLine) {
            if (m_pScrollBar) {
                m_pScrollBar->setScrollPos(nCurLineNew);
            }
        }
        return true;
    } else {
        return CLyricShowMultiRowObj::onKeyDown(nChar, nFlags);
    }
}

void CLyricShowTxtObj::fastDraw(CRawGraph *canvas, CRect *prcUpdate) {
    if (!m_curLyrics->hasLyricsOpened() || m_lyrLines.size() == 0) {
        fastDrawMediaInfo(canvas, prcUpdate);
        return;
    }

    int nCurLine = m_curLyrics->getCurPlayLine(m_lyrLines);
    if (nCurLine < 0 || nCurLine >= (int)m_lyrLines.size()) {
        return;
    }

    if (m_bInVerticalScrollingMode) {
        // Switched from vertical scrolling mode.
        m_nCurLine = m_nCurLineNew = nCurLine;
        m_bInVerticalScrollingMode = false;
    } else {
        if (prcUpdate && !m_bInScrollingToNewLine) {
            // Fast draw, don't update anything
            prcUpdate->setEmpty();
            return;
        }
    }

    if (prcUpdate) {
        *prcUpdate = m_rcObj;
    }

    if (m_lyrLines.size() == 0) {
        updateLyricDrawBufferBackground(canvas, m_rcObj);
        return;
    }

    int nLineHeight = getLineHeight();
    int yCurLine = getLineVertAlignPos();

    if (m_bInScrollingToNewLine) {
        auto now = getTickCount();
        if (now < m_timeBeginScroll
            || (now - m_timeBeginScroll) >= SCROLL_ANIMATION_DURATION) {
            m_bInScrollingToNewLine = false;
            m_nCurLine = m_nCurLineNew;
            m_pSkin->unregisterTimerObject(this);
        } else {
            int    distance = (m_nCurLine - m_nCurLineNew) * nLineHeight
            - m_yOffsetScroll;
            yCurLine = yCurLine + m_yOffsetScroll + (distance * (int)(now - m_timeBeginScroll) / SCROLL_ANIMATION_DURATION);
        }

        if (prcUpdate
            && yCurLine == m_yCurLineLatest
            && m_nCurLine == m_nCurLineLatest) {
            prcUpdate->setEmpty();
            return;
        }
        m_nCurLineLatest = m_nCurLine;
        m_yCurLineLatest = yCurLine;
    }

    if (m_nCurLine >= (int)m_lyrLines.size() || m_nCurLine < 0) {
        m_nCurLine = 0;
    }

    // clear background
    updateLyricDrawBufferBackground(canvas, m_rcObj);

    CRect rcClip;

    rcClip.setLTRB(m_rcObj.left + m_nXMargin,
        m_rcObj.top + m_nYMargin,
        m_rcObj.right - m_nXMargin,
        m_rcObj.bottom - m_nYMargin);

    CRawGraph::CClipBoxAutoRecovery autoCBR(canvas);
    canvas->setClipBoundBox(rcClip);

    CDispOptRecover dispOptRecover(this);
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
        if (y + nLineHeight > rcClip.top) {
            drawRow(canvas, m_lyrLines[i], AUTO_CAL_X, y, LP_BELOW_CUR_LINE);
        }
    }

    fadeOutVertBorder(canvas, rcClip.top, rcClip.bottom);
}

void CLyricShowTxtObj::onLyricsChanged() {
    CLyricShowMultiRowObj::onLyricsChanged();

    m_nCurLine = 0;
    m_nCurLineNew = 0;

    setScrollInfo();

    m_pSkin->invalidateRect();
}

void CLyricShowTxtObj::onLyrDrawContextChanged() {
    CLyricShowMultiRowObj::onLyrDrawContextChanged();

    setScrollInfo();
}

void CLyricShowTxtObj::setScrollInfo() {
    if (m_pScrollBar) {
        if (m_nCurLine < 0 || m_nCurLine >= (int)m_lyrLines.size()) {
            m_nCurLine = 0;
        }

        m_pScrollBar->setScrollInfo(0, (int)m_lyrLines.size() + getLinesPerPage() - 1, getLinesPerPage(), m_nCurLine);
    }
}

void CLyricShowTxtObj::onPlayTimeChangedUpdate() {
    // No need to update lyrics with time
    if (!m_bInScrollingToNewLine && !m_bInVerticalScrollingMode) {
        return;
    }

    CLyricShowMultiRowObj::onPlayTimeChangedUpdate();
}

void CLyricShowTxtObj::onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) {
    int nCurLineNew = pScrollBar->getScrollPos();
    if (nCurLineNew != m_nCurLine) {
        startScrollAnimation(nCurLineNew);
    }
}

void CLyricShowTxtObj::onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) {
    if (m_pObjScrollBar) {
        m_pObjScrollBar->onMouseWheel(nWheelDistance, nMkeys, pt);
    } else {
        CLyricShowMultiRowObj::onMouseWheel(nWheelDistance, nMkeys, pt);
    }
}

void CLyricShowTxtObj::onCreate() {
    CLyricShowMultiRowObj::onCreate();

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_LYRICS_ON_SAVE_EDIT);

    m_pObjScrollBar = m_pContainer->getUIObjectByClassName(CSkinVScrollBar::className());
    if (m_pObjScrollBar) {
        m_pScrollBar = (CSkinVScrollBar*)m_pObjScrollBar;
    }

    if (m_pScrollBar) {
        m_pScrollBar->setScrollNotify(this);
    }

    setScrollInfo();
}

void CLyricShowTxtObj::onSize() {
    CLyricShowMultiRowObj::onSize();

    setScrollInfo();
}

void CLyricShowTxtObj::onTimer(int nId) {
    onPlayTimeChangedUpdate();
}

bool CLyricShowTxtObj::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (isPropertyName(szProperty, "MarginY")) {
        m_nYMargin = atoi(szValue);
    } else {
        return CLyricShowMultiRowObj::setProperty(szProperty, szValue);
    }

    return true;
}

int CLyricShowTxtObj::getLinesPerPage() {
    return (m_rcObj.height() - m_nYMargin * 2) / getLineHeight();
}

void CLyricShowTxtObj::startScrollAnimation(int nCurLineNew) {
    bool bMultipleLineScrolled = (abs(nCurLineNew - m_nCurLine) > 1);

    if (m_bInScrollingToNewLine) {
        m_yOffsetScroll = m_yCurLineLatest - getLineVertAlignPos();
    } else {
        m_yOffsetScroll = 0;
    }

    m_nCurLineNew = nCurLineNew;
    m_bInScrollingToNewLine = true;
    m_timeBeginScroll = getTickCount();

    m_pSkin->registerTimerObject(this, SCROLL_ANIMATION_TIMEOUT);
}
