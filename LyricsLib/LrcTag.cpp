#include "LrcTag.h"
#include "LrcParserHelper.h"


CharEncodingType g_defaultLyricsEncoding = ED_SYSDEF;

void setDefaultLyricsEncoding(CharEncodingType encoding) {
    g_defaultLyricsEncoding = encoding;
}

bool isTxtTagAtTailOfFile(LyrTagType tagType) {
    return (tagType & (LTT_OFFSET | LTT_MEDIA_LENGTH | LTT_ID | LTT_ENCODING)) != 0;
}

void formatPlayTime(int nmsTime, char szBuf[]) {
    int nMinute, nSec, nHour;

    nSec = nmsTime / 1000;
    nMinute = nSec / 60;
    nSec %= 60;
    nHour = nMinute / 60;
    nMinute %= 60;

    if (nHour > 0) {
        snprintf(szBuf, 32, "%d:%02d:%02d", nHour, nMinute, nSec);
    } else {
        snprintf(szBuf, 32, "%02d:%02d", nMinute, nSec);
    }
}

bool getLRCTagValue(cstr_t szLyrics, cstr_t szTagPropName, string &strValue) {
    const string strPropName = szTagPropName;
    int nValPosBeg, nValPosEnd;

    if (!searchLrcPropTag(szLyrics, strPropName, nValPosBeg, nValPosEnd)) {
        return false;
    }

    strValue.assign(szLyrics + nValPosBeg, size_t(nValPosEnd - nValPosBeg));

    trimStr(strValue);

    return true;
}

bool getTxtTagValue(cstr_t szLyrics, cstr_t szTagPropName, string &strValue) {
    const string strPropName = szTagPropName;
    int nValPosBeg, nValPosEnd;

    if (!searchTxtPropTag(szLyrics, strPropName, nValPosBeg, nValPosEnd)) {
        return false;
    }

    strValue.assign(szLyrics + nValPosBeg, size_t(nValPosEnd - nValPosBeg));

    trimStr(strValue);

    return true;
}

CharEncodingType getLyricsTextEncoding(uint8_t *&szLrc, size_t &nLen) {
    int nBomSize = 0;
    CharEncodingType encoding = detectFileEncoding(szLrc, (int)nLen, nBomSize);

    szLrc += nBomSize;
    nLen -= nBomSize;

    if (encoding == ED_SYSDEF) {
#ifdef _WIN32
        if (strlen((const char *)szLrc) <= 3 && nLen >= 10) {
            int nFlag = IS_TEXT_UNICODE_UNICODE_MASK;
            if (IsTextUnicode((void *)szLrc, nLen, &nFlag)) {
                encoding = ED_UNICODE;
                return encoding;
            }
        }
#endif

        // ANSI text, Retrieve the encoding in lyrics.
        string strValue;
        if (getLRCTagValue((const char *)szLrc, SZ_LRC_ENCODING, strValue)) {
            encoding = getCharEncodingID(strValue.c_str());
        } else if (getTxtTagValue((const char *)szLrc, SZ_TXT_ENCODING, strValue)) {
            encoding = getCharEncodingID(strValue.c_str());
        } else {
            encoding = g_defaultLyricsEncoding;
        }

        // Discard ED_UNICODE encoding
        if (encoding == ED_UNICODE_BIG_ENDIAN || encoding == ED_UNICODE) {
            encoding = g_defaultLyricsEncoding;
        }
    }

    return encoding;
}

string convertLyricsToUtf8(uint8_t *text, size_t len, bool useSpecifiedEncoding, CharEncodingType encoding) {
    if (!useSpecifiedEncoding) {
        encoding = getLyricsTextEncoding(text, len);
    }

    //
    // Convert lyrics to the current encoding
    //
    string txtUtf8;
    if (encoding == ED_UNICODE || encoding == ED_UNICODE_BIG_ENDIAN) {
        u16string wstrBE;
        len /= sizeof(WCHAR);

        if (encoding == ED_UNICODE_BIG_ENDIAN) {
            wstrBE.reserve(len);
            wstrBE.append((WCHAR *)text, len);
            ucs2EncodingReverse((WCHAR *)wstrBE.data(), (int)wstrBE.size());
            text = (uint8_t *)wstrBE.data();
        }

        ucs2ToUtf8((WCHAR *)text, (int)len, txtUtf8);
    } else if (encoding == ED_UTF8) {
        txtUtf8.assign((const char *)text, len);
    } else {
        mbcsToUtf8((cstr_t)text, (int)len, txtUtf8, encoding);
    }

    return txtUtf8;

}

//////////////////////////////////////////////////////////////////////////

bool LyricsProperties::equal(const LyricsProperties &other) const {
    return m_strArtist == other.m_strArtist &&
        m_strTitle == other.m_strTitle &&
        m_strAlbum == other.m_strAlbum &&
        m_strBy == other.m_strBy &&
        m_strId == other.m_strId &&
        m_strTimeOffset == other.m_strTimeOffset &&
        m_strMediaLength == other.m_strMediaLength;
}

void LyricsProperties::clear() {
    m_strTitle.resize(0);
    m_strArtist.resize(0);
    m_strAlbum.resize(0);
    m_strBy.resize(0);
    m_strTimeOffset.resize(0);
    m_strMediaLength.resize(0);
    m_strId.resize(0);
    m_nTimeOffset = 0;
    m_lyrContentType = LCT_UNKNOWN;
}

void LyricsProperties::setOffsetTime(int nOffsetTime) {
    m_nTimeOffset = nOffsetTime;
    assert(m_strTimeOffset.empty() || isDigit(m_strTimeOffset.c_str()[0]) || m_strTimeOffset.c_str()[0] == '-');
    m_strTimeOffset = stringPrintf("%d", nOffsetTime).c_str();
}

void LyricsProperties::setMediaLength(int nMediaLength) {
    if (nMediaLength > 0) {
        char szBuf[64] = { 0 };
        formatPlayTime(nMediaLength * 1000, szBuf);
        m_strMediaLength = szBuf;
    }
}

int LyricsProperties::getMediaLengthInt() const {
    if (m_strMediaLength.size() < 4) {
        return 0;
    }

    //snprintf(szBuf, 32, "%d:%02d:%02d", nHour, nMinute, nSec);
    // snprintf(szBuf, 32, "%02d:%02d", nMinute, nSec);

    int n1, n2, n3;
    cstr_t p = m_strMediaLength.c_str();
    p = readInt_t(p, n1);
    if (*p != ':') {
        return 0;
    }
    p++;

    p = readInt_t(p, n2);
    if (*p == '\0') {
        return n1 * 60 +n2;
    }

    if (*p != ':') {
        return 0;
    }
    p++;
    readInt_t(p, n3);

    return (n1 * 60 + n2) * 60 + n3;
}

//////////////////////////////////////////////////////////////////////////

