#pragma once

#include "../Utils/Utils.h"


#define TEMP_TIME        -1

struct LyricsPiece
{
    bool                bTempBegTime, bTempEndTime;
    int                    nBegTime;
    int                    nEndTime;
    int16_t                nDrawWidth;        // The width of lyrics in Graph
    int16_t                nLen;            // length of szLyric
    char                szLyric[1];

    bool isTempBegTime() { return bTempBegTime; }

};

typedef vector<LyricsPiece *>        VecLyricsPiece;

class LyricsLine
{
public:
    bool            bAdvertise;
    bool            bTempLine;
    bool            bLyricsLine;

    // For lyrics line:
    VecLyricsPiece    vFrags;
    int                nBegTime;
    int                nEndTime;

    // For Non-lyrics line:
    char            *szContent;        // content of the row

    LyricsLine();

    ~LyricsLine();

    LyricsPiece *appendPiece(int nBegTime, int nEndTime, 
        cstr_t szLyrics, size_t nLen, bool bTempBegTime, bool bTempEndTime);

    void setContent(cstr_t szContent, size_t nLen);

    bool isTempBegTime()
    {
        if (vFrags.size())
            return vFrags[0]->isTempBegTime();

        return false;
    }

    bool isAdvertise() { return bAdvertise; }

    bool isTempLine() { return bTempLine; }
};

class CLyricsLines : public vector<LyricsLine *>
{
public:
    CLyricsLines(bool bAutoFreeItem = true) { m_bAutoFreeItem = bAutoFreeItem; }

    virtual ~CLyricsLines() { clear(); }

    void clear();

    void clearDrawContextWidth();

    int getCountWithSameTimeStamps(int nLine);

protected:
    bool        m_bAutoFreeItem;

};

LyricsLine *newLyricsLine(int nBegTime, int nEndTime, 
                          cstr_t szContent = nullptr, size_t nLen = 0, bool bLyricsLine = true);

LyricsLine *duplicateLyricsLine(LyricsLine *pLineSrc);
