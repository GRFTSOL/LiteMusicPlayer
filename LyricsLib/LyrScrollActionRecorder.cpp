#include "MLData.h"
#include "LyrScrollActionRecorder.h"


//////////////////////////////////////////////////////////////////////

#define AUTO_SCROLL_LYR_HEADERV0 "#"
#define AUTO_SCROLL_LYR_DATA_POSV0  3

/*
FORMAT of Lyrics Scrolling Events, version 0.0:

All data are base64.

1.    VERSION   CRC        SCROLL_EVENTS   SCROLL_EVENTS  .....
(1char)  (2 chars)    (n chars)

2.  SCROLL_EVENTS = TIME_DISTANCE_WIDTH   TIME        DISTANCE
(timeWidth)  (distanceWidth)

3. TIME_DISTANCE_WIDTH = timeWidth + distanceWidth * 10;

*/

#define EMPTY_LINE_CHAR_COUNT   5

#define MAX_SPEED                    ((25.0 / 1000) )
#define MIN_SPEED                    (1.0 / 1000)
#define OPTIMIZE_TIME_BLOCK            (10 * 1000)

const char *getBase64Set();

void intToBase64(int n, string &str) {
    str.clear();

    char szBuff[10];
    szBuff[CountOf(szBuff) - 1] = '\0';
    int i = CountOf(szBuff) - 1;
    const char *SZ_BASE64_SET = getBase64Set();

    do {
        i--;
        szBuff[i] = SZ_BASE64_SET[n % 64];
        n /= 64;
    }
    while (n > 0);

    str = szBuff + i;
}

uint32_t intFromBase64(cstr_t str, int nLen) {
    uint32_t n = 0;
    const char *SZ_BASE64_SET = getBase64Set();
    for (; nLen > 0; nLen--) {
        cstr_t p = strchr(SZ_BASE64_SET, *str);
        assert(p);
        if (p) {
            n = n * 64 + (int)(p - SZ_BASE64_SET);
        }
        str++;
    }
    return n;
}

uint8_t CRC8(cstr_t str) {
    uint8_t crc = 0;
    while (*str) {
        crc += (uint8_t)(*str);
        str++;
    }

    return crc;
}

#define MIN_DISTANCE_LINE   4
inline int charCountToCharDistance(int nLen) {
    if (nLen >= MIN_DISTANCE_LINE) {
        return nLen;
    } else {
        return MIN_DISTANCE_LINE;
    }
}

CLyrScrollActionRecorder::CLyrScrollActionRecorder() {
    m_nLyrCharCount = -1;
    m_bHasActions = false;
}

void CLyrScrollActionRecorder::initCreateAutoEvents(CLyricsLines &lyrLines) {
    CLyrScrollActionRecorder::close();

    m_listEvents.push_back(ScrollTo(0, 0));
    m_nLyrCharCount = countLyrChar(lyrLines);

    int nStartTime = 0;
    for (int i = 0; i < (int)lyrLines.size(); i++) {
        lyrLines[i]->nBegTime = nStartTime;
        nStartTime = MAX_TIME;
        lyrLines[i]->nEndTime = nStartTime;
    }
}

void CLyrScrollActionRecorder::close() {
    m_bHasActions = false;
    m_listEvents.clear();
    m_nLyrCharCount = -1;
}

bool CLyrScrollActionRecorder::isEventTag(cstr_t str) {
    return strncmp(str, AUTO_SCROLL_LYR_HEADERV0, strlen(AUTO_SCROLL_LYR_HEADERV0)) == 0;
}

string CLyrScrollActionRecorder::eventsDataToString() {
    int nLastTime = 0, nLastDistance = 0;

    // If events too less, don't store these actions.
    if (m_listEvents.size() <= 6 || !m_bHasActions) {
        return "";
    }

    string strData = AUTO_SCROLL_LYR_HEADERV0 "AA"; // "AA"(base64) = "00"(16#)

    string strTime, strDistance, strEventWidth;
    for (int i = 0; i < (int)m_listEvents.size(); i++) {
        intToBase64(m_listEvents[i].nTime - nLastTime, strTime);
        intToBase64(m_listEvents[i].distance - nLastDistance, strDistance);
        int nEventWidth = (int)(strTime.size() + strDistance.size() * 10);
        assert(nEventWidth < 60);
        intToBase64(nEventWidth, strEventWidth);

        strData.append(strEventWidth.c_str());
        strData.append(strTime.c_str());
        strData.append(strDistance.c_str());

        nLastTime = m_listEvents[i].nTime;
        nLastDistance = m_listEvents[i].distance;
    }

    uint8_t crc = CRC8(strData.c_str() + AUTO_SCROLL_LYR_DATA_POSV0);
    intToBase64(crc, strTime);
    for (int k = 0; k < (int)strTime.size(); k++) {
        strData[AUTO_SCROLL_LYR_DATA_POSV0 - strTime.size() + k] = strTime[k];
    }

    return strData;
}

