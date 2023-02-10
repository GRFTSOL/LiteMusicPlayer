/********************************************************************
    Created  :    2001年12月27日 0:01:36
    FileName :    LyricShowTwoRowObj.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "MPlayerApp.h"
#include "LyricShowTwoRowObj.h"


// set classname
cstr_t CLyricShowTwoRowObj::ms_szClassName = "LyricShowTwoRow";

CLyricShowTwoRowObj::CLyricShowTwoRowObj() {
    m_nCurRowOld = -1;
    m_nRowTheOtherOld = -1;
    m_nDarkenTopArea = float(1.25);
    m_nDarkenBottomArea = float(1.25);
}

CLyricShowTwoRowObj::~CLyricShowTwoRowObj() {
}

void CLyricShowTwoRowObj::fastDraw(CRawGraph *canvas, CRect *prcUpdate) {
    if (m_pMLData == nullptr) {
        return;
    }

    canvas->setFont(&m_font);

    if (prcUpdate) {
        *prcUpdate = m_rcObj;
    }

    // get the current playing row
    int nRowCur = m_pMLData->getCurPlayLine(m_lyrLines);
    if (nRowCur == -1) {
        // no lyrics, redraw background
        updateLyricDrawBufferBackground(canvas, m_rcObj);
        return;
    }

    LyricsLine *pLyricRow = m_lyrLines[nRowCur];

    int nRowHeight = getLineHeight();

    int y = getLineVertAlignPos() - getLineHeight() + m_nLineSpacing / 2;
    if (y < m_rcObj.top + m_nYMargin) {
        y = m_rcObj.top + m_rcObj.height() / 2 - getLineHeight() + m_nLineSpacing / 2;
    }

    // 如果当前歌词行的显示时间超过一半，则预显示下一行歌词
    int nRowTheOther = 0;
    if ( (m_pMLData->getPlayElapsedTime() - pLyricRow->nBegTime) * 2 >= pLyricRow->nEndTime - m_pMLData->getPlayElapsedTime()) {
        nRowTheOther = nRowCur + 1;//bShowNextRow = true;
    } else {
        nRowTheOther = nRowCur - 1;
        if (nRowTheOther < 0) {
            // 当前行为最开始的一行，则显示下一行
            nRowTheOther = nRowCur + 1;
        }
    }

    if (nRowTheOther >= (int)m_lyrLines.size()) {
        // 当前行为最后一行，则显示前一行
        nRowTheOther = nRowCur - 1;
    }

    LyricsLine *pLineTheOther = nRowTheOther >= 0 ? m_lyrLines[nRowTheOther] : nullptr;

    int yCurRow = y;
    int yTheOtherRow = y + nRowHeight;
    if (nRowCur % 2 != 0) {
        std::swap(yCurRow, yTheOtherRow);
    }

    if (prcUpdate && m_nCurRowOld == nRowCur
        && m_nRowTheOtherOld == nRowTheOther
        && m_LyricsDisplayOpt == DO_NORMAL) {
        if (!isKaraoke()) {
            prcUpdate->setEmpty();
            // 如果没有使用KOROKE方式则直接返回
            return;
        }

        // 只重绘当前歌词行
        prcUpdate->top = yCurRow;
        prcUpdate->bottom = yCurRow + nRowHeight;
        if (prcUpdate->top < m_rcObj.top + m_nYMargin) {
            prcUpdate->top = m_rcObj.top + m_nYMargin;
        }
        if (prcUpdate->bottom > m_rcObj.bottom - m_nYMargin) {
            prcUpdate->bottom = m_rcObj.bottom - m_nYMargin;
        }

        // 当前行歌词没有超出歌词显示区域
        prcUpdate->left = m_rcObj.left + m_nXMargin;
        prcUpdate->right = m_rcObj.right - m_nXMargin;

        // clear background
        updateLyricDrawBufferBackground(canvas, *prcUpdate);

        CRect rcClip;

        rcClip = *prcUpdate;

        CRawGraph::CClipBoxAutoRecovery autoCBR(canvas);
        canvas->setClipBoundBox(rcClip);

        // 显示当前歌词行
        drawCurrentRow(canvas, pLyricRow, AUTO_CAL_X, yCurRow);

        return;
    } else {
        m_nCurRowOld = nRowCur;
        m_nRowTheOtherOld = nRowTheOther;
    }

    // clear back buffer
    CRect rcClip;
    if (prcUpdate) {
        prcUpdate->setLTRB(m_rcObj.left + m_nXMargin, y, m_rcObj.right - m_nXMargin, y + nRowHeight * 2);
        if (prcUpdate->top < m_rcObj.top + m_nYMargin) {
            prcUpdate->top = m_rcObj.top + m_nYMargin;
        }
        if (prcUpdate->bottom > m_rcObj.bottom - m_nYMargin) {
            prcUpdate->bottom = m_rcObj.bottom - m_nYMargin;
        }
        updateLyricDrawBufferBackground(canvas, *prcUpdate);
        rcClip = *prcUpdate;
    } else {
        updateLyricDrawBufferBackground(canvas, m_rcObj);


        rcClip.setLTRB(m_rcObj.left + m_nXMargin,
            m_rcObj.top + m_nYMargin,
            m_rcObj.right - m_nXMargin,
            m_rcObj.bottom - m_nYMargin);
    }
    CRawGraph::CClipBoxAutoRecovery autoCBR(canvas);
    canvas->setClipBoundBox(rcClip);

    // 显示当前歌词行
    drawCurrentRow(canvas, pLyricRow, AUTO_CAL_X, yCurRow);

    if (pLineTheOther) {
        // 显示前（后）一歌词行
        drawRow(canvas, pLineTheOther, AUTO_CAL_X, yTheOtherRow,
            nRowTheOther < nRowCur ? LP_ABOVE_CUR_LINE : LP_BELOW_CUR_LINE);
    }
}

cstr_t CLyricShowTwoRowObj::getClassName() {
    return ms_szClassName;
}

bool CLyricShowTwoRowObj::isKindOf(cstr_t szClassName) {
    if (CLyricShowObj::isKindOf(szClassName)) {
        return true;
    }

    return strcasecmp(szClassName, ms_szClassName) == 0;
}
