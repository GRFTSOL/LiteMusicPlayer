#include "LyricsData.h"


LyricsLine::LyricsLine() {
    bAdvertise = false;
    bTempLine = false;
    bLyricsLine = true;
    szContent = nullptr;
}

LyricsLine::~LyricsLine() {
    if (szContent) {
        delete szContent;
    }

    for (int k = 0; k < (int)vFrags.size(); k++) {
        delete[] (uint8_t*)vFrags[k];
    }
}

LyricsPiece *LyricsLine::appendPiece(int nBegTime, int nEndTime,
    cstr_t szLyrics, size_t nLen, bool bTempBegTime, bool bTempEndTime) {
    LyricsPiece *pPiece;

    pPiece = (LyricsPiece *)new uint8_t[sizeof(LyricsPiece) + nLen * sizeof(char)];

    pPiece->nBegTime = nBegTime;
    pPiece->nEndTime = nEndTime;
    pPiece->nDrawWidth = 0;
    strcpy_safe(pPiece->szLyric, nLen + 1, szLyrics);
    pPiece->nLen = nLen;
    pPiece->bTempBegTime = bTempBegTime;
    pPiece->bTempEndTime = bTempEndTime;

    vFrags.push_back(pPiece);

    return pPiece;
}

void LyricsLine::setContent(cstr_t szContent, size_t nLen) {
    if (this->szContent != nullptr) {
        delete[] this->szContent;
    }

    this->szContent = new char[nLen + 1];
    strncpy_safe(this->szContent, nLen + 1, szContent, nLen);
}

LyricsLine *newLyricsLine(int nBegTime, int nEndTime,
    cstr_t szContent, size_t nLen, bool bLyricsLine) {
    LyricsLine *pLine;

    pLine = new LyricsLine;
    pLine->nBegTime = nBegTime;
    pLine->nEndTime = nEndTime;
    pLine->bLyricsLine = bLyricsLine;

    if (szContent) {
        pLine->setContent(szContent, nLen);
    }

    return pLine;
}

LyricsLine *duplicateLyricsLine(LyricsLine *pLineSrc) {
    LyricsLine *pLine = new LyricsLine;
    pLine->nBegTime = pLineSrc->nBegTime;
    pLine->nEndTime = pLineSrc->nEndTime;
    pLine->bLyricsLine = pLineSrc->bLyricsLine;
    pLine->bTempLine = pLineSrc->bTempLine;
    pLine->bAdvertise = pLineSrc->bAdvertise;

    if (pLineSrc->szContent) {
        size_t n = strlen(pLineSrc->szContent) + 1;
        pLine->szContent = new char[n];
        strcpy_safe(pLine->szContent, n, pLineSrc->szContent);
    }

    for (int i = 0; i < (int)pLineSrc->vFrags.size(); i++) {
        LyricsPiece *pPieceSrc = pLineSrc->vFrags[i];

        int len = sizeof(LyricsPiece) + pPieceSrc->nLen * sizeof(char);
        LyricsPiece *pPiece = (LyricsPiece *)new uint8_t[len];
        memcpy(pPiece, pPieceSrc, len);

        pLine->vFrags.push_back(pPiece);
    }

    return pLine;
}

//////////////////////////////////////////////////////////////////////////

void CLyricsLines::clear() {
    if (m_bAutoFreeItem) {
        for (iterator it = begin(); it != end(); ++it) {
            LyricsLine *pLine = *it;
            delete pLine;
        }
    }
    vector<LyricsLine *>::clear();
}

void CLyricsLines::clearDrawContextWidth() {
    for (iterator it = begin(); it != end(); ++it) {
        LyricsLine *pLine = *it;
        for (int k = 0; k < (int)pLine->vFrags.size(); k++) {
            pLine->vFrags[k]->nDrawWidth = 0;
        }
    }
}

int CLyricsLines::getCountWithSameTimeStamps(int nLine) {
    if (nLine < 0 || nLine >= size()) {
        return 0;
    }

    LyricsLine *pLine = at(nLine);
    for (int i = nLine + 1; i < size(); i++) {
        LyricsLine *p = at(i);
        if (pLine->nBegTime != p->nBegTime || pLine->nEndTime != p->nEndTime) {
            return i - nLine;
        }
    }

    return (int)size() - nLine;
}