CLyrTagNameValueMap::CLyrTagNameValueMap(LyricsProperties &prop, uint32_t tagMask)
: m_prop(prop) {
    m_tagMask = tagMask;
    m_strOffset = prop.getOffsetTimeStr();

    // If offset is 0, don't write it.
    if (strcmp(m_strOffset.c_str(), "0") == 0) {
        m_strOffset.clear();
    }

    Item        vNames[] = {
        { SZ_LRC_AR,        SZ_TXT_AR,          LTT_ARTIST,         &(prop.m_strArtist) },
        { SZ_LRC_TI,        SZ_TXT_TI,          LTT_TITLE,          &(prop.m_strTitle) },
        { SZ_LRC_AL,        SZ_TXT_AL,          LTT_ALBUM,          &(prop.m_strAlbum) },
        { SZ_LRC_BY,        "",                 LTT_BY,             &(prop.m_strBy) },
        // { SZ_LRC_ENCODING,  SZ_TXT_ENCODING,    LTT_ENCODING,       &m_strEncoding },
        { SZ_LRC_OFFSET,    SZ_TXT_OFFSET,      LTT_OFFSET,         &m_strOffset },
        { SZ_LRC_LENGTH,    SZ_TXT_LENGTH,      LTT_MEDIA_LENGTH,   &(prop.m_strMediaLength) },
        { SZ_LRC_ID,        SZ_TXT_ID,          LTT_ID,             &(prop.m_strId) },
    };

    for (int i = 0; i < CountOf(vNames); i++) {
        if (tagMask & vNames[i].tagType) {
            m_vItems.push_back(vNames[i]);
        }
    }
}

LyrTagType CLyrTagNameValueMap::getTagType(int nIndex) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());
    return m_vItems[nIndex].tagType;
}

const string &CLyrTagNameValueMap::getLrcName(int nIndex) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());
    return m_vItems[nIndex].strLrcName;
}

const string &CLyrTagNameValueMap::getTxtName(int nIndex) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());
    return m_vItems[nIndex].strTxtName;
}

void CLyrTagNameValueMap::setValue(int nIndex, cstr_t szValue, int nLen) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());

    string *pValue = m_vItems[nIndex].pstrValue;
    if (pValue->empty()) {
        pValue->assign(szValue, nLen);
        onSetValue(nIndex);
    }
}

void CLyrTagNameValueMap::onSetValue(int nIndex) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());

    Item &item = m_vItems[nIndex];

    string *pValue = item.pstrValue;
    assert(pValue);
    trimStr(*pValue);

    if (item.tagType == LTT_OFFSET) {
        m_prop.setOffsetTime(pValue->c_str());
    }
}

const string &CLyrTagNameValueMap::getValue(int nIndex) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());

    string *pValue = m_vItems[nIndex].pstrValue;
    return *pValue;
}

void CLyrTagNameValueMap::clearValue(int nIndex) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());

    m_vItems[nIndex].pstrValue->clear();
}

string CLyrTagNameValueMap::getLrcTagString(int nIndex, bool bIncLineRet) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());

    Item &item = m_vItems[nIndex];
    if (item.tagType == LTT_ENCODING) {
        return "";
    }

    string strTag;

    strTag += "[";
    strTag += item.strLrcName;
    strTag += " ";
    strTag += *(item.pstrValue);
    if (bIncLineRet) {
        strTag += "]\r\n";
    } else {
        strTag += "]";
    }
    return strTag;
}

string CLyrTagNameValueMap::getTxtTagString(int nIndex, bool bIncLineRet) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());

    Item &item = m_vItems[nIndex];
    if (item.tagType == LTT_ENCODING) {
        return "";
    }

    string strTag;

    strTag += item.strTxtName;
    strTag += " ";
    strTag += *item.pstrValue;
    if (bIncLineRet) {
        strTag += "\r\n";
    }
    return strTag;
}

int CLyrTagNameValueMap::parseTxtTag(cstr_t szText, int nLen) {
    int nValBeg, nValEnd;

    for (int k = 0; k < getCount(); k++) {
        if (parseTxtPropTag(szText, nLen, getTxtName(k), nValBeg, nValEnd)) {
            setValue(k, szText + nValBeg, nValEnd - nValBeg);
            return k;
        }
    }

    return -1;
}

int CLyrTagNameValueMap::parseLrcTag(cstr_t szText, int nLen) {
    int nValBeg, nValEnd;

    for (int k = 0; k < getCount(); k++) {
        if (parseLrcPropTag(szText, nLen, getLrcName(k), nValBeg, nValEnd)) {
            setValue(k, szText + nValBeg, nValEnd - nValBeg);
            return k;
        }
    }

    return -1;
}

void CLyrTagNameValueMap::remove(int nIndex) {
    assert(nIndex >= 0 && nIndex < (int)m_vItems.size());

    m_vItems.erase(m_vItems.begin() + nIndex);
}

//////////////////////////////////////////////////////////////////////////

CLrcTag::CLrcTag(LyricsProperties &orgProp, LyricsProperties &newProp, uint32_t tagMask) {
    m_tagMask = 0;
    if (newProp.m_strArtist != orgProp.m_strArtist) {
        m_strArtist = newProp.m_strArtist;
        m_tagMask |= LTT_ARTIST;
    }
    if (newProp.m_strTitle != orgProp.m_strTitle) {
        m_strTitle = newProp.m_strTitle;
        m_tagMask |= LTT_TITLE;
    }
    if (newProp.m_strAlbum != orgProp.m_strAlbum) {
        m_strAlbum = newProp.m_strAlbum;
        m_tagMask |= LTT_ALBUM;
    }
    if (newProp.m_strBy != orgProp.m_strBy) {
        m_strBy = newProp.m_strBy;
        m_tagMask |= LTT_BY;
    }
    if (strcmp(newProp.getOffsetTimeStr(), orgProp.getOffsetTimeStr()) != 0) {
        m_strTimeOffset = newProp.getOffsetTimeStr();
        m_tagMask |= LTT_OFFSET;
    }
    if (newProp.m_strId != orgProp.m_strId) {
        m_strId = newProp.m_strId;
        m_tagMask |= LTT_ID;
    }
    if (newProp.m_strMediaLength != orgProp.m_strMediaLength) {
        m_strMediaLength = newProp.m_strMediaLength;
        m_tagMask |= LTT_MEDIA_LENGTH;
    }

    m_tagMask &= tagMask;
}


int CLrcTag::readFromFile(cstr_t szFile) {
    string lyrics;

    if (!isFileExist(szFile)) {
        return ERR_FILE_NOT_EXIST;
    }

    if (!readFile(szFile, lyrics)) {
        return ERR_READ_FILE;
    }

    readFromText((uint8_t *)lyrics.data(), lyrics.size(), false, ED_SYSDEF);

    return ERR_OK;
}

bool CLrcTag::readFromText(uint8_t *str, size_t nLen, bool bUseSpecifiedEncoding, CharEncodingType encoding) {
    string utf8 = convertLyricsToUtf8(str, nLen, bUseSpecifiedEncoding, encoding);

    return readFromText(utf8.c_str(), utf8.size());
}

int CLrcTag::writeToFile(cstr_t szFile, bool bLrcTag) {
    string lyrics;

    if (!isFileExist(szFile)) {
        return ERR_FILE_NOT_EXIST;
    }

    if (!readFile(szFile, lyrics)) {
        return ERR_READ_FILE;
    }

    writeToBuffer(lyrics, bLrcTag);

    if (!writeFile(szFile, lyrics.c_str(), lyrics.size())) {
        return ERR_OPEN_FILE;
    }

    return ERR_OK;
}

void CLrcTag::writeToBuffer(string &bufData, bool bLrcTag) {
    bufData = convertLyricsToUtf8((uint8_t *)bufData.c_str(), bufData.size(), false, ED_SYSDEF);

    writeToText(bufData, bLrcTag);

    if (!isAnsiStr(bufData.c_str())) {
        bufData.insert(0, SZ_FE_UTF8);
    }
}