void CLyrScrollActionRecorder::eventsDataFromString(cstr_t szEventsData, CLyricsLines &lyrLines) {
    initCreateAutoEvents(lyrLines);

    if (strncmp(szEventsData, AUTO_SCROLL_LYR_HEADERV0, strlen(AUTO_SCROLL_LYR_HEADERV0)) != 0) {
        return;
    }

    m_bHasActions = true;

    uint8_t crc = CRC8(szEventsData + AUTO_SCROLL_LYR_DATA_POSV0);
    uint8_t crcOrg = intFromBase64(szEventsData + strlen(AUTO_SCROLL_LYR_HEADERV0), 2);
    if (crc != crcOrg) {
        ERR_LOG0("FAILED to Verify lyrics scroll event crc value.");
        return;
    }

    int nCurTime = 0, nCurDistance = 0;
    cstr_t p = szEventsData + AUTO_SCROLL_LYR_DATA_POSV0;
    cstr_t pEnd = p + strlen(p);

    while (p < pEnd) {
        int nEventWidth = intFromBase64(p, 1);
        p++;
        if (p >= pEnd || nEventWidth <= 0 || nEventWidth >= 60) {
            break;
        }
        uint8_t widthTime = nEventWidth % 10;
        uint8_t widthDistance = nEventWidth / 10;

        if (p + widthTime + widthDistance > pEnd) {
            break;
        }

        nCurTime += intFromBase64(p, widthTime);
        p += widthTime;

        nCurDistance += intFromBase64(p, widthDistance);
        p += widthDistance;

        addScrollTo(lyrLines, nCurDistance, nCurTime, false);
    }

    updateTimeTagByEvents(lyrLines);
}

void CLyrScrollActionRecorder::scrollToLine(CLyricsLines &lyrLines, int nLine, int nTime, bool bUpdateTimeTag) {
    m_bHasActions = true;

    addScrollTo(lyrLines, getDistanceOfLine(lyrLines, nLine), nTime, bUpdateTimeTag);
}

void CLyrScrollActionRecorder::addScrollTo(CLyricsLines &lyrLines, int nPos, int nTime, bool bUpdateTimeTag) {
    assert(m_listEvents.size() >= 1);
    const int RECENT_ACTION = 500;

    // remove events conflicted with inserted item.
    // nTime + RECENT_ACTION < Event's within < nTime + RECENT_ACTION
    // m_listEvents[0] is fixed event, don't remove it.
    for (int k = 1; k < (int)m_listEvents.size(); ) {
        if (m_listEvents[k].distance == nPos) {
            DBG_LOG1("remove distance conflict event: position: %d", nPos);
            m_listEvents.erase(m_listEvents.begin() + k);
            continue;
        }

        if (m_listEvents[k].nTime <= nTime - RECENT_ACTION) {
            k++;
            continue;
        } else if (m_listEvents[k].nTime >= nTime + RECENT_ACTION) {
            break;
        } else {
            DBG_LOG2("remove time conflict event: %d, time: %d", k, m_listEvents[k].nTime);
            m_listEvents.erase(m_listEvents.begin() + k);
        }
    }

    for (int k = 1; k < (int)m_listEvents.size(); ) {
        if ((m_listEvents[k].distance <= nPos && m_listEvents[k].nTime >= nTime)
            || (m_listEvents[k].distance >= nPos && m_listEvents[k].nTime <= nTime)) {
            DBG_LOG2("remove time or distance conflict event: %d, time: %d", k, m_listEvents[k].nTime);
            m_listEvents.erase(m_listEvents.begin() + k);
            continue;
        }
        k++;
    }

    int i;
    for (i = 0; i < (int)m_listEvents.size(); i++) {
        if (nPos < m_listEvents[i].distance) {
            m_listEvents.insert(m_listEvents.begin() + i, ScrollTo(nPos, nTime));
            break;
        }
    }

    if (i ==  m_listEvents.size()) {
        m_listEvents.push_back(ScrollTo(nPos, nTime));
    }

    if (bUpdateTimeTag) {
        updateTimeTagByEvents(lyrLines);
    }
}

