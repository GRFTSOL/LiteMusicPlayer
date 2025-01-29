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
    if (m_curLyrics == nullptr) {
        return;
    }

    // get the current playing row
    int nRowCur = m_curLyrics->getCurPlayLine(m_lyrLines);
    if (nRowCur == -1) {
        // no lyrics
        if (prcUpdate) {
            prcUpdate->setEmpty();
        }
        return;
    }

    LyricsLine &lyricRow = m_lyrLines[nRowCur];

    int nRowHeight = getLineHeight();

    int y = getLineVertAlignPos() - getLineHeight() + m_nLineSpacing / 2;
    if (y < m_rcObj.top + m_nYMargin) {
        y = m_rcObj.top + m_rcObj.height() / 2 - getLineHeight() + m_nLineSpacing / 2;
    }

    // 如果当前歌词行的显示时间超过一半，则预显示下一行歌词
    int nRowTheOther = 0;
    if ( (m_curLyrics->getPlayElapsedTime() - lyricRow.beginTime) * 2 >= lyricRow.endTime - m_curLyrics->getPlayElapsedTime()) {
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

    LyricsLine *lineTheOther = nRowTheOther >= 0 ? &m_lyrLines[nRowTheOther] : nullptr;

    int yCurRow = y;
    int yTheOtherRow = y + nRowHeight;
    if (nRowCur % 2 != 0) {
        std::swap(yCurRow, yTheOtherRow);
    }

    if (prcUpdate) {
        prcUpdate->top = yCurRow;

        if (m_nCurRowOld == nRowCur
            && m_nRowTheOtherOld == nRowTheOther
            && m_LyricsDisplayOpt == DO_NORMAL)
        {
            if (!isKaraoke()) {
                prcUpdate->setEmpty();
                // 如果没有使用KOROKE方式则直接返回
                return;
            }

            // 只重绘当前歌词行
            prcUpdate->bottom = yCurRow + nRowHeight;
        } else {
            // 绘制两行
            prcUpdate->bottom = nRowHeight * 2;
        }

        prcUpdate->intersect(getClipRect());

        // 不进行真正的绘制
        return;
    } else {
        m_nCurRowOld = nRowCur;
        m_nRowTheOtherOld = nRowTheOther;
    }

    // 显示当前歌词行
    drawCurrentRow(canvas, lyricRow, AUTO_CAL_X, yCurRow);

    if (lineTheOther) {
        // 显示前（后）一歌词行
        drawRow(canvas, *lineTheOther, AUTO_CAL_X, yTheOtherRow,
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