void trimAppend(string &strTarget, const char *str, int nLen) {
    while (nLen > 0 && (str[nLen - 1] == '\r' || str[nLen - 1] == '\n' || str[nLen - 1] == ' ')) {
        nLen--;
    }

    while (nLen > 0 && (*str == '\r' || *str == '\n' || *str == ' ')) {
        str++;
        nLen--;
    }

    strTarget.append(str, nLen);
}

bool lrcTagReadFromText(const char *str, size_t nLen, CLyrTagNameValueMap &propNameValueMap, string &strLyrContent) {
    strLyrContent.clear();

    const char *szLine = str;
    bool bFoundTag = false;

    while (szLine) {
        const char * szNextLine = nextLine(szLine);
        szLine = ignoreSpaces(szLine);
        int nLineLen = (szNextLine != nullptr) ? int(szNextLine - szLine) : (int)strlen(szLine);
        LyrTagType tagType = LTT_UNKNOWN;

        if (*szLine == '[') {
            if (isDigit(szLine[1]) && propNameValueMap.shouldReadLyrContentType()) {
                if (isLrcTimeTag(szLine)) {
                    propNameValueMap.setLyrContentType(LCT_LRC);
                }
            }

            int propIndex = propNameValueMap.parseLrcTag(szLine, nLineLen);
            if (propIndex != -1) {
                bFoundTag = true;
                tagType = propNameValueMap.getTagType(propIndex);
            }
        }

        if (propNameValueMap.shouldReadMd5() && tagType != LTT_ID) {
            trimAppend(strLyrContent, szLine, nLineLen);
        }

        szLine = szNextLine;
    }

    return bFoundTag;
}

bool txtTagReadFromText(const char *str, size_t nLen, CLyrTagNameValueMap &propNameValueMap, string &strLyrContent) {
    strLyrContent.clear();

    const char *szLine = str;
    bool bFoundTag = false;

    while (szLine) {
        const char * szNextLine = nextLine(szLine);
        ignoreSpaces(szLine);
        int nLineLen = (szNextLine != nullptr) ? int(szNextLine - szLine) : (int)strlen(szLine);
        LyrTagType tagType = LTT_UNKNOWN;

        int propIndex = propNameValueMap.parseTxtTag(szLine, nLineLen);
        if (propIndex != -1) {
            bFoundTag = true;
            tagType = propNameValueMap.getTagType(propIndex);
        }

        if (propNameValueMap.shouldReadMd5() && tagType != LTT_ID) {
            trimAppend(strLyrContent, szLine, nLineLen);
        }

        szLine = szNextLine;
    }

    return bFoundTag;
}

bool CLrcTag::readFromText(const char *str, size_t nLen) {
    CLyrTagNameValueMap propNameValueMap(*this, m_tagMask);

    string strLyrContent;
    bool bFoundTag = lrcTagReadFromText(str, nLen, propNameValueMap, strLyrContent);
    if (!bFoundTag) {
        if (propNameValueMap.shouldReadLyrContentType()) {
            propNameValueMap.setLyrContentType(LCT_TXT);
        }
        bFoundTag = txtTagReadFromText(str, nLen, propNameValueMap, strLyrContent);
    }

    if (propNameValueMap.shouldReadMd5()) {
        m_md5OfLyrContent = md5ToString((cstr_t)strLyrContent.c_str(), strLyrContent.size() * sizeof(char));
    }

    return bFoundTag;
}

void lrcTagWriteToText(string &str, CLyrTagNameValueMap &propNameValueMap) {
    const char *szLine = str.c_str();
    bool bFoundTag = false;
    char SZ_SPACE[] = { ' ', 0 };

    // Find existing tag, and replace it.
    while (szLine) {
        const char * szNextLine = nextLine(szLine);
        const char *szLineBeg = szLine;

        szLine = ignoreSpaces(szLine);
        if (*szLine == '[' && !isDigit(szLine[1])) {
            int nLineLen;
            if (szNextLine) {
                nLineLen = int(szNextLine - szLine);
            } else {
                nLineLen = (int)strlen(szLine);
            }

            for (int i = 0; i < propNameValueMap.getCount(); i++) {
                int nValBeg, nValEnd;
                if (parseLrcPropTag(szLine, nLineLen,
                    propNameValueMap.getLrcName(i), nValBeg, nValEnd)) {
                    bFoundTag = true;
                    string strValue = propNameValueMap.getValue(i);

                    if (strValue.empty()) {
                        // remove the whole line
                        int nLineOffset = int(szLineBeg - str.c_str());
                        str.erase(nLineOffset, nLineLen);
                        szNextLine = str.c_str() + nLineOffset;
                    } else {
                        // Convert to current encoding string.
                        strValue.insert(0, SZ_SPACE, 1);

                        // Replace
                        int nLineOffset = int(szLine - str.c_str());
                        str.replace(nLineOffset + nValBeg, nValEnd - nValBeg, strValue.c_str(), strValue.size());

                        // set next line
                        szLine = str.c_str() + nLineOffset + nValBeg;
                        szNextLine = nextLine(szLine);
                    }
                    // remove updated.
                    propNameValueMap.remove(i);
                    break;
                }
            }
        }
        szLine = szNextLine;
    }

    // For left properties, insert at the head of file.
    string strTagsAtHead;
    for (int i = (int)propNameValueMap.getCount() - 1; i >= 0; i--) {
        if (propNameValueMap.getValue(i).empty()) {
            continue;
        }

        string strValue = propNameValueMap.getLrcTagString(i);
        strTagsAtHead.append(strValue.c_str(), strValue.size());
    }

    if (strTagsAtHead.size()) {
        str.insert(0, strTagsAtHead.c_str(), strTagsAtHead.size());
    }
}

bool isStrEndWithNewLine(const string &str) {
    return str.size() > 0
        && str[str.size() - 1] != '\r'
        && str[str.size() - 1] != '\n';
}

void txtTagWriteToText(string &str, CLyrTagNameValueMap &propNameValueMap) {
    const char *szLine = str.c_str();
    bool bFoundTag = false;
    string strValue;
    char SZ_SPACE[] = { ' ', 0 };
    char _SZ_RETURN[] = { '\r', '\n', 0 };

    // Find existing tag, and replace it.
    while (szLine) {
        const char * szNextLine = nextLine(szLine);
        const char *szLineBeg = szLine;

        ignoreSpaces(szLine);
        if (isAlpha((uint8_t)*szLine)) {
            int nLineLen;
            if (szNextLine) {
                nLineLen = int(szNextLine - szLine);
            } else {
                nLineLen = (int)strlen(szLine);
            }

            int nValBeg, nValEnd;
            int i;
            for (i = 0; i < propNameValueMap.getCount(); i++) {
                if (parseTxtPropTag(szLine, nLineLen,
                    propNameValueMap.getTxtName(i), nValBeg, nValEnd)) {
                    bFoundTag = true;
                    string strValue = propNameValueMap.getValue(i);

                    if (strValue.empty()) {
                        // remove the whole line
                        int nLineOffset = int(szLineBeg - str.c_str());
                        str.erase(nLineOffset, nLineLen);
                        szNextLine = str.c_str() + nLineOffset;
                    } else {
                        // Convert to current encoding string.
                        strValue.insert(0, SZ_SPACE, 1);

                        // Replace
                        int nLineOffset = int(szLine - str.c_str());
                        str.replace(nLineOffset + nValBeg, nValEnd - nValBeg, strValue.c_str(), strValue.size());

                        // set next line
                        szLine = str.c_str() + nLineOffset + nValBeg;
                        szNextLine = nextLine(szLine);
                    }
                    // remove updated.
                    propNameValueMap.remove(i);
                    break;
                }
            }
        }
        szLine = szNextLine;
    }

    string strTagsAtHead, strTagsAtTail;

    // For left properties, insert at the head or tail of file.
    for (int i = (int)propNameValueMap.getCount() - 1; i >= 0; i--) {
        if (propNameValueMap.getValue(i).empty()) {
            continue;
        }

        string strValue = propNameValueMap.getTxtTagString(i);
        if (isTxtTagAtTailOfFile(propNameValueMap.getTagType(i))) {
            strTagsAtTail.append(strValue);
        } else {
            strTagsAtHead.append(strValue);
        }
    }

    if (strTagsAtHead.size()) {
        str.insert(0, strTagsAtHead.c_str(), strTagsAtHead.size());
    }

    if (strTagsAtTail.size()) {
        // append return if needed.
        if (isStrEndWithNewLine(str)) {
            str.append(_SZ_RETURN);
        }
        str.append(strTagsAtTail.c_str(), strTagsAtTail.size());
    }
}

