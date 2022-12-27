/********************************************************************
    Created  :    2002/01/03    17:23:02
    FileName :    LrcParser.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "MLLib.h"
#include "LyricsParser.h"
#include "LrcParser.h"
#include "LrcTag.h"
#include "LyrScrollActionRecorder.h"
#include "LrcParserHelper.h"
#include "LyrTimeStamp.h"
#include "../Skin/SkinTypes.h"


void removeExtraBlankLine(string &str) {
    bool bAppendNewLine = (str.size() > 0 && str[str.size() - 1] == '\n');
    trimStr(str, "\r\n");

    size_t oldSize = str.size();

    while (true) {
        // Replace 2 blank lines to 1 blank line
        strrep(str, "\r\n\r\n\r\n", "\r\n\r\n");

        cstr_t p = str.c_str();
        int nLineCount = 0, nBlankLineCount = 0;
        bool bPrevEndLine = false;

        while (*p) {
            while (*p && *p != '\n') {
                if (*p != '\r') {
                    bPrevEndLine = false;
                }
                p++;
            }
            if (*p == '\0') {
                break;
            }
            p++;

            if (bPrevEndLine) {
                nBlankLineCount++;
            }
            nLineCount++;
            bPrevEndLine = true;
        }

        if (nBlankLineCount >= nLineCount * 2 / 5) {
            strrep(str, "\r\n\r\n", "\r\n");
        }

        if (oldSize == str.size()) {
            break;
        }
        oldSize = str.size();
    }

    if (bAppendNewLine) {
        str += "\r\n";
    }
}

bool readIntValue(cstr_t &szBeg, cstr_t szEnd, int nSize, int &value) {
    cstr_t p = szBeg;
    value = 0;
    while (isDigit(*p) && nSize > 0 && p < szEnd) {
        value *= 10;
        value += *p - '0';
        p++;
        nSize--;
    }

    if (nSize == 0) {
        szBeg = p;
        return true;
    }

    return false;
}

// [mm:ss.xx]word 1<mm:ss.xx> word 2 <mm:ss.xx> ...
cstr_t eatFrag(cstr_t szLyricsBeg, int &nEndTime, string &buffLyrics) {
    cstr_t szPos;
    cstr_t szLyricsEnd = szLyricsBeg + strlen(szLyricsBeg);

    buffLyrics.clear();
    szPos = szLyricsBeg;
    while (szPos < szLyricsEnd) {
        if (*szPos == '<') {
            int n;
            cstr_t szTemp = szPos + 1;
            nEndTime = 0;
            if (readIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == ':') {
                szTemp++;
                nEndTime = n * 60 * 1000;
                if (readIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == '.') {
                    szTemp++;
                    nEndTime += n * 1000;
                    if (readIntValue(szTemp, szLyricsEnd, 3, n) && *szTemp == '>') {
                        szTemp++;
                        nEndTime += n;
                        buffLyrics.append(szLyricsBeg, int(szPos - szLyricsBeg));
                        return szTemp;
                    } else if (readIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == '>') {
                        szTemp++;
                        nEndTime += n * 10;
                        buffLyrics.append(szLyricsBeg, int(szPos - szLyricsBeg));
                        return szTemp;
                    }
                }
            }
        }
        szPos++;
    }

    buffLyrics.append(szLyricsBeg, int(szLyricsEnd - szLyricsBeg));
    return szLyricsEnd;
}

//////////////////////////////////////////////////////////////////////////

CLrcParser::CLrcParser(CMLData *pMLData, bool bTurnOffPref) : CLyricsParser(pMLData) {
    m_lyrContentType = LCT_LRC;
    m_bTurnOffPref = bTurnOffPref;
}

CLrcParser::~CLrcParser() {
}

int CLrcParser::parseFile(bool bUseSpecifiedEncoding, CharEncodingType encoding) {
    string lyrics;

    if (!isFileExist(m_pMLData->getLyricsFileName())) {
        return ERR_FILE_NOT_EXIST;
    }

    if (!readFile(m_pMLData->getLyricsFileName(), lyrics)) {
        return ERR_READ_FILE;
    }

    return parseString((uint8_t *)lyrics.data(), (int)lyrics.size(), false, bUseSpecifiedEncoding, encoding);
}

int CLrcParser::saveAsFile(cstr_t file) {
    string strData;

    bool bToTxtFormat = tobool(fileIsExtSame(file, ".txt"));
    toString(strData, true, bToTxtFormat);

    if (!writeFile(file, strData)) {
        return ERR_WRITE_FILE;
    }

    return ERR_OK;
}

int CLrcParser::parseString(cstr_t szText, bool bAddTimeStampForTextLine) {
    if (isEmptyString(szText)) {
        return ERR_EMPTY_LYRICS;
    }

    CLyrTagNameValueMap propNameValueMap(m_pMLData->properties());

    string strLine;

    //
    // parse .lrc line by line
    //
    cstr_t szNextLine = szText;
    while (szNextLine) {
        szNextLine = readLine(szNextLine, strLine);

        int nNextPos, nTime;
        vector<int> vTimes;
        cstr_t szLine = strLine.c_str();

        szLine = ignoreSpaces(szLine);

        if (*szLine != '[') {
            // NOT lrc tag or time stamps.
            if (bAddTimeStampForTextLine) {
                addStrAsTimedLine(szLine);
            } else {
                addStrAsUnknowLine(szLine);
            }
            continue;
        }

        while (parseLrcTimeTag(szLine, nNextPos, nTime)) {
            szLine += nNextPos;
            vTimes.push_back(nTime);
        }
        if (vTimes.size()) {
            // Time tag.
            addLyrLines(vTimes, szLine);
            continue;
        }

        // parse lrc tag.
        int propIndex = propNameValueMap.parseLrcTag(szLine, (int)strlen(szLine));
        if (propIndex == -1) {
            addStrAsUnknowLine(szLine);
        }
    }

    if (m_pMLData->getLyricsLinesCount() == 0 && !bAddTimeStampForTextLine) {
        // .txt lyrics
        m_pMLData->clearLyrics();

        // parse as text lyrics
        parseString(szText, true);

        // parse text tags from lyrics text.
        parseTxtTagInLyrics();

        m_lyrContentType = LCT_TXT;

        // Does text tag including time stamps.
        string strTimeStamps = m_pMLData->getTxtLyrScrollEvents();
        if (CLyrTimeStamp::isTimeStamps(strTimeStamps.c_str())) {
            if (CLyrTimeStamp::parse(strTimeStamps.c_str(), m_pMLData)) {
                m_lyrContentType = LCT_LRC;
            }
        }

        finalFixOnLyrics(bAddTimeStampForTextLine);
    } else {
        m_lyrContentType = LCT_LRC;

        finalFixOnLyrics(bAddTimeStampForTextLine);
    }

    return ERR_OK;
}

void CLrcParser::finalFixOnLyrics(bool bAddTimeStampForTextLine) {
    //
    // Extra fix or check on lyrics time tags.
    //
    if (m_lyrContentType == LCT_TXT) {
        removeExtraBlankLinesOfTextLyr();

        fixTextLrcTimeTag();
        return;
    }

    if (!bAddTimeStampForTextLine) {
        lRCCheckRowTime();
    } else {
        //
        // 将未设置时间标签的歌词行的时间设置为－1，再调用checkLyricsLineTime重新
        // 校正时间。注意：不能调用lRCCheckRowTime()函数
        CLyricsLines &lyrLines = m_pMLData->getRawLyrics();
        for (int i = 0; i < (int)lyrLines.size(); i++) {
            LyricsLine *pLine = lyrLines[i];
            if (pLine->vFrags.size()) {
                LyricsPiece *pPiece = pLine->vFrags[0];
                if (pPiece->bTempBegTime) {
                    pPiece->nBegTime = -1;
                    pPiece->nEndTime = -1;
                    pLine->nBegTime = -1;
                    pLine->nEndTime = -1;
                }
            }
        }

        m_pMLData->checkLyricsLineTime();
    }

    m_pMLData->fixExtendedLrcTimeStamps();
}

//
// parse .lrc or .txt lyrics text
//
int CLrcParser::parseString(uint8_t *szLrc, size_t nLen, bool bAddTimeStampForTextLine, bool bUseSpecifiedEncoding, CharEncodingType encoding) {
    string lyricsUtf8 = convertLyricsToUtf8(szLrc, nLen, bUseSpecifiedEncoding, encoding);

    return parseString(lyricsUtf8.c_str(), bAddTimeStampForTextLine);
}

void CLrcParser::parseTxtTagInLyrics() {
    CLyrTagNameValueMap propNameValueMap(m_pMLData->properties());
    CLyricsLines &lyrLines = m_pMLData->getRawLyrics();

    // For some duplicated tags (lyrics file are not well written,
    // we need to filter those tags).

    //
    // parse text tags in the file header.
    //
    for (int i = 0; i < (int)lyrLines.size(); i++) {
        LyricsLine *pLine = lyrLines[i];
        assert(pLine->vFrags.size() == 1);
        LyricsPiece *pPiece = pLine->vFrags[0];
        assert(pPiece->isTempBegTime());

        // parse Txt tag.
        int propIndex = propNameValueMap.parseTxtTag(pPiece->szLyric, (int)strlen(pPiece->szLyric));
        if (propIndex != -1) {
            if (isTxtTagAtTailOfFile(propNameValueMap.getTagType(propIndex))) {
                // Encoding tag is very special, it may be defined by String BOM.
                if (propNameValueMap.getTagType(propIndex) != LTT_ENCODING) {
                    propNameValueMap.clearValue(propIndex);
                }
            } else {
                propNameValueMap.remove(propIndex);
                m_pMLData->removeLyricsLine(i);
                i--;
            }
        }
    }

    //
    // parse text tags at the end of file.
    //
    for (int i = (int)lyrLines.size() - 1; i >= 0; i--) {
        LyricsLine *pLine = lyrLines[i];
        assert(pLine->vFrags.size() == 1);
        LyricsPiece *pPiece = pLine->vFrags[0];
        assert(pPiece->isTempBegTime());

        // parse Txt tag.
        int propIndex = propNameValueMap.parseTxtTag(pPiece->szLyric, (int)strlen(pPiece->szLyric));
        if (propIndex != -1) {
            propNameValueMap.remove(propIndex);
            m_pMLData->removeLyricsLine(i);
        }
    }
}

void CLrcParser::toString(string &str, bool bIncTags, bool bToTxtFormat) {
    if (bToTxtFormat) {
        toTxtString(str, bIncTags);
    } else {
        toLrcString(str, bIncTags, true);
    }

    if (!m_bTurnOffPref) {
        // remove extra blank line only work under !turn of preferences.
        if (!bToTxtFormat
            || !g_profile.getBool("AppendTimeStampsInTxtLyr", true)
            || m_lyrContentType == LCT_TXT) {
            if (g_profile.getBool("RemoveExtraBlankLines", true)) {
                removeExtraBlankLine(str);
            }
        }
    }
}

void CLrcParser::toLrcString(string &str, bool bIncTags, bool bLongTimeFmt) {
    // reserver 4k spaces for lyrics data.
    str.reserve(1024 * 4);

    if (bIncTags) {
        CLyrTagNameValueMap propNameValueMap(m_pMLData->properties(), LTT_ALL_TAG & ~ LTT_ENCODING);

        for (int i = 0; i < propNameValueMap.getCount(); i++) {
            string strValue = propNameValueMap.getValue(i);
            if (strValue.size()) {
                str += propNameValueMap.getLrcTagString(i);
            }
        }
    }

    if (!bLongTimeFmt) {
        // test whether is long time format
        for (int i = 0; i < (int)m_pMLData->m_arrFileLines.size(); i++) {
            LyricsLine *pLine = m_pMLData->m_arrFileLines[i];
            if (pLine->vFrags.empty()) {
                continue;
            }

            if (!pLine->vFrags[0]->bTempBegTime) {
                if (pLine->nBegTime % 1000 != 0) {
                    bLongTimeFmt = true;
                    break;
                }
            }
        }
    }

    //
    // save every lyrics line
    string strBuff;
    for (int i = 0; i < (int)m_pMLData->m_arrFileLines.size(); i++) {
        LyricsLine *pLine = m_pMLData->m_arrFileLines[i];
        assert(pLine);

        if (lyricsLineToLrcString(pLine, bLongTimeFmt, strBuff)) {
            str += strBuff;
            str += "\r\n";
        }
    }

    if (str.size() > 2 && str[str.size() - 1] == '\n') {
        str.resize(str.size() - 2);
    }
}

void CLrcParser::toTxtString(string &str, bool bIncTags) {
    // reserver 4k spaces for lyrics data.
    str.reserve(1024 * 4);

    //
    // Convert and remove lyrics line which is text lyrics tag.
    //
    string strBuff;
    CLyricsLines lyrLines(false);
    LyricsProperties properties;
    CLyrTagNameValueMap propNameValueMap(properties);

    bool bRemoveExtraBlankLines = false;
    if (!m_bTurnOffPref) {
        bRemoveExtraBlankLines = g_profile.getBool("RemoveExtraBlankLines", true);
    }
    bool bLastLineBlank = false;

    for (int i = 0; i < (int)m_pMLData->m_arrFileLines.size(); i++) {
        LyricsLine *pLine = m_pMLData->m_arrFileLines[i];
        if (pLine->bLyricsLine) {
            bLastLineBlank = false;
            lyricsLineToTxtString(pLine, strBuff);
            if (propNameValueMap.parseTxtTag(strBuff.c_str(), (int)strBuff.size()) == -1) {
                lyrLines.push_back(pLine);
                str += strBuff;
                str += "\r\n";
            }
        } else {
            // Don't write extra blank lines.
            if (bRemoveExtraBlankLines && bLastLineBlank) {
                continue;
            }

            lyrLines.push_back(pLine);
            // Only append blank spaces, not "pLine->szContent";
            str += "\r\n";
            bLastLineBlank = true;
        }
    }

    if (bIncTags) {
        // Make a new copy of lyrics properties, we may modify it and do not want to save it.
        LyricsProperties properties = m_pMLData->properties();
        if (m_pMLData->getLyrContentType() == LCT_LRC) {
            // Convert to timestamps.
            string strTimeStamps;
            CLyrTimeStamp::toString(strTimeStamps, m_pMLData->getOffsetTime(), lyrLines);
            properties.setTxtLyrScrollEvents(strTimeStamps.c_str());
        }

        CLyrTagNameValueMap propNameValueMap(properties, LTT_ALL_TAG & ~ LTT_ENCODING);

        string strTagsAtEndOfFile, strTagsAtBeginOfFile;
        for (int i = 0; i < propNameValueMap.getCount(); i++) {
            string strValue = propNameValueMap.getValue(i);
            if (strValue.size() && propNameValueMap.getTxtName(i).size()) {
                string strTagLine = propNameValueMap.getTxtTagString(i);
                if (isTxtTagAtTailOfFile(propNameValueMap.getTagType(i))) {
                    strTagsAtEndOfFile += strTagLine;
                } else {
                    strTagsAtBeginOfFile += strTagLine;
                }
            }
        }

        if (strTagsAtBeginOfFile.size()) {
            str.insert(0, strTagsAtBeginOfFile.c_str());
        }

        if ((m_bTurnOffPref || g_profile.getBool("AppendTimeStampsInTxtLyr", true))
            && strTagsAtEndOfFile.size()) {
            if (str.size() && str[str.size() - 1] != '\n') {
                str += "\r\n";
            }
            str += strTagsAtEndOfFile;
        }
    }

    if (str.size() > 2 && str[str.size() - 2] == '\r' && str[str.size() - 1] == '\n') {
        str.resize(str.size() - 2);
    }
}

//
bool CLrcParser::lyricsLineToLrcString(LyricsLine *pLine, bool bLongTimeFmt, string &strBuff) {
    assert(pLine);
    strBuff.clear();

    if (pLine->isTempLine()) {
        return false;
    }

    if (!pLine->bLyricsLine) {
        assert(pLine->szContent);
        strBuff = pLine->szContent;
        return true;
    }

    if (pLine->vFrags.size() && !(pLine->vFrags[0]->bTempBegTime)) {
        strBuff = formtLrcTimeTag(pLine->nBegTime, bLongTimeFmt);
    }

    char szTime[64];
    for (int i = 0; i < (int)pLine->vFrags.size(); i++) {
        LyricsPiece *pPiece = pLine->vFrags[i];

        strBuff += pPiece->szLyric;

        if (i + 1 < (int)pLine->vFrags.size()) {
            pPiece = pLine->vFrags[i + 1];
            if (!pPiece->bTempBegTime) {
                int nMin, nSec, nMs;
                int nTime = pPiece->nBegTime + 10 / 2;

                nMs = (nTime / 10) % 100;
                nSec = (nTime / 1000) % 60;
                nMin = nTime / (1000 * 60);
                snprintf(szTime, CountOf(szTime), "<%02d:%02d.%02d>", nMin, nSec, nMs);
                strBuff += szTime;
            }
        }
    }
    if (strBuff.empty()) {
        return false; // do not insert temp empty lyrics
    }

    return true;
}

//
void CLrcParser::lyricsLineToTxtString(LyricsLine *pLine, string &strBuff) {
    assert(pLine->bLyricsLine);
    strBuff.clear();

    if (pLine->isTempLine()) {
        return;
    }

    for (int i = 0; i < (int)pLine->vFrags.size(); i++) {
        LyricsPiece *pPiece = pLine->vFrags[i];
        strBuff += pPiece->szLyric;
    }
}

//
// add lyrics lyrics line
// [mm:ss.xx]word 1<mm:ss.xx> word 2 <mm:ss.xx> ...
void CLrcParser::addLyrLines(vector<int> &vTimes, cstr_t szLyrLine) {
    int nBegTime, nEndTime;
    string buff;

    for (int i = 0; i < (int)vTimes.size(); i++) {
        nBegTime = vTimes[i];
        LyricsLine *pLine = newLyricsLine(nBegTime, -1);

        cstr_t szLyricsBeg = szLyrLine;
        do {
            nEndTime = -1;
            szLyricsBeg = eatFrag(szLyricsBeg, nEndTime, buff);

            pLine->appendPiece(nBegTime, nEndTime, buff.c_str(), (int)buff.size(), false, true);

            if (nEndTime != -1) {
                nBegTime = nEndTime;
            }
        }
        while (*szLyricsBeg);

        m_nLatestTime = nBegTime;

        if (pLine->vFrags.size() > 1) {
            m_pMLData->setKaraokeStyle(true);
        }

        m_pMLData->addLineByTime(pLine);
    }

    m_lyrContentType = LCT_LRC;
}


//
// add text file lyrics row
//
void CLrcParser::addStrAsTimedLine(const char *szLine) {
    LyricsLine *pLine = newLyricsLine(m_nLatestTime, -1);
    pLine->appendPiece(m_nLatestTime, -1, szLine, (int)strlen(szLine), true, true);

    m_pMLData->addLineByTime(pLine);
}

void CLrcParser::addStrAsUnknowLine(const char *szLine) {
    // Unknown tag, just add it in the file.
    LyricsLine *pLine = newLyricsLine(0, 0, szLine, (int)strlen(szLine), false);
    m_pMLData->addLine(pLine);
}

void CLrcParser::fixTextLrcTimeTag() {
    if (m_pMLData->getLyricsLinesCount() == 0) {
        return;
    }

    int nBegTime, nEndTime, nSpan;

    nEndTime = m_pMLData->getMediaLength();
    if (nEndTime < 0) {
        nEndTime = 5 * 60 * 1000; // 5 minutes
    }

    nBegTime = nEndTime / 15;
    if (nBegTime > 15) {
        nBegTime = 15;
    }

    nEndTime -= nBegTime;

    if (m_pMLData->getLyricsLinesCount() == 1) {
        nSpan = 0;
    } else {
        nSpan = (nEndTime - nBegTime) / (m_pMLData->getLyricsLinesCount() - 1);
    }

    for (int i = 0; i < (int)m_pMLData->getLyricsLinesCount(); i++) {
        LyricsLine *pLine = m_pMLData->getLyricsLine(i);

        if (pLine->vFrags.size() > 0) {
            pLine->vFrags[0]->nBegTime = pLine->nBegTime = nBegTime;
            pLine->vFrags[0]->nEndTime = pLine->nEndTime = nBegTime + nSpan;
        }
        nBegTime += nSpan;
    }
}

void CLrcParser::removeExtraBlankLinesOfTextLyr() {
    int i, nBlankLines = 0;

    for (i = 0; i < (int)m_pMLData->getLyricsLinesCount(); i++) {
        LyricsLine *pLine = m_pMLData->getLyricsLine(i);

        if (pLine->vFrags.size() == 1) {
            if (isEmptyString(pLine->vFrags[0]->szLyric)) {
                nBlankLines++;
            } else {
                nBlankLines = 0;
            }
        } else if (pLine->vFrags.size() == 0) {
            nBlankLines++;
        } else {
            nBlankLines = 0;
        }
        if (nBlankLines >= 3) {
            // remove this line
            m_pMLData->removeLyricsLine(i);
            i--;
            nBlankLines--;
        }
    }

    //
    // if the blank lines take the half part of all lyrics, remove them all.
    //
    if (m_pMLData->getLyricsLinesCount() < 10) {
        return;
    }

    // get blank line count of lyrics.
    nBlankLines = 0;
    for (i = 0; i < (int)m_pMLData->getLyricsLinesCount(); i++) {
        LyricsLine *pLine = m_pMLData->getLyricsLine(i);

        if (pLine->vFrags.size() == 1) {
            if (isEmptyString(pLine->vFrags[0]->szLyric)) {
                nBlankLines++;
            }
        } else if (pLine->vFrags.size() == 0) {
            nBlankLines++;
        }
    }

    if (nBlankLines >= m_pMLData->getLyricsLinesCount() / 2) {
        nBlankLines = 0;
        for (i = 0; i < (int)m_pMLData->getLyricsLinesCount(); i++) {
            LyricsLine *pLine = m_pMLData->getLyricsLine(i);

            if (pLine->vFrags.size() == 1) {
                if (isEmptyString(pLine->vFrags[0]->szLyric)) {
                    nBlankLines++;
                }
            } else if (pLine->vFrags.size() == 0) {
                nBlankLines++;
            }
            if (nBlankLines > 0) {
                // remove this line
                m_pMLData->removeLyricsLine(i);
                i--;
                nBlankLines = 0;
            }
        }
    }
}

void CLrcParser::lRCCheckRowTime() {
    int nSize;
    int nSizeFrag;
    int i;

    nSize = (int)m_pMLData->getLyricsLinesCount();

    // 如果歌词段的开始时间 < 此行歌词的开始时间，则修改
    for (i = 0; i < nSize; i ++) {
        LyricsLine *pLine = m_pMLData->getLyricsLine(i);
        nSizeFrag = (int)pLine->vFrags.size();

        for (int k = 0; k < nSizeFrag; k++) {
            LyricsPiece *pPiece = pLine->vFrags[k];

            if (pPiece->nBegTime < pLine->nBegTime) {
                pPiece->nBegTime = pLine->nBegTime;
            }
        }
    }

    /*
    // 如果两行歌词的时间没有按先后顺序排，则修改之。
    for (i = 0; i < nSize; i ++)
    {
        LyricsLine *pLine;
        LyricsPiece *pPiece;
        int                k;
        int                nIdx;
        PLYRIC_ROW        pLine1;

        pLine = m_pMLData->m_arrLyricsLines[i];
        pLine = pLine1 = m_pMLData->m_arrLyricsLines[i];
        nIdx = i;
        for (k = i + 1; k < nSize; k ++)
        {
            PLYRIC_ROW    pLine2;
            pLine2 = (PLYRIC_ROW)m_pMLData->m_arrLyricsLines[k];
            if (pLine2->nBegTime < pLine->nBegTime )
            {
                pLine = pLine2;
                nIdx = k;
            }
        }
        // exchange prow and prow1
        if (pLine != pLine1)
        {
            m_pMLData->m_arrLyricsLines.setAt(nIdx, pLine1);
            m_pMLData->m_arrLyricsLines.setAt(i, pLine);
        }
    }*/

    // 检查每一行歌词中的所有歌词段是否正确，并且进行矫正。
    for (i = 0; i < nSize; i ++) {
        LyricsLine *pLine, *pLineNext;
        LyricsPiece *pPiece, *pPieceNext;

        pLine = m_pMLData->getLyricsLine(i);

        // 如果此行的开始时间小于结束时间
        if (pLine->nEndTime < pLine->nBegTime) {
            if (i + 1 >= nSize) {
                pLine->nEndTime = pLine->nBegTime + 10 * 1000; // delay ten second
            } else {
                pLineNext = m_pMLData->getLyricsLine(i + 1);
                pLine->nEndTime = pLineNext->nBegTime;
            }
        }

        // 对每一段歌词的时间正确性进行校验
        nSizeFrag = (int)pLine->vFrags.size();
        for (int k = 0; k < nSizeFrag; k++) {
            pPiece = pLine->vFrags[k];

            if (pPiece->nEndTime < pPiece->nBegTime) {
                if (k + 1 < nSizeFrag) {
                    pPieceNext = pLine->vFrags[k + 1];
                    pPiece->nEndTime = pPieceNext->nBegTime;
                } else {
                    pPiece->nEndTime = pLine->nEndTime;
                }
            } else {
                if (k + 1 < nSizeFrag) {
                    pPieceNext = pLine->vFrags[k + 1];
                    if (pPiece->nEndTime > pPieceNext->nBegTime) {
                        pPiece->nEndTime = pPieceNext->nBegTime;
                    }
                } else {
                    if (pPiece->nEndTime > pLine->nEndTime) {
                        pPiece->nEndTime = pLine->nEndTime;
                    }
                }
            }
        }
    }

    CLyricsLines &lyrLines = m_pMLData->getRawLyrics();

    // If a lyrics line is 0 length, recover to display multiple line at same time.
    for (int i = 0; i < lyrLines.size(); i++) {
        LyricsLine *pLine = lyrLines[i];
        if (pLine->nBegTime != pLine->nEndTime || pLine->vFrags.size() != 1) {
            continue;
        }

        int nextLine = i;
        for (; nextLine < lyrLines.size(); nextLine++) {
            LyricsLine *pNext = lyrLines[nextLine];
            if (pNext->nBegTime == pLine->nBegTime && pNext->nBegTime != pNext->nEndTime) {
                // next line is not 0 length.
                pLine->nEndTime = pNext->nEndTime;
                pLine->vFrags[0]->nEndTime = pNext->nEndTime;
                break;
            }

            if (pNext->nBegTime != pLine->nBegTime) {
                // Find till next line has different begin time.
                pLine->nEndTime = pNext->nBegTime;
                pLine->vFrags[0]->nEndTime = pNext->nBegTime;
                break;
            }
        }
    }
}