void CLyrScrollActionRecorder::updateTimeTagByEvents(CLyricsLines &lyrLines) {
    if (m_nLyrCharCount == 0 || m_listEvents.empty()) {
        return;
    }

    double speedlast = 0.0;
    int nTimeLast = 0;
    // Calculate the speed of every events
    for (int i = 0; i < (int)m_listEvents.size() - 1; i++) {
        m_listEvents[i].speed = (m_listEvents[i + 1].distance - m_listEvents[i].distance)
        / (double) (m_listEvents[i + 1].nTime - m_listEvents[i].nTime);

        if (i == 0) {
            continue;
        }

        if (m_listEvents[i].speed > MAX_SPEED && m_listEvents[i].distance <= EMPTY_LINE_CHAR_COUNT) {
            // remove events speed too fast
            DBG_LOG2("remove events too fast: speed1: %f, speed2: %f", m_listEvents[i].speed, MAX_SPEED);
            m_listEvents.erase(m_listEvents.begin() + i);
            i -= 2;
        } else if (speedlast - m_listEvents[i].speed <= MIN_SPEED && speedlast - m_listEvents[i].speed >= -MIN_SPEED
            && m_listEvents[i].nTime - nTimeLast < OPTIMIZE_TIME_BLOCK) {
            // remove speed same
            DBG_LOG2("remove events: speed1: %f, speed2: %f", speedlast, m_listEvents[i].speed);
            m_listEvents.erase(m_listEvents.begin() + i);
            i -= 2;
        } else {
            nTimeLast = m_listEvents[i].nTime;
            speedlast = m_listEvents[i].speed;
        }
    }

    int nCurEvent = 0;
    int nCurDistance = 0;
    int nLastTime = 0;

    for (int i = 0; i < (int)lyrLines.size(); i++) {
        LyricsLine *pLine = lyrLines[i];
        assert(pLine->vFrags.size() == 1);
        if (pLine->vFrags.size() != 1) {
            continue;
        }

        LyricsPiece *pPiece = pLine->vFrags[0];
        pPiece->nBegTime = nLastTime;
        nCurDistance += charCountToCharDistance(pPiece->nLen);

        while (nCurEvent < (int)m_listEvents.size() - 1
            && nCurDistance >= m_listEvents[nCurEvent].distance) {
            nCurEvent++;
        }
        if (nCurEvent > 0 && m_listEvents[nCurEvent].distance >= nCurDistance) {
            nCurEvent--;
        }
        if (nCurEvent == m_listEvents.size() - 1) {
            // in last event, set end time as MAX_TIME.
            pPiece->nEndTime = MAX_TIME;
        } else {
            ScrollTo *pEvent = &(m_listEvents[nCurEvent]);
            assert(nCurDistance >= pEvent->distance);

            if (pEvent->speed > 0) {
                pPiece->nEndTime = int(pEvent->nTime + (nCurDistance - pEvent->distance) / pEvent->speed);
            } else {
                pPiece->nEndTime = pEvent->nTime;
            }
        }

        pLine->nBegTime = pPiece->nBegTime;
        nLastTime = pLine->nEndTime = pPiece->nEndTime;
    }
}

int CLyrScrollActionRecorder::getDistanceOfLine(CLyricsLines &lyrLines, int nLine) {
    if (nLine > (int)lyrLines.size()) {
        nLine = (int)lyrLines.size();
    }

    // count all the characters.
    int nCharCount = 0;
    for (int i = 0; i < nLine; i++) {
        LyricsLine *pLine = lyrLines[i];
        assert(pLine->bLyricsLine);
        assert(pLine->vFrags.size() == 1);
        if (pLine->vFrags.size() != 1) {
            continue;
        }
        nCharCount += charCountToCharDistance(pLine->vFrags[0]->nLen);
    }

    return nCharCount;
}

int CLyrScrollActionRecorder::countLyrChar(CLyricsLines &lyrLines) {
    return getDistanceOfLine(lyrLines, (int)lyrLines.size());
}


#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

IMPLEMENT_CPPUNIT_TEST_REG(CLyrScrollActionRecorder)

class CTestCaseLyrScrollActionRecorder : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CTestCaseLyrScrollActionRecorder);
    CPPUNIT_TEST(testAddLyrEvent);
    CPPUNIT_TEST_SUITE_END();

protected:

public:
    void setUp() {
    }
    void tearDown() {
    }

protected:
    void testAddLyrEvent() {
        // CPPUNIT_ASSERT(strcmp(szStr, "abcdef1213") == 0);
        // CPPUNIT_FAIL_T(stringPrintf("strSplit(set) test, case(Can't find): %d, %s", i, strResult[i]).c_str());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLyrScrollActionRecorder);


#endif // _CPPUNIT_TEST
