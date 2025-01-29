

#include "MPlayerApp.h"
#include "LyricShowVobSub.h"


// set classname
cstr_t CLyricShowVobSub::ms_szClassName = "LyricShowVobSub";

CLyricShowVobSub::CLyricShowVobSub() {
    m_nDarkenTopArea = float(0.65);
    m_nDarkenBottomArea = float(0.65);

    m_nCurRowOld = -1;
}

CLyricShowVobSub::~CLyricShowVobSub() {

}

void CLyricShowVobSub::fastDraw(CRawGraph *canvas, CRect *prcUpdate) {
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

    int nPlayPos = m_curLyrics->getPlayElapsedTime();

    auto &lyricRow = m_lyrLines[nRowCur];
    int nRowNext = nRowCur +1;

    LyricsLine *lyricRowNext = nullptr;
    if (nRowNext < (int)m_lyrLines.size()) {
        lyricRowNext = &m_lyrLines[nRowNext];
        if (nPlayPos < lyricRowNext->beginTime) {
            lyricRowNext = nullptr;
        }
    }

    int y = 0;
    if (lyricRowNext) {
        y = getLineVertAlignPos() - getLineHeight() - m_nLineSpacing / 2;
    } else {
        y = getLineVertAlignPos() - getLineHeight() / 2;
    }

    //
    // 增加此判断提高效率
    //
    if (nPlayPos >= lyricRow.beginTime &&
        nPlayPos <= lyricRow.endTime &&
        nRowCur == m_nCurRowOld && prcUpdate != nullptr &&
        (m_LyricsDisplayOpt == DO_NORMAL && !isKaraoke())) {
        return;
    }

    if (nPlayPos >= lyricRow.beginTime &&
        nPlayPos <= lyricRow.endTime) {
        // 显示此行
        drawCurrentRow(canvas, lyricRow, AUTO_CAL_X, y);
        y += getLineHeight();
    }

    if (lyricRowNext && nPlayPos >= lyricRowNext->beginTime &&
        nPlayPos <= lyricRowNext->endTime) {
        // 显示此行
        drawCurrentRow(canvas, *lyricRowNext, AUTO_CAL_X, y);
    }

    if (nPlayPos >= lyricRow.beginTime && nPlayPos <= lyricRow.endTime) {
        m_nCurRowOld = nRowCur;
    }
}

cstr_t CLyricShowVobSub::getClassName() {
    return ms_szClassName;
}

bool CLyricShowVobSub::isKindOf(cstr_t szClassName) {
    if (CLyricShowObj::isKindOf(szClassName)) {
        return true;
    }

    return strcasecmp(szClassName, ms_szClassName) == 0;
}