#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

void encodeTimeStamps(int duration, string &timeStamps);

cstr_t decodeTimeStamps(cstr_t szTimeStamps, int &duration);

class CTestCaseLrcParser : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CTestCaseLrcParser);
    CPPUNIT_TEST(test_EncodeTimeStamps);
    CPPUNIT_TEST(test_CLrcParser_Lrc);
    CPPUNIT_TEST(test_CLrcParser_Txt);
    CPPUNIT_TEST_SUITE_END();

protected:
    void test_EncodeTimeStamps() {
        for (int nDuration = -22 * 1000; nDuration < 1000 * 30; nDuration += 100) {
            string timeStamps;
            int nDuration2 = 0;

            encodeTimeStamps(nDuration, timeStamps);

            cstr_t szRet = decodeTimeStamps(timeStamps.c_str(), nDuration2);
            CPPUNIT_ASSERT(isEmptyString(szRet));
            CPPUNIT_ASSERT(nDuration2 == nDuration);
        }
    }

    void test_CLrcParser_Lrc() {
        cstr_t SZ_LYRICS_TEST = "[ar:artist]\n[ti:title]\n[encoding:big5]\n[offset:-500]\n[00:01.12]L1\n[00:02.01]\nDummy Line\n[00:03:01]L3\nL4\n\n";
        cstr_t valArtist = "artist";
        cstr_t valTitle = "title";
        cstr_t vLyrLines[] = { "L1", "", "L3" };
        int vTime[] = { 1 * 1000 + 120, 2 * 1000 + 10, 3 *1000 + 10 };

        string wStr;
        utf8ToUCS2(SZ_LYRICS_TEST, -1, wStr);

        string wLyrics;
        wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
        wLyrics.append(wStr.c_str(), wStr.size());

        string wLyricsBe;
        wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
        uCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

        string utf8Lyrics;
        utf8Lyrics.append(SZ_FE_UTF8);
        utf8Lyrics.append(SZ_LYRICS_TEST);

        uint8_t *vLyrics[] = { uint8_t *SZ_LYRICS_TEST, uint8_t *wLyrics.c_str(), uint8_t *wLyricsBe.c_str(), uint8_t *utf8Lyrics.c_str(), };
        int vLen[] = { strlen(SZ_LYRICS_TEST), wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR), utf8Lyrics.size(), };
        CharEncodingType vEncoding[] = { ED_BIG5, ED_UNICODE, ED_UNICODE_BIG_ENDIAN, ED_UTF8, };


        for (int i = 0; i < CountOf(vLyrics); i++) {
            CMLData data;
            CLrcParser parser(&data, true);

            int nRet = parser.parseString(vLyrics[i], vLen[i], false, false);
            CPPUNIT_ASSERT(nRet == ERR_OK);
            data.properties().m_lyrContentType = parser.getLyrContentType();

            CPPUNIT_ASSERT(strcmp(data.properties().m_strArtist.c_str(), valArtist) == 0);
            CPPUNIT_ASSERT(strcmp(data.properties().m_strTitle.c_str(), valTitle) == 0);
            CPPUNIT_ASSERT(data.properties().getEncodingIndex() == vEncoding[i]);

            CLyricsLines &lyrLines = data.getRawLyrics();

            for (int k = 0; k < lyrLines.size(); k++) {
                LyricsLine *pLine = lyrLines[k];

                CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
                CPPUNIT_ASSERT(k < CountOf(vLyrLines));
                CPPUNIT_ASSERT(strcmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
                CPPUNIT_ASSERT(pLine->vFrags[0]->nBegTime == vTime[k]);
            }
            CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));

            //
            // Cases on CLrcParser::toString
            //
            string strNewLyr;
            CLrcParser parser2(&data, true);
            parser2.toString(strNewLyr, true, false);

            CMLData data3;
            CLrcParser parser3(&data3, true);
            parser3.parseString(strNewLyr.c_str(), false);

            {
                // Compare toString data.
                CLrcTag tag(data.properties(), data3.properties());
                CPPUNIT_ASSERT(!tag.isTagChanged()); // data and data3 should be same.

                CPPUNIT_ASSERT(strcmp(data3.properties().m_strArtist.c_str(), valArtist) == 0);
                CPPUNIT_ASSERT(strcmp(data3.properties().m_strTitle.c_str(), valTitle) == 0);
                CPPUNIT_ASSERT(data3.properties().getEncodingIndex() == vEncoding[i]);

                CLyricsLines &lyrLines = data3.getRawLyrics();

                for (int k = 0; k < lyrLines.size(); k++) {
                    LyricsLine *pLine = lyrLines[k];

                    CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
                    CPPUNIT_ASSERT(k < CountOf(vLyrLines));
                    CPPUNIT_ASSERT(strcmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
                    CPPUNIT_ASSERT(pLine->vFrags[0]->nBegTime == vTime[k]);
                }
                CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));
            }

            //
            // Case: Convert to text including timestamps
            //
            string strTxtLyr;
            CLrcParser parserTxt(&data, true);
            parserTxt.toString(strTxtLyr, true, true);

            CMLData data5;
            CLrcParser parser5(&data5, true);
            parser5.parseString(strTxtLyr.c_str(), false);

            // The new parsed data5 shall be same with previous data.
            CPPUNIT_ASSERT(data5.properties().getOffsetTime() == data.getOffsetTime());

            // Compare line by line
            CLyricsLines &lyrLines2 = data5.getRawLyrics();
            for (int k = 0; k < lyrLines2.size(); k++) {
                LyricsLine *pLine = lyrLines2[k];

                CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
                CPPUNIT_ASSERT(strcmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
                // 100 = TIME UNIT (TIME_STAMP_UNIT) in timestamps.
                CPPUNIT_ASSERT(pLine->vFrags[0]->nBegTime == (vTime[k] + 50) / 100 * 100);
            }

        }
    }

    void test_CLrcParser_Txt() {
        cstr_t SZ_LYRICS_TEST = "Artist:artist\nTitle: title\nEncoding:big5\nL1\n\n\nL3\nL4\n\nL6";
        cstr_t valArtist = "artist";
        cstr_t valTitle = "title";
        cstr_t vLyrLines[] = { "L1", "", "", "L3", "L4", "", "L6" };

        string wStr;
        utf8ToUCS2(SZ_LYRICS_TEST, -1, wStr);

        string wLyrics;
        wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
        wLyrics.append(wStr.c_str(), wStr.size());

        string wLyricsBe;
        wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
        uCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

        string utf8Lyrics;
        utf8Lyrics.append(SZ_FE_UTF8);
        utf8Lyrics.append(SZ_LYRICS_TEST);

        uint8_t *vLyrics[] = { uint8_t *SZ_LYRICS_TEST, uint8_t *wLyrics.c_str(), uint8_t *wLyricsBe.c_str(), uint8_t *utf8Lyrics.c_str(), };
        int vLen[] = { strlen(SZ_LYRICS_TEST), wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR), utf8Lyrics.size(), };
        CharEncodingType vEncoding[] = { ED_BIG5, ED_UNICODE, ED_UNICODE_BIG_ENDIAN, ED_UTF8, };


        for (int i = 0; i < CountOf(vLyrics); i++) {
            CMLData data;
            CLrcParser parser(&data, true);

            int nRet = parser.parseString(vLyrics[i], vLen[i], false, false);
            CPPUNIT_ASSERT(nRet == ERR_OK);

            CPPUNIT_ASSERT(strcmp(data.properties().m_strArtist.c_str(), valArtist) == 0);
            CPPUNIT_ASSERT(strcmp(data.properties().m_strTitle.c_str(), valTitle) == 0);
            CPPUNIT_ASSERT(data.properties().getEncodingIndex() == vEncoding[i]);

            CLyricsLines &lyrLines = data.getRawLyrics();

            for (int k = 0; k < lyrLines.size(); k++) {
                LyricsLine *pLine = lyrLines[k];

                CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
                CPPUNIT_ASSERT(k < CountOf(vLyrLines));
                CPPUNIT_ASSERT(strcmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
            }
            CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));

            //
            // Cases on CLrcParser::toString
            //
            string strNewLyr;
            CLrcParser parser2(&data, true);
            parser2.toString(strNewLyr, true, true);

            CMLData data3;
            CLrcParser parser3(&data3, true);
            parser3.parseString(strNewLyr.c_str(), false);

            {
                // Compare toString data.
                CLrcTag tag(data.properties(), data3.properties());
                CPPUNIT_ASSERT(!tag.isTagChanged()); // data and data3 should be same.

                CPPUNIT_ASSERT(strcmp(data3.properties().m_strArtist.c_str(), valArtist) == 0);
                CPPUNIT_ASSERT(strcmp(data3.properties().m_strTitle.c_str(), valTitle) == 0);
                CPPUNIT_ASSERT(data3.properties().getEncodingIndex() == vEncoding[i]);

                CLyricsLines &lyrLines = data3.getRawLyrics();

                for (int k = 0; k < CountOf(vLyrLines); k++) {
                    LyricsLine *pLine = lyrLines[k];

                    CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
                    CPPUNIT_ASSERT(k < lyrLines.size());
                    CPPUNIT_ASSERT(strcmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
                }
                // new generated lyrics may increase 1 line by adding encoding.
                // CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));
            }
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLrcParser);

#endif // _CPPUNIT_TEST
