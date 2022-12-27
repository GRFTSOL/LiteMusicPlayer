#include "MLData.h"
#include "LyrTimeStamp.h"


#define HEADER_TIME_STAMP_V0 '@'
#define CHAR_NO_TIME_STAMP  '_'
#define CHAR_MAX_TIME_VALUE '^'
#define CHAR_NEGATIVE_TIME_VALUE '-'
#define TIME_STAMP_UNIT         100
#define MAX_DURATION            64

bool CLyrTimeStamp::isTimeStamps(cstr_t szTimeStamps) {
    return szTimeStamps[0] == HEADER_TIME_STAMP_V0;
}

const char *getBase64Set();

void encodeTimeStamps(int duration, string &timeStamps) {
    const char *charSet = getBase64Set();
    if (duration < 0) {
        timeStamps += CHAR_NEGATIVE_TIME_VALUE;
        duration = -duration;
    }
    // duration can't be too large.
    if (duration > 10 * 60 * 1000) {
        duration = 6 * 1000;
    }

    duration /= TIME_STAMP_UNIT;

    while (duration >= 0) {
        if (duration >= MAX_DURATION) {
            timeStamps += CHAR_MAX_TIME_VALUE;
        } else {
            timeStamps += charSet[duration];
        }

        duration -= MAX_DURATION;
    }
}

cstr_t decodeTimeStamps(cstr_t szTimeStamps, int &duration) {
    assert(*szTimeStamps != CHAR_NO_TIME_STAMP);

    duration = 0;

    bool bNegative = (*szTimeStamps == CHAR_NEGATIVE_TIME_VALUE);
    if (bNegative) {
        szTimeStamps++;
    }

    while (*szTimeStamps == CHAR_MAX_TIME_VALUE) {
        duration += MAX_DURATION;
        szTimeStamps++;
    }

    const char *charSet = getBase64Set();
    int i = 0;
    for (i = 0; i < MAX_DURATION; i++) {
        if (charSet[i] == *szTimeStamps) {
            duration += i;
            break;
        }
    }
    assert(i < MAX_DURATION);

    duration *= TIME_STAMP_UNIT;

    if (bNegative) {
        duration = -duration;
    }

    return szTimeStamps + 1;
}

bool CLyrTimeStamp::parse(cstr_t szTimeStamps, CMLData *pMLData) {
    // Header
    if (szTimeStamps[0] != HEADER_TIME_STAMP_V0) {
        return false;
    }
    szTimeStamps++;

    // offset time
    int nTimeOffset = 0;
    szTimeStamps = decodeTimeStamps(szTimeStamps, nTimeOffset);

    // Time of every line
    int nTimeAboveLine = 0;
    CLyricsLines &lines = pMLData->getRawLyrics();
    for (CLyricsLines::iterator it = lines.begin(); it != lines.end() && *szTimeStamps;) {
        LyricsLine *pLine = *it;
        LyricsPiece *pPiece = pLine->vFrags[0];
        assert(pLine->bLyricsLine && pPiece->isTempBegTime());

        if (*szTimeStamps == CHAR_NO_TIME_STAMP) {
            szTimeStamps++;
            // Just remove this line from RawLyrics (will be kept in m_arrFileLines).
            it = lines.erase(it);
            pLine->bLyricsLine = false;
            pLine->setContent(pPiece->szLyric, strlen(pPiece->szLyric));
        } else {
            int duration;
            szTimeStamps = decodeTimeStamps(szTimeStamps, duration);
            pPiece->nBegTime = pLine->nBegTime = nTimeAboveLine + duration;
            pPiece->bTempBegTime = false;

            nTimeAboveLine = pLine->nBegTime;
            ++it;
        }
    }

    pMLData->properties().setTxtLyrScrollEvents("");
    pMLData->properties().setOffsetTime(nTimeOffset);

    return true;
}

void CLyrTimeStamp::toString(string &strTimeStamps, int nOffsetTime, CLyricsLines &lyrLines) {
    // Header
    strTimeStamps.clear();
    strTimeStamps += HEADER_TIME_STAMP_V0;

    // offset time
    encodeTimeStamps(nOffsetTime, strTimeStamps);

    // Time of every line
    int nTimeAboveLine = 0;
    for (uint32_t i = 0; i < lyrLines.size(); i++) {
        LyricsLine *pLine = lyrLines[i];
        if (pLine->bLyricsLine) {
            int duration = pLine->nBegTime - nTimeAboveLine + TIME_STAMP_UNIT / 2;
            encodeTimeStamps(duration, strTimeStamps);
            // Round the above line.
            nTimeAboveLine += duration / TIME_STAMP_UNIT * TIME_STAMP_UNIT;
        } else {
            strTimeStamps += CHAR_NO_TIME_STAMP;
        }
    }
}