void CLrcTag::writeToText(string &str, bool bLrcTag) {
    CLyrTagNameValueMap propNameValueMap(*this, m_tagMask);

    if (bLrcTag) {
        lrcTagWriteToText(str, propNameValueMap);
    } else {
        txtTagWriteToText(str, propNameValueMap);
    }
}

string formtLrcTimeTag(int time, bool isLongTimeFmt) {
    int min, sec, ms;
    if (isLongTimeFmt) {
        time += 10 / 2;
    } else {
        time += 1000 / 2;
    }

    ms = (time / 10) % 100;
    sec = (time / 1000) % 60;
    min = time / (1000 * 60);

    if (isLongTimeFmt) {
        return stringPrintf("[%02d:%02d.%02d]", min, sec, ms).c_str();
    } else {
        return stringPrintf("[%02d:%02d]", min, sec).c_str();
    }
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(LrcTag)

class CTestCaseLrcTag : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CTestCaseLrcTag);
    CPPUNIT_TEST(testCLrcCompressor);
    CPPUNIT_TEST(testParseLrcTimeTag);
    CPPUNIT_TEST(testParseLrcPropTag);
    CPPUNIT_TEST(testParseTxtPropTag);
    CPPUNIT_TEST(testReadLine);
    CPPUNIT_TEST(testSearchLrcPropTag);
    CPPUNIT_TEST(testSearchTxtPropTag);
    CPPUNIT_TEST(testGetLRCTagValue);
    CPPUNIT_TEST(testGetLyricsTextEncoding);
    CPPUNIT_TEST(testCLyrTagNameValueMap);
    CPPUNIT_TEST(testCLrcTag_ReadFromText_Lrc);
    CPPUNIT_TEST(testCLrcTag_WriteToText_Lrc);
    CPPUNIT_TEST(testCLrcTag_WriteToFile_Lrc);
    CPPUNIT_TEST(testCLrcTag_ReadFromText_Txt);
    CPPUNIT_TEST(testCLrcTag_WriteToText_Txt);
    CPPUNIT_TEST_SUITE_END();

protected:

public:
    void setUp() {
    }
    void tearDown() {
    }

protected:
    void testCLrcCompressor() {
#ifndef _MAC_OS

        cstr_t        szLyrics[] = {
            "[aa:tag]\n [00:01]  line1   \n [00:02][00:03]  line2\n[00:04]line1",
            "[aa:tag]\r\n[00:01][00:04]line1\r\n[00:02][00:03]line2",

            "a[aa:tag]\n[00:01]line1\nc\n[00:02]line2\n[00:03]line1\n[00:04]line2",
            "[00:01][00:03]line1\r\n[00:02][00:04]line2",

            "[aa:tag]\n[00:01]line1\n[00:02][00:03]line2\n[00:04]line3",
            "[aa:tag]\r\n[00:01]line1\r\n[00:02][00:03]line2\r\n[00:04]line3",
        };

        for (int i = 0; i < CountOf(szLyrics[i]); i++) {
            CLrcCompressor<char> compressor;

            compressor.parse(szLyrics[i]);

            string str;
            if (compressor.isLrcFormat()) {
                compressor.save(str, true);
            }

            if (i % 2 == 0) {
                CPPUNIT_ASSERT(strcmp(str.c_str(), szLyrics[i + 1]) == 0);
            } else {
                CPPUNIT_ASSERT(strcmp(str.c_str(), szLyrics[i]) == 0);
            }
        }

        for (int i = 0; i < CountOf(szLyrics[i]); i += 2) {
            string buf;
            string bufCmp;
            string strAnsi;

            stringToAnsi(szLyrics[i + 1], -1, strAnsi);

            buf = strAnsi.c_str();

            //
            // test Utf8
            //
            buf.clear();
            buf.append(SZ_FE_UTF8);
            buf.append(szLyrics[i]);
            CPPUNIT_ASSERT(compressLyrics(buf));

            bufCmp.clear();
            bufCmp.append(SZ_FE_UTF8);
            bufCmp.append(szLyrics[i + 1]);

            CPPUNIT_ASSERT(buf.size() == bufCmp.size());
            CPPUNIT_ASSERT(memcmp(buf.c_str(), bufCmp.c_str(), buf.size()) == 0);

            CPPUNIT_ASSERT(buf.size() == strAnsi.size());
            CPPUNIT_ASSERT(memcmp(buf.c_str(), strAnsi.c_str(), strAnsi.size()) == 0);

            //
            // test Ucs2
            //
            wstring_t strUcs2;
            utf8ToUCS2(szLyrics[i], -1, strUcs2);
            buf.clear();
            buf.append(SZ_FE_UCS2);
            buf.append((char *)strUcs2.c_str(), strUcs2.size() * sizeof(WCHAR));
            CPPUNIT_ASSERT(compressLyrics(buf));

            utf8ToUCS2(szLyrics[i + 1], -1, strUcs2);
            bufCmp.clear();
            bufCmp.append(SZ_FE_UCS2);
            bufCmp.append((char *)strUcs2.c_str(), strUcs2.size() * sizeof(WCHAR));

            CPPUNIT_ASSERT(buf.size() == bufCmp.size());
            CPPUNIT_ASSERT(memcmp(buf.c_str(), bufCmp.c_str(), buf.size()) == 0);

            CPPUNIT_ASSERT(buf.size() == strAnsi.size());
            CPPUNIT_ASSERT(memcmp(buf.c_str(), strAnsi.c_str(), strAnsi.size()) == 0);

            //
            // test Ucs2 Big endian
            //
            utf8ToUCS2(szLyrics[i], -1, strUcs2);
            buf.clear();
            buf.append(SZ_FE_UCS2_BE);
            buf.append((char *)strUcs2.c_str(), strUcs2.size() * sizeof(WCHAR));
            uCS2LEToBE((WCHAR *)buf.data() + 1, strUcs2.size());
            CPPUNIT_ASSERT(compressLyrics(buf));

            utf8ToUCS2(szLyrics[i + 1], -1, strUcs2);
            bufCmp.clear();
            bufCmp.append(SZ_FE_UCS2_BE);
            bufCmp.append((char *)strUcs2.c_str(), strUcs2.size() * sizeof(WCHAR));
            uCS2LEToBE((WCHAR *)bufCmp.data() + 1, strUcs2.size());

            CPPUNIT_ASSERT(buf.size() == bufCmp.size());
            CPPUNIT_ASSERT(memcmp(buf.c_str(), bufCmp.c_str(), buf.size()) == 0);

            CPPUNIT_ASSERT(buf.size() == strAnsi.size());
            CPPUNIT_ASSERT(memcmp(buf.c_str(), strAnsi.c_str(), strAnsi.size()) == 0);
        }
#endif // #ifndef _MAC_OS
    }

    void testParseLrcTimeTag() {
        // These are succeeded case.
        cstr_t vTestTag[] = { "[1:30.001]", "[ 10 : 03 . 01 ] abc", "[01:30.1][abc" };
        int vTime[] = { (1 * 60 + 30 ) * 1000 + 1, (10 * 60 + 3 ) * 1000 + 1 * 10, (1 * 60 + 30 ) * 1000 + 1 * 100 };
        cstr_t vNextPosStr[] = { "", " abc", "[abc" };

        int nNextPos, nTime;
        for (int i = 0; i < CountOf(vTestTag); i++) {
            bool bRet = parseLrcTimeTag(vTestTag[i], nNextPos, nTime);
            if (!bRet) {
                CPPUNIT_FAIL_T(stringPrintf("parseLrcTimeTag: failed, case: %d, %s", i, vTestTag[i]).c_str());
            }

            if (strncmp(vTestTag[i] + nNextPos, vNextPosStr[i], strlen(vNextPosStr[i])) != 0) {
                CPPUNIT_FAIL_T(stringPrintf("parseLrcTimeTag: Wrong nextPos: %d, case: %d, %s", nNextPos, i, vTestTag[i]).c_str());
            }

            if (nTime != vTime[i]) {
                CPPUNIT_FAIL_T(stringPrintf("parseLrcTimeTag: Wrong Time: %d, case: %d, %s", nNextPos, i, vTestTag[i]).c_str());
            }
        }

        // Failed case
        cstr_t vTestTagFailed[] = { "[1:30.001 abc", "[ 10a : 03 . 01 ]", "[01:30.1x]" };
        for (int i = 0; i < CountOf(vTestTagFailed); i++) {
            bool bRet = parseLrcTimeTag(vTestTagFailed[i], nNextPos, nTime);
            if (bRet) {
                CPPUNIT_FAIL_T(stringPrintf("parseLrcTimeTag(Failure test): failed, case: %d, %s", i, vTestTagFailed[i]).c_str());
            }
        }
    }

    void testParseLrcPropTag() {
        // These are succeeded case.
        cstr_t vTestTag[] = { "[ar:value", "[ar:value]", "[ar:]", "[ ar:  value]aaaa", "[ ar:v[xxx]alue]" };
        cstr_t vValue[] = { "value", "value", "", "  value", "v[xxx]alue" };

        string strPropName = SZ_LRC_AR;
        int nValBegPos, nValEndPos;
        for (int i = 0; i < CountOf(vTestTag); i++) {
            bool bRet = parseLrcPropTag(vTestTag[i], strlen(vTestTag[i]), strPropName, nValBegPos, nValEndPos);
            if (!bRet) {
                CPPUNIT_FAIL_T(stringPrintf("parseLrcPropTag: failed, case: %d, %s", i, vTestTag[i]).c_str());
            }

            if (strncmp(vValue[i], vTestTag[i] + nValBegPos, int(nValEndPos - nValBegPos)) != 0
                || strlen(vValue[i]) != int(nValEndPos - nValBegPos)) {
                CPPUNIT_FAIL_T(stringPrintf("parseLrcPropTag: Wrong Pos: %d, %d, case: %d, %s", nValBegPos, nValEndPos, i, vTestTag[i]).c_str());
            }
        }

        // Failed case
        cstr_t vTestTagFailed[] = { "[a r:value]aaaa", "[a:rxxx]", "[ar :rxxx]" };
        for (int i = 0; i < CountOf(vTestTagFailed); i++) {
            bool bRet = parseLrcPropTag(vTestTagFailed[i], strlen(vTestTagFailed[i]), strPropName, nValBegPos, nValEndPos);
            if (bRet) {
                CPPUNIT_FAIL_T(stringPrintf("parseLrcPropTag(Failure test): failed, case: %d, %s", i, vTestTagFailed[i]).c_str());
            }
        }
    }

    void testParseTxtPropTag() {
        // These are succeeded case.
        cstr_t vTestTag[] = { "Artist: value", "artist:value", "Artist:" };
        cstr_t vValue[] = { " value", "value", "" };

        string strPropName = SZ_TXT_AR;
        int nValBegPos, nValEndPos;
        for (int i = 0; i < CountOf(vTestTag); i++) {
            bool bRet = parseTxtPropTag(vTestTag[i], strlen(vTestTag[i]), strPropName, nValBegPos, nValEndPos);
            if (!bRet) {
                CPPUNIT_FAIL_T(stringPrintf("parseTxtPropTag: failed, case: %d, %s", i, vTestTag[i]).c_str());
            }

            if (strncmp(vValue[i], vTestTag[i] + nValBegPos, int(nValEndPos - nValBegPos)) != 0) {
                CPPUNIT_FAIL_T(stringPrintf("parseTxtPropTag: Wrong Pos: %d, %d, case: %d, %s", nValBegPos, nValEndPos, i, vTestTag[i]).c_str());
            }
        }

        // Failed case
        cstr_t vTestTagFailed[] = { "Artist :value", "Artist", "A rtist:" };
        for (int i = 0; i < CountOf(vTestTagFailed); i++) {
            bool bRet = parseTxtPropTag(vTestTagFailed[i], strlen(vTestTagFailed[i]), strPropName, nValBegPos, nValEndPos);
            if (bRet) {
                CPPUNIT_FAIL_T(stringPrintf("parseTxtPropTag(Failure test): failed, case: %d, %s", i, vTestTagFailed[i]).c_str());
            }
        }
    }

    void testReadLine() {
#define SZ_TEXT_TEST        "L1\r\nL2 \n L3 \nL4\nL5\n\n"

        cstr_t vValue[] = { "L1", "L2 ", " L3 ", "L4", "L5", "" };
        cstr_t szLine = SZ_TEXT_TEST;

        string strLine;
        int i = 0;
        while ((szLine = readLine(szLine, strLine)) != nullptr) {
            if (strcmp(strLine.c_str(), vValue[i]) != 0) {
                CPPUNIT_FAIL_T(stringPrintf("readLine: failed, case: %d, %s, value: %s", i, szLine, strLine.c_str()).c_str());
            }
            i++;
        }
        if (i != CountOf(vValue)) {
            CPPUNIT_FAIL_T(stringPrintf("readLine: Readed line Count NOT match: %d, %d", i, CountOf(vValue)).c_str());
        }

        // test for nextLine
        szLine = SZ_TEXT_TEST;
        i = 0;
        while (szLine && *szLine) {
            if (strncmp(szLine, vValue[i], strlen(vValue[i])) != 0) {
                CPPUNIT_FAIL_T(stringPrintf("nextLine: failed, case: %d, %s, value: %s", i, szLine, vValue[i]).c_str());
            }
            szLine = nextLine(szLine);
            i++;
        }

        if (i != CountOf(vValue)) {
            CPPUNIT_FAIL_T(stringPrintf("nextLine: Readed line Count NOT match: %d, %d", i, CountOf(vValue)).c_str());
        }
    }

    void testSearchLrcPropTag() {
        // These are succeeded cases.
        cstr_t vTestTag[] = { "[ ti:value]\n[ar:val2]\n[ti:title]", "   [TI:val ue]", "[xx]\n  [Ti:value]\n[ar:val]" };
        cstr_t vValue[] = { "value", "val ue", "value" };

        string strPropName = SZ_LRC_TI;
        int nValBegPos, nValEndPos;
        for (int i = 0; i < CountOf(vTestTag); i++) {
            bool bRet = searchLrcPropTag(vTestTag[i], strPropName, nValBegPos, nValEndPos);
            if (!bRet) {
                CPPUNIT_FAIL_T(stringPrintf("searchLrcPropTag: failed, case: %d, %s", i, vTestTag[i]).c_str());
            }

            if (strncmp(vValue[i], vTestTag[i] + nValBegPos, int(nValEndPos - nValBegPos)) != 0) {
                CPPUNIT_FAIL_T(stringPrintf("searchLrcPropTag: Wrong Pos: %d, %d, case: %d, %s", nValBegPos, nValEndPos, i, vTestTag[i]).c_str());
            }
        }

        // Failed case
        cstr_t vTestTagFailed[] = { "[t i:value]\n[ar:val2]", "  [ti val ue]" };
        for (int i = 0; i < CountOf(vTestTagFailed); i++) {
            bool bRet = searchLrcPropTag(vTestTagFailed[i], strPropName, nValBegPos, nValEndPos);
            if (bRet) {
                CPPUNIT_FAIL_T(stringPrintf("parseTxtPropTag(Failure test): failed, case: %d, %s", i, vTestTagFailed[i]).c_str());
            }
        }
    }

    void testSearchTxtPropTag() {
        // These are succeeded cases. encoding: iso-8859-15\r\nArtist: Alexandra Burke\r\nTitle: Candyman
        cstr_t vTestTag[] = { "Title:value\nArtist:val2\nTitle:title", "   title:val ue", "[xx]\n  Title:value\nArtist:val" };
        cstr_t vValue[] = { "value", "val ue", "value" };

        string strPropName = SZ_TXT_TI;
        int nValBegPos, nValEndPos;
        for (int i = 0; i < CountOf(vTestTag); i++) {
            bool bRet = searchTxtPropTag(vTestTag[i], strPropName, nValBegPos, nValEndPos);
            if (!bRet) {
                CPPUNIT_FAIL_T(stringPrintf("searchTxtPropTag: failed, case: %d, %s", i, vTestTag[i]).c_str());
            }

            if (strncmp(vValue[i], vTestTag[i] + nValBegPos, int(nValEndPos - nValBegPos)) != 0) {
                CPPUNIT_FAIL_T(stringPrintf("searchTxtPropTag: Wrong Pos: %d, %d, case: %d, %s", nValBegPos, nValEndPos, i, vTestTag[i]).c_str());
            }
        }

        // Failed case
        cstr_t vTestTagFailed[] = { "Ti tle:value\nAr:val2", "Title val ue" };
        for (int i = 0; i < CountOf(vTestTagFailed); i++) {
            bool bRet = searchTxtPropTag(vTestTagFailed[i], strPropName, nValBegPos, nValEndPos);
            if (bRet) {
                CPPUNIT_FAIL_T(stringPrintf("searchTxtPropTag(Failure test): failed, case: %d, %s", i, vTestTagFailed[i]).c_str());
            }
        }
    }

    void testGetLRCTagValue() {
        // These are succeeded cases.
        cstr_t vTestTag[] = { "[ ti:value]\n[ar:val2]\n[ti:title]", "   [ti:val ue]\n[ti:title]", "[xx]\n  [ti:value]\n[ar:val]" };
        cstr_t vValue[] = { "value", "val ue", "value" };

        string strValue;
        for (int i = 0; i < CountOf(vTestTag); i++) {
            bool bRet = getLRCTagValue(vTestTag[i], SZ_LRC_TI, strValue, ED_SYSDEF);
            if (!bRet) {
                CPPUNIT_FAIL_T(stringPrintf("getLRCTagValue: failed, case: %d, %s", i, vTestTag[i]).c_str());
            }

            if (strcmp(vValue[i], strValue.c_str()) != 0) {
                CPPUNIT_FAIL_T(stringPrintf("getLRCTagValue: Wrong Value: %s, case: %d, %s", strValue.c_str(), i, vTestTag[i]).c_str());
            }
        }

        // Failed case
        cstr_t vTestTagFailed[] = {"[t i:value]\n[ar:val2]", "  [ti val ue]" };
        for (int i = 0; i < CountOf(vTestTagFailed); i++) {
            bool bRet = getLRCTagValue(vTestTagFailed[i], SZ_LRC_TI, strValue, ED_SYSDEF);
            if (bRet) {
                CPPUNIT_FAIL_T(stringPrintf("getLRCTagValue(Failure test): failed, case: %d, %s", i, vTestTagFailed[i]).c_str());
            }
        }
    }

    void testGetLyricsTextEncoding() {
        // These are succeeded cases.
        {
            // ANSI file testing...
            cstr_t        vTestTag[] = { "[ encoding:gb2312]\n[ar:val2]\n[ti:title]", "   [encoding:big5]", "[xx]\n  [encoding:utf-8]\n[ar:val]\n[ti:title]",
                " encoding:gb2312\nArtist:val2\n[", "   encoding:big5", "[xx]\n  encoding:utf-8\n",
                SZ_FE_UCS2 "abc", SZ_FE_UCS2_BE "abc", SZ_FE_UTF8 "abc", ""
                };
                CharEncodingType    vEncoding[] = { ED_GB2312, ED_BIG5, ED_UTF8,
                ED_GB2312, ED_BIG5, ED_UTF8,
                ED_UNICODE, ED_UNICODE_BIG_ENDIAN, ED_UTF8, ED_SYSDEF
                };

                string        strValue;
                for (int i = 0; i < CountOf(vTestTag); i++) {
                uint8_t *p = uint8_t *vTestTag[i];
                size_t nLen = strlen(vTestTag[i]);
                CharEncodingType encoding = getLyricsTextEncoding(p, nLen);
                if (encoding != vEncoding[i]) {
                    CPPUNIT_FAIL_T(stringPrintf("getLyricsTextEncoding(ANSI): failed, case: %d, %s", i, vTestTag[i]).c_str());
                }
            }
        }
    }

    void testCLyrTagNameValueMap() {
        LyricsProperties prop;

        prop.m_strArtist = "Artist";
        prop.m_strTitle = "Title";
        prop.m_strAlbum = "Album";
        prop.m_strBy = "By";
        prop.m_strId = "Id";
        // prop.setEncoding(ED_GB2312);
        prop.setOffsetTime(1000);

        prop.m_strMediaLength = "01:32";
        CPPUNIT_ASSERT(prop.getMediaLengthInt() == 60 + 32);

        prop.m_strMediaLength = "2:11:32";
        CPPUNIT_ASSERT(prop.getMediaLengthInt() == (2 * 60 + 11) * 60 + 32);

        string strEncoding = prop.getEncoding(), strOffset = prop.getOffsetTimeStr();
        CLyrTagNameValueMap nameValueMap(prop);

        CLyrTagNameValueMap::Item        vNames[] = {
            { SZ_LRC_AR,        SZ_TXT_AR,        LTT_ARTIST,        &(prop.m_strArtist) },
            { SZ_LRC_TI,        SZ_TXT_TI,        LTT_TITLE,        &(prop.m_strTitle) },
            { SZ_LRC_AL,        SZ_TXT_AL,        LTT_ALBUM,        &(prop.m_strAlbum) },
            { SZ_LRC_BY,        "",                LTT_BY,            &(prop.m_strBy) },
            { SZ_LRC_ENCODING,    SZ_TXT_ENCODING,LTT_ENCODING,    &strEncoding },
            { SZ_LRC_OFFSET,    SZ_TXT_OFFSET,    LTT_OFFSET,        &strOffset },
            { SZ_LRC_LENGTH,    SZ_TXT_LENGTH,    LTT_MEDIA_LENGTH,&(prop.m_strMediaLength) },
            { SZ_LRC_ID,        SZ_TXT_ID,        LTT_ID,            &(prop.m_strId) },
        };

        assert(nameValueMap.getTagType(4) == LTT_ENCODING);
        nameValueMap.setValue(4, "big5", -1);
        strEncoding = "big5";

        for (int k = 0; k < nameValueMap.getCount(); k++) {
            LyrTagType tagType = nameValueMap.getTagType(k);
            bool bFound = false;
            for (int i = 0; i < CountOf(vNames); i++) {
                if (tagType != vNames[i].tagType) {
                    continue;
                }

                // make sure
                if (strcmp(vNames[i].strLrcName.c_str(), nameValueMap.getLrcName(k).c_str()) != 0) {
                    CPPUNIT_FAIL_T(stringPrintf("CLyrTagNameValueMap: case: %d, %d", k, i).c_str());
                }

                if (strcmp(vNames[i].strTxtName.c_str(), nameValueMap.getTxtName(k).c_str()) != 0) {
                    CPPUNIT_FAIL_T(stringPrintf("CLyrTagNameValueMap: case: %d, %d", k, i).c_str());
                }

                if (strcmp(vNames[i].pstrValue->c_str(), nameValueMap.getValue(k).c_str()) != 0) {
                    CPPUNIT_FAIL_T(stringPrintf("CLyrTagNameValueMap: case: %d, %d", k, i).c_str());
                }

                string strLrcTag = nameValueMap.getLrcTagString(i, false);
                auto strLrcTag2 = stringPrintf("[%s %s]", vNames[i].strLrcName.c_str(), vNames[i].pstrValue->c_str());
                if (strcmp(strLrcTag.c_str(), strLrcTag2.c_str()) != 0) {
                    CPPUNIT_FAIL_T(stringPrintf("CLyrTagNameValueMap: case: %d, %d", k, i).c_str());
                }

                string strTxtTag = nameValueMap.getTxtTagString(i, false);
                auto strTxtTag2 = stringPrintf("%s %s", vNames[i].strTxtName.c_str(), vNames[i].pstrValue->c_str());
                if (strcmp(strTxtTag.c_str(), strTxtTag2.c_str()) != 0) {
                    CPPUNIT_FAIL_T(stringPrintf("CLyrTagNameValueMap: case: %d, %d", k, i).c_str());
                }

                bFound = true;
                break;
            }
            if (!bFound) {
                // NOT FOUND.
                CPPUNIT_FAIL_T("CLyrTagNameValueMap: Not found");
            }
        }

        nameValueMap.remove(0);
        nameValueMap.remove(nameValueMap.getCount() - 1);
    }

    void testCLrcTag_ReadFromText_Lrc() {
        // test cases for CLrcTag.

        // Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS           "[ti:title]\r\n\r\n[00:01.20]\r\n[encoding:gb2312]sxx\r\n[ar:artist]"

        cstr_t valArtist = "artist";
        cstr_t valTitle = "title";
        string wStr;
        mbcsToUCS2(SZ_LYRICS, -1, wStr);

        string wLyrics;
        wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
        wLyrics.append(wStr.c_str(), wStr.size());

        string wLyricsBe;
        wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
        uCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

        string utf8Lyrics;
        utf8Lyrics.append(SZ_FE_UTF8);
        utf8Lyrics.append(SZ_LYRICS);

        uint8_t *vLyrics[] = { uint8_t *SZ_LYRICS, uint8_t *wLyrics.c_str(), uint8_t *wLyricsBe.c_str(), uint8_t *utf8Lyrics.c_str(), };
        int vLen[] = { sizeof(SZ_LYRICS), wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR), utf8Lyrics.size(), };
        CharEncodingType vEncoding[] = { ED_GB2312, ED_UNICODE, ED_UNICODE_BIG_ENDIAN, ED_UTF8, };

        for (int i = 0; i < CountOf(vLyrics); i++) {
            CLrcTag tag(LTT_ALL);
            CPPUNIT_ASSERT(tag.readFromText(vLyrics[i], vLen[i]));
            {
                CPPUNIT_ASSERT(strcmp(tag.m_strArtist.c_str(), valArtist) == 0);
                CPPUNIT_ASSERT(strcmp(tag.m_strTitle.c_str(), valTitle) == 0);
                CPPUNIT_ASSERT(tag.getEncodingIndex() == vEncoding[i]);
                CPPUNIT_ASSERT(tag.m_lyrContentType == LCT_LRC);
            }
        }

    }

    void testCLrcTag_WriteToText_Lrc() {
        // test cases for CLrcTag.

        // Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS           "[ti: title]\r\n\r\n[00:01.20]\r\n[encoding: gb2312]sxx\r\n[ar: artist]"

        string wStr;
        mbcsToUCS2(SZ_LYRICS, -1, wStr);

        string ansiLyrics;
        ansiLyrics.append(SZ_LYRICS);

        string wLyrics;
        wLyrics.append(wStr.c_str(), wStr.size());

        LyricsProperties orgProp, newProp;

        orgProp.m_strArtist = "artist";
        orgProp.m_strTitle = "title";
        orgProp.setEncoding(ED_GB2312);

        newProp.m_strArtist = "newAr";
        newProp.m_strTitle = "newTi";
        newProp.m_strId = "newID";
        newProp.m_strMediaLength = "newMeidaLen";
        newProp.setEncoding(ED_BIG5);

        //
        // test on ANSI lyrics
        //
        {
            // write new
            CLrcTag tag(orgProp, newProp);
            tag.writeToText(ansiLyrics, true);

            // read new
            CLrcTag tagReader;
            CPPUNIT_ASSERT(tagReader.readFromText(uint8_t *ansiLyrics.c_str(), ansiLyrics.size()));

            // Is same?
            CLrcTag tagCmp(tagReader, newProp);
            CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
            CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);
            CPPUNIT_ASSERT(!tagCmp.isTagChanged());

            // write old,
            CLrcTag tag2(newProp, orgProp);
            tag2.writeToText(ansiLyrics, true);

            // Is same with original?
            CPPUNIT_ASSERT(strcmp(SZ_LYRICS, ansiLyrics.c_str()) == 0);
        }

        //
        // test on UCS2 lyrics
        //
        {
            // write new
            CLrcTag tag(orgProp, newProp);
            tag.writeToText(wLyrics, true);

            // read new
            CLrcTag tagReader;
            CPPUNIT_ASSERT(tagReader.readFromText(wLyrics.c_str(), wLyrics.size()));

            // Is same?
            CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
            CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);

            // write old,
            CLrcTag tag2(newProp, orgProp);
            tag2.writeToText(wLyrics, true);

            // Is same with org?
            CPPUNIT_ASSERT(wcscmp(wStr.c_str(), wLyrics.c_str()) == 0);
        }
    }

    void testCLrcTag_WriteToFile_Lrc() {
        // test cases for CLrcTag.

        // Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS           "[ti: title]\r\n\r\n[00:01.20]\r\n[encoding: gb2312]sxx\r\n[ar: artist]"

        string wStr;
        mbcsToUCS2(SZ_LYRICS, -1, wStr);

        string ansiLyrics;
        ansiLyrics.append(SZ_LYRICS);

        string wLyrics;
        wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
        wLyrics.append(wStr.c_str(), wStr.size());

        string wLyricsBe;
        wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
        uCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

        string utf8Lyrics;
        utf8Lyrics.append(SZ_FE_UTF8);
        utf8Lyrics.append(SZ_LYRICS);

        LyricsProperties orgProp, newProp;

        orgProp.m_strArtist = "artist";
        orgProp.m_strTitle = "title";

        newProp.m_strArtist = "newAr";
        newProp.m_strTitle = "newTi";

        uint8_t *vLyrics[] = { uint8_t *SZ_LYRICS, uint8_t *wLyrics.c_str(), uint8_t *wLyricsBe.c_str(), uint8_t *utf8Lyrics.c_str(), };
        int vLen[] = { sizeof(SZ_LYRICS), wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR), utf8Lyrics.size(), };

        string strFile = getAppDataDir();
        strFile += "unit_test.lrc";
        for (int i = 0; i < CountOf(vLen); i++) {
            CPPUNIT_ASSERT(writeFile(strFile.c_str(), vLyrics[i], vLen[i]));

            CLrcTag tag(orgProp, newProp);
            CPPUNIT_ASSERT(tag.writeToFile(strFile.c_str(), true) == ERR_OK);

            CLrcTag tagReader;
            CPPUNIT_ASSERT(tagReader.readFromFile(strFile.c_str()) == ERR_OK);

            CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
            CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);

            CLrcTag tag2(newProp, orgProp);
            CPPUNIT_ASSERT(tag2.writeToFile(strFile.c_str(), true) == ERR_OK);

            string newContent;
            CPPUNIT_ASSERT(readFile(strFile.c_str(), newContent));
            CPPUNIT_ASSERT(memcmp(newContent.c_str(), vLyrics[i], vLen[i]) == 0);
            CPPUNIT_ASSERT(newContent.size() == vLen[i]);
        }

        deleteFile(strFile.c_str());
    }


    void testCLrcTag_ReadFromText_Txt() {
        // test cases for CLrcTag.

        // Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS           "Title:title\r\n\r\nabc\r\nEncoding:gb2312\r\nArtist:artist"

        cstr_t valArtist = "artist";
        cstr_t valTitle = "title";
        string wStr;
        mbcsToUCS2(SZ_LYRICS, -1, wStr);

        string wLyrics;
        wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
        wLyrics.append(wStr.c_str(), wStr.size());

        string wLyricsBe;
        wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
        uCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

        string utf8Lyrics;
        utf8Lyrics.append(SZ_FE_UTF8);
        utf8Lyrics.append(SZ_LYRICS);

        uint8_t *vLyrics[] = { uint8_t *SZ_LYRICS, uint8_t *wLyrics.c_str(), uint8_t *wLyricsBe.c_str(), uint8_t *utf8Lyrics.c_str(), };
        int vLen[] = { sizeof(SZ_LYRICS), wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR), utf8Lyrics.size(), };
        CharEncodingType vEncoding[] = { ED_GB2312, ED_UNICODE, ED_UNICODE_BIG_ENDIAN, ED_UTF8, };

        for (int i = 0; i < CountOf(vLyrics); i++) {
            CLrcTag tag(LTT_ALL);
            CPPUNIT_ASSERT(tag.readFromText(vLyrics[i], vLen[i]));
            {
                CPPUNIT_ASSERT(strcmp(tag.m_strArtist.c_str(), valArtist) == 0);
                CPPUNIT_ASSERT(strcmp(tag.m_strTitle.c_str(), valTitle) == 0);
                CPPUNIT_ASSERT(tag.getEncodingIndex() == vEncoding[i]);
                CPPUNIT_ASSERT(tag.m_lyrContentType == LCT_TXT);
            }
        }

    }

    void testCLrcTag_WriteToText_Txt() {
        // test cases for CLrcTag.

        // Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS           "Title: title\r\n\r\nabc\r\nEncoding: gb2312\r\nArtist: artist"

        string wStr;
        mbcsToUCS2(SZ_LYRICS, -1, wStr);

        string ansiLyrics;
        ansiLyrics.append(SZ_LYRICS);

        string wLyrics;
        wLyrics.append(wStr.c_str(), wStr.size());

        LyricsProperties orgProp, newProp;


        orgProp.m_strArtist = "artist";
        orgProp.m_strTitle = "title";
        orgProp.setEncoding(ED_GB2312);

        newProp.m_strArtist = "newAr";
        newProp.m_strTitle = "newTi";
        newProp.m_strId = "newID";
        newProp.m_strMediaLength = "newMeidaLen";
        newProp.setEncoding(ED_BIG5);

        //
        // test on ANSI lyrics
        //
        {
            // write new
            CLrcTag tag(orgProp, newProp);
            tag.writeToText(ansiLyrics, false);

            // read new
            CLrcTag tagReader;
            CPPUNIT_ASSERT(tagReader.readFromText(ansiLyrics.c_str(), ansiLyrics.size()));

            // Is same?
            CLrcTag tagCmp(tagReader, newProp);
            CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
            CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);
            CPPUNIT_ASSERT(!tagCmp.isTagChanged());

            // write old,
            CLrcTag tag2(newProp, orgProp);
            tag2.writeToText(ansiLyrics, false);

            // Is same with org?
            // CLrcTag may append \r\n at the end of lyrics, don't compare it
            CPPUNIT_ASSERT(strncmp(SZ_LYRICS, ansiLyrics.c_str(), strlen(SZ_LYRICS)) == 0);
            CPPUNIT_ASSERT(ansiLyrics.size() <= strlen(SZ_LYRICS) + 2);
        }

        //
        // test on UCS2 lyrics
        //
        {
            // write new
            CLrcTag tag(orgProp, newProp);
            tag.writeToText(wLyrics, false);

            // read new
            CLrcTag tagReader;
            CPPUNIT_ASSERT(tagReader.readFromText(wLyrics.c_str(), wLyrics.size()));

            // Is same?
            CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
            CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);

            // write old,
            CLrcTag tag2(newProp, orgProp);
            tag2.writeToText(wLyrics, false);

            // Is same with org?
            // CLrcTag may append \r\n at the end of lyrics, don't compare it
            CPPUNIT_ASSERT(wcsncmp(wStr.c_str(), wLyrics.c_str(), wStr.size()) == 0);
            CPPUNIT_ASSERT(wLyrics.size() <= wStr.size() + 2);
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLrcTag);

#endif // _CPPUNIT_TEST
