/********************************************************************
    Created  :    2002年1月3日 17:54:54
    FileName :    MLData.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "CurrentLyrics.h"
#include "LyricsSearch.h"
#include "HelperFun.h"
#include "LrcParser.h"
#ifndef _IPHONE
#include "SncParser.h"
#include "SrtParser.h"
#endif // #ifndef _IPHONE
#include <algorithm>
#include "../MediaTags/LrcParser.h"
#include "../Skin/SkinTypes.h"


void wrapText(cstr_t szText, vector<string> &vOutput, int nLineSize) {
#ifdef UNICODE
    int nCount;
    cwstr_t szLineBeg, szPtr, szLastSpace;
    szLineBeg = szText;
    string str;
    while (*szLineBeg) {
        szPtr = szLineBeg;
        nCount = 0;
        szLastSpace = nullptr;
        while (*szPtr && *szPtr != '\n' && nCount <= nLineSize) {
            if ((*szPtr) > 255 || !(isDigit(*szPtr) || isDigit(*szPtr) || *szPtr == '_' || *szPtr == '-')) {
                szLastSpace = szPtr;
            }

            if (unsigned(*szPtr) >= 0xFF) {
                nCount += 2;
            } else {
                nCount += 1;
            }
            szPtr++;
        }
        str.clear();
        if (szLastSpace) {
            szPtr = szLastSpace;
        }
        str.append(szLineBeg, int(szPtr - szLineBeg));
        if (str.size()) {
            if (str.data()[str.size() - 1] == '\r') {
                str.resize(str.size() - 1);
            }
        }
        vOutput.push_back(str.c_str());

        if (*szPtr == '\n') {
            szPtr++;
        }
        szLineBeg = szPtr;
    }
#else
    vOutput.push_back(szText);
#endif
}

CurrentLyrics::CurrentLyrics() {
    m_md5OrgLyrContent = md5ToString("", 0);
    m_isUsedSpecifiedEncoding = false;

    m_bKaraokeStyle = false;

    m_lrcSourceType = LST_FILE;

    m_nMediaLength = 0;
    m_nTimeElapsed = 0;
}

CurrentLyrics::~CurrentLyrics() {
    close();
}

int CurrentLyrics::openLyrics(cstr_t mediaFile, int nMediaLength, cstr_t szLrcSource, bool bUseSpecifiedEncoding, CharEncodingType encodingSpecified) {

    close();

    int ret = ERR_OK;
    if (MediaTags::isEmbeddedLyricsUrl(szLrcSource)) {
        ret = MediaTags::openEmbeddedLyrics(mediaFile, szLrcSource, bUseSpecifiedEncoding, encodingSpecified, m_rawLyrics);
    } else {
        ret = openLyricsFile(szLrcSource, bUseSpecifiedEncoding, encodingSpecified);
    }
    if (ret != ERR_OK) {
        return ret;
    }

    m_lyricsLines = m_rawLyrics.toLyricsLinesOnly(nMediaLength);

    filterContents();

    m_nMediaLength = nMediaLength;

    m_lrcSourceType = lyrSrcTypeFromName(mediaFile);
    m_strSongFile = mediaFile;
    m_strLrcSource = szLrcSource;
    m_isUsedSpecifiedEncoding = bUseSpecifiedEncoding;

    resetModificationFlag();

    return ret;
}

int CurrentLyrics::newLyrics(cstr_t mediaFile, int nMediaLength) {
    close();

    m_strSongFile = mediaFile;
    m_nMediaLength = nMediaLength;

    return ERR_OK;
}

//
// 重新打开歌词
//
int CurrentLyrics::reopenWithEncoding(CharEncodingType encodingSpecified) {
    string strLrcSource = m_strLrcSource, strSongFile = m_strSongFile;

    return openLyrics(strSongFile.c_str(), m_nMediaLength, strLrcSource.c_str(), true, encodingSpecified);
}

int CurrentLyrics::openLyricsFile(cstr_t szFile, bool bUseSpecifiedEncoding, CharEncodingType encodingSpecified) {
    close();

    ILyricsParser *parser = nullptr;
    if (fileIsExtSame(szFile, ".txt")
        || fileIsExtSame(szFile, ".lrc")) {
        parser = new CLrcParser();
    }
#ifndef _IPHONE
    else if (fileIsExtSame(szFile, ".srt")) {
        parser = new CSrtParser();
    }
#endif // #ifndef _IPHONE
    else {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    m_strLrcSource = szFile;

    int ret = parser->parseFile(szFile, bUseSpecifiedEncoding, encodingSpecified, m_rawLyrics);

    delete parser;

    return ret;
}

int CurrentLyrics::save() {
    if (!hasLyricsOpened()) {
        return ERR_NOLYRICS_OPENED;
    }

    if (!isModified()) {
        return ERR_OK;
    }

    if (MediaTags::isEmbeddedLyricsUrl(m_strLrcSource.c_str())) {
        return MediaTags::saveEmbeddedLyrics(m_strSongFile.c_str(), {m_strLrcSource}, m_rawLyrics);
    } else {
        if (isContentModified()
            || fileIsExtSame(m_strLrcSource.c_str(), ".srt")) {
            string strFile = m_strLrcSource;
            bool bUseNewFileName;

            return saveAsFile(strFile.c_str(), bUseNewFileName);
        } else {
            // Only .lrc or .txt can call saveTag()
            if (saveTag()) {
                return ERR_OK;
            } else {
                return ERR_FALSE;
            }
        }
    }
}

// COMMENT:
//        将CMLData中的数据保存为文件 file，文件的类型，由 file 的扩展名决定。
int CurrentLyrics::saveAsFile(cstr_t file, bool &bUseNewFileName) {
    if (!hasLyricsOpened()) {
        return ERR_NOLYRICS_OPENED;
    }

    // 基类接口
    ILyricsParser *parser = nullptr;

    bUseNewFileName = false;

    // 根据文件的扩展名来判断文件类型，然后调用不同的歌词分析类来分析歌词。
    if (fileIsExtSame(file, ".lrc") ) {
        parser = new CLrcParser();
        if (getLyrContentType() <= LCT_LRC) {
            bUseNewFileName = true;
        }
    } else if (fileIsExtSame(file, ".txt") ) {
        parser = new CLrcParser();
        if (getLyrContentType() <= LCT_TXT) {
            bUseNewFileName = true;
        }
    }
    else if (fileIsExtSame(file, ".srt") ) {
        parser = new CSrtParser();
        if (getLyrContentType() <= LCT_TXT) {
            bUseNewFileName = true;
        }
    } else if (fileIsExtSame(file, ".snc") ) {
        parser = new CSncParser();
    }
    else {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    int ret = parser->saveAsFile(file, m_rawLyrics);
    if (ret == ERR_OK && bUseNewFileName) {
        resetModificationFlag();

        // 保存歌词文件名
        m_strLrcSource = file;
    }

    delete parser;

    return ret;
}

bool CurrentLyrics::saveTag() {
    assert(!isContentModified());
    assert(fileIsExtSame(m_strLrcSource.c_str(), ".txt")
        || fileIsExtSame(m_strLrcSource.c_str(), ".lrc"));

    LyrTagParser tagParser(m_rawLyrics.properties());
    if (!tagParser.replaceInFile(m_strLrcSource.c_str(), getLyrContentType() >= LCT_LRC)) {
        return false;
    }

    m_isUsedSpecifiedEncoding = false;
    m_lyrPropertiesOrg = m_rawLyrics.properties();

    return true;
}

void CurrentLyrics::resetModificationFlag() {
    m_lyrPropertiesOrg = m_rawLyrics.properties();
    string strContent = toString(false);
    m_md5OrgLyrContent = md5ToString(strContent.c_str(), strContent.size());

    m_isUsedSpecifiedEncoding = false;
}

void CurrentLyrics::clearLyrics() {
    m_lyricsLines.clear();
    m_rawLyrics.clear();
}

void CurrentLyrics::close() {
    m_lyrPropertiesOrg.clear();
    m_md5OrgLyrContent = md5ToString((const char *)"", 0);
    m_nTimeElapsed = 0;

    m_bKaraokeStyle = false;

    m_lrcSourceType = LST_FILE;
    m_strSongFile.resize(0);;
    m_strLrcSource.resize(0);;

    clearLyrics();
    assert(getLyrContentType() == LCT_UNKNOWN);
}

bool CurrentLyrics::hasLyricsOpened() {
    return getLyrContentType() != LCT_UNKNOWN;
}

string CurrentLyrics::getSuggestedLyricsFileName() {
    // Combine file name
    auto &lyrProps = m_rawLyrics.properties();
    auto strFileName = formatMediaTitle(lyrProps.artist.c_str(), lyrProps.title.c_str());
    strFileName = fileNameFilterInvalidChars(strFileName.c_str());

    if (strFileName.empty()) {
        strFileName = fileGetTitle(m_strSongFile.c_str());
    }

    if (lyrProps.lyrContentType == LCT_LRC) {
        strFileName += ".lrc";
    } else {
        strFileName += ".txt";
    }

    return strFileName;
}

cstr_t CurrentLyrics::getLyricsFileName() {
    return m_strLrcSource.c_str();
}

// COMMENT:
//        设置时间偏移
void CurrentLyrics::SetPlayElapsedTime(int nTimeElapsed) {
    m_nTimeElapsed = nTimeElapsed + m_rawLyrics.properties().getOffsetTime();
}

// COMMENT:
//        取得
int CurrentLyrics::getPlayElapsedTime() {
    return m_nTimeElapsed;
}

int CurrentLyrics::getCurPlayLine(const LyricsLines &lyrLines) {
    static int index = 0; // Using static variable to cache latest result.

    if (lyrLines.empty()) {
        return -1;
    }

    int lineCount = (int)lyrLines.size();
    if (index >= lineCount) {
        index = 0;
    }

    // 找到前一行
    for (; index > 0; index--) {
        auto &line = lyrLines[index];

        if (line.endTime != -1 && line.endTime < getPlayElapsedTime()) {
            index++;
            break;
        }
    }

    // 找到下一行
    for (; index < lineCount; index++) {
        auto &line = lyrLines[index];
        if (line.endTime > getPlayElapsedTime()) {
            break;
        }
    }

    if (index >= lineCount) {
        index = lineCount - 1;
    }

    assert(index != -1);
    return index;
}

bool CurrentLyrics::doesChooseNewFileName() {
    return (m_strLrcSource.empty() || getLyrContentType() != m_lyrPropertiesOrg.lyrContentType);
}

bool CurrentLyrics::isModified() {
    // No lyrics opened.
    if (!hasLyricsOpened()) {
        return false;
    }

    return !m_rawLyrics.properties().equal(m_lyrPropertiesOrg) || isContentModified();
}

bool CurrentLyrics::isContentModified() {
    string strContent = toString(false);
    string md5 = md5ToString(strContent.c_str(), strContent.size());

    return md5 != m_md5OrgLyrContent || m_isUsedSpecifiedEncoding;
}

//
// 1、过滤歌词中的一些无法显示的字符：Tab,回车符号
// ?2、增加选项是否过滤行末、行首的空格
// 3、对HTML中的一些符号进行转义
int CurrentLyrics::filterContents(void) {
    static cstr_t        szEscape[] = {
        "&lt;", "<",
        "&gt;", ">",
        "&amp;", "&",
        "&quot;", "\"",
        "&apos;", "\'",
        "&#58;", ":",
        nullptr, nullptr
    };

    //
    // 进行替换，TAB，
    // 对HTML进行转义
    for (auto &line : m_lyricsLines) {
        for (auto &piece : line.pieces) {
            char *szBeg = (char *)piece.text.data();

            int pos = 0;
            for (int m = 0; szBeg[m] != '\0'; m++) {
                if (szBeg[m] == '\t') {
                    szBeg[pos] = ' ';
                    pos++;
                } else if (szBeg[m] == '&') {
                    cstr_t *pszEsc = szEscape;
                    while (*pszEsc) {
                        int nLen = (int)strlen(*pszEsc);
                        if (strncasecmp(*pszEsc, szBeg, nLen) == 0) {
                            szBeg[pos] = (*(pszEsc + 1))[0];
                            pos++;
                            m += nLen - 1;
                            break;
                        }
                        pszEsc+=2;
                    }
                    if (*pszEsc == nullptr) {
                        // "&#58;", ":",
                        bool bNumb = false;
                        cstr_t szNumb = szBeg + m + 1;
                        if (*szNumb == '#') {
                            int nNumb = 0;
                            int x;
                            for (x = 0; x < 3 && isDigit(szNumb[x + 1]); x++) {
                                nNumb = nNumb * 10 + (szNumb[x + 1] - '0');
                            }
                            if (nNumb > 0 && nNumb <= 0xFFF && szNumb[x + 1] == ';') {
                                bNumb = true;
                                m += 2 + x;
                                szBeg[pos] = (char)nNumb;
                                pos++;
                            }
                        }

                        if (!bNumb) {
                            szBeg[pos] = '&';
                            pos++;
                        }
                    }
                } else {
                    szBeg[pos] = szBeg[m];
                    pos++;
                }
            }
            piece.text.resize(pos);
        }
    }

    return 0;
}

string CurrentLyrics::toString(bool isIncTags) {
    return toLyricsString(m_rawLyrics, isIncTags, getLyrContentType() == LCT_TXT);
}

int CurrentLyrics::fromString(cstr_t szLyrics) {
    clearLyrics();

    int ret = parseLyricsString(szLyrics, false, m_rawLyrics);
    if (ret == ERR_OK) {
        m_lyricsLines = m_rawLyrics.toLyricsLinesOnly(getMediaLength());

        filterContents();
    }

    return ret;
}

#if UNIT_TEST

#include "../TinyJS/utils/unittest.h"

static string getTestDataFolder() {
    string testDataFolder = fileGetPath(__FILE__);
    dirStringAddSep(testDataFolder);
    testDataFolder += "TestData" PATH_SEP_STR;
    return testDataFolder;
}

void removeTempMediaFile() {
    auto testDataFolder = getTestDataFolder();
    string        vstrSongFile[] = {
        testDataFolder + "lyr-usly.mp3",
        testDataFolder + "lyr(wmp-chs).mp3",
        testDataFolder + "lyr-usly(chs).mp3",
        testDataFolder + "lyrics3v2.mp3",
        testDataFolder + "lyr-chs.m4a",
    };

    for (int i = 0; i < (int)CountOf(vstrSongFile); i++) {
        string strFile = getUnittestTempDir();
        strFile += fileGetName(vstrSongFile[i].c_str());
        deleteFile(strFile.c_str());
    }
}

TEST(CurrentLyrics, OpenLrcFile) {
    auto testDataFolder = getTestDataFolder();

    cstr_t mediaFile = "artist - title.mp3";
    string        vstrLyrFile[] = {
        testDataFolder + "test(gb).lrc",
        testDataFolder + "test(ucs2).lrc",
        testDataFolder + "test(ucs2-be).lrc",
        testDataFolder + "test(utf8).lrc"
    };
    int nRet;
    CurrentLyrics data;

    for (int i = 0; i < CountOf(vstrLyrFile); i++) {
        nRet = data.openLyrics(mediaFile, 60 * 1000, vstrLyrFile[i].c_str());
        ASSERT_EQ(nRet, ERR_OK);


        {
            // Lyrics content ok?
            // [00:01.00]L1
            // [01:01.01][01:00.01]L2
            // [02:01.10]L3
            cstr_t vLyrLines[] = { "L1", "L2", "L2", "L3" };
            int vTime[] = { 1*1000, 1*60*1000+10, (1*60+1)*1000+10, (2*60+1)*1000+100};

            LyricsLines &lyrLines = data.getLyricsLines();

            for (int k = 0; k < lyrLines.size(); k++) {
                LyricsLine &line = lyrLines[k];

                ASSERT_TRUE(line.pieces.size() == 1);
                ASSERT_TRUE(k < CountOf(vLyrLines));
                ASSERT_EQ(line.pieces[0].text, vLyrLines[k]);
                ASSERT_TRUE(line.pieces[0].beginTime == vTime[k]);
            }
            ASSERT_TRUE(lyrLines.size() == CountOf(vLyrLines));
        }
    }

    removeTempMediaFile();
}

void prepareMediaFile() {
    removeTempMediaFile();

    auto testDataFolder = getTestDataFolder();
    string        vstrSongFile[] = {
        testDataFolder + "lyr-usly.mp3",
        testDataFolder + "lyr(wmp-chs).mp3",
        testDataFolder + "lyr-usly(chs).mp3",
        testDataFolder + "lyrics3v2.mp3",
        testDataFolder + "lyr-chs.m4a",
    };

    for (int i = 0; i < (int)CountOf(vstrSongFile); i++) {
        string strFile = getUnittestTempDir();
        strFile += fileGetName(vstrSongFile[i].c_str());
        ASSERT_TRUE(copyFile(vstrSongFile[i].c_str(), strFile.c_str(), false));
    }
}

TEST(CurrentLyrics, OpenSaveID3v2_USLT) {
    auto testDataFolder = getTestDataFolder();
    int nRet;

    prepareMediaFile();

    string testTmpFolder = getUnittestTempDir();

    //
    // test on embedded USLT lyrics.
    //

    string        vstrSongFile[] = {
        testTmpFolder + "lyr-usly.mp3",
        testTmpFolder + "lyr(wmp-chs).mp3",
        testTmpFolder + "lyr-usly(chs).mp3",
        testTmpFolder + "lyr(wmp-chs).mp3",
        testTmpFolder + "lyr-chs.m4a",
    };
    cstr_t        vLyrFile[] = {
        "song://id3v2/uslt/eng",
        "song://id3v2/uslt/eng",
        "song://id3v2/uslt/eng",
        "song://id3v2/uslt/chi",
        SZ_SONG_M4A_LYRICS,
    };
    bool vWriteTested[] = { false, false, false, false, false };
    assert(CountOf(vWriteTested) == CountOf(vLyrFile));
    assert(CountOf(vWriteTested) == CountOf(vstrSongFile));

    for (int i = 0; i < CountOf(vstrSongFile); i++) {
        for (int kk = 0; kk < 2; kk++) {
            printf("Round: %d, %d\n", i, kk);

            // loop twice, first for reading, second for writing.
            CurrentLyrics data;
            nRet = data.openLyrics(vstrSongFile[i].c_str(), 60 * 1000, vLyrFile[i]);
            ASSERT_EQ(nRet, ERR_OK);

            {
                //
                // open, Lyrics content ok?
                //
                VecStrings vLyrLines = { "Line1", "Line2", "Line3" };
                VecStrings vLyrLinesChs = { "第一行", "第二行", "第三行" };

                if (!data.properties().artist.empty() && !data.properties().title.empty()) {
                    vLyrLines.insert(vLyrLines.begin(), "artist - title");
                    vLyrLinesChs.insert(vLyrLinesChs.begin(), "artist - title");
                }

                LyricsLines &lyrLines = data.getLyricsLines();

                ASSERT_EQ(lyrLines.size(), vLyrLines.size());
                for (int k = 0; k < lyrLines.size(); k++) {
                    LyricsLine &line = lyrLines[k];

                    ASSERT_TRUE(line.pieces.size() == 1);
                    if (i < 2) {
                        ASSERT_EQ(line.pieces[0].text, vLyrLines[k]);
                    } else {
                        ASSERT_EQ(line.pieces[0].text, vLyrLinesChs[k]);
                    }
                }
            }

            // test on save uslt
            if (!vWriteTested[i]) {
                data.properties().artist = "artist";
                data.properties().title = "title";
                ASSERT_TRUE(data.save() == ERR_OK);
                vWriteTested[i] = true;
            } else {
                ASSERT_TRUE(strcmp(data.properties().artist.c_str(), "artist") == 0);
                ASSERT_TRUE(strcmp(data.properties().title.c_str(), "title") == 0);
            }
        }
    }

    removeTempMediaFile();
}

TEST(CurrentLyrics, OpenSaveID3v2_SYLT) {
    auto testDataFolder = getTestDataFolder();
    int nRet;
    CurrentLyrics data;

    prepareMediaFile();

    string testTmpFolder = getUnittestTempDir();

    //
    // test on embedded SYLT lyrics.
    //
    string        vstrSongFileSylt[] = {
        testTmpFolder + "lyr(wmp-chs).mp3",
        testTmpFolder + "lyr(wmp-chs).mp3",
    };
    cstr_t        vLyrFileSylt[] = {
        "song://id3v2/sylt/eng",
        "song://id3v2/sylt/chi",
    };

    bool vWriteTested[] = { false, false, false, false };

    for (int i = 0; i < CountOf(vstrSongFileSylt); i++) {
        for (int kk = 0; kk < 2; kk++) {
            printf("Round: %d, %d\n", i, kk);

            // loop twice, first for reading, second for writing.
            int nOffsetTime = 0;
            if (kk == 1) {
                nOffsetTime = 500;
            }

            nRet = data.openLyrics(vstrSongFileSylt[i].c_str(), 60 * 1000, vLyrFileSylt[i]);
            ASSERT_EQ(nRet, ERR_OK);

            {
                // Lyrics content ok?
                cstr_t vLyrLines[] = { "Line1", "Line2", "Line3" };
                cstr_t vLyrLinesChs[] = { "第一行", "第二行", "第三行" };
                int vTime[] = { 1, 1*1000+9, 1*1000+18 };

                LyricsLines &lyrLines = data.getLyricsLines();

                for (int k = 0; k < lyrLines.size(); k++) {
                    LyricsLine &line = lyrLines[k];

                    ASSERT_EQ(line.pieces.size(), 1);
                    ASSERT_TRUE(k < CountOf(vLyrLines));
                    if (i < 1) {
                        ASSERT_EQ(line.pieces[0].text, vLyrLines[k]);
                    } else {
                        ASSERT_EQ(line.pieces[0].text, vLyrLinesChs[k]);
                    }

                    ASSERT_EQ(line.pieces[0].beginTime, vTime[k] + nOffsetTime);
                }
                ASSERT_EQ(lyrLines.size(), CountOf(vLyrLines));
            }

            // test on save SYLT
            if (!vWriteTested[i]) {
                data.properties().artist = "artist";
                data.properties().title = "title";
                data.properties().setOffsetTime(-500);
                int ret = data.save();
                ASSERT_EQ(ret, ERR_OK);
                vWriteTested[i] = true;
            } else {
                ASSERT_EQ(data.properties().artist, "artist");
                ASSERT_EQ(data.properties().title, "title");
            }
        }
    }

    removeTempMediaFile();
}

TEST(CurrentLyrics, OpenSaveLyrics3v2) {
    auto testDataFolder = getTestDataFolder();
    int nRet;
    CurrentLyrics data;

    prepareMediaFile();

    string testTmpFolder = getUnittestTempDir();

    string        vstrSongFileSylt[] = {
        testTmpFolder + "lyrics3v2.mp3",
    };
    cstr_t        vLyrFileSylt[] = {
        SZ_SONG_LYRICS3V2,
    };
    bool vWriteTested[] = { false };
    CharEncodingType orgDefEncoding = getDefaultLyricsEncoding();

    // 读取 lyrics3v2.mp3 的歌词需要提供缺省编码
    setDefaultLyricsEncoding(ED_GB2312);


    for (int i = 0; i < CountOf(vstrSongFileSylt); i++) {
        for (int kk = 0; kk < 2; kk++) {
            printf("Round: %d, %d\n", i, kk);

            // loop twice, first for reading, second for writing.
            int nOffsetTime = 0;
            if (kk == 1) {
                nOffsetTime = 500;
            }

            nRet = data.openLyrics(vstrSongFileSylt[i].c_str(), 60 * 1000, vLyrFileSylt[i]);
            ASSERT_TRUE(nRet == ERR_OK);

            {
                // Lyrics content ok?
                cstr_t vLyrLines[] = { "第一行", "第二行", "第三行" };
                int vTime[] = { 0, 1*1000+10, 1*1000+20 };

                LyricsLines &lyrLines = data.getLyricsLines();

                for (int k = 0; k < lyrLines.size(); k++) {
                    LyricsLine &line = lyrLines[k];

                    ASSERT_TRUE(line.pieces.size() == 1);
                    ASSERT_TRUE(k < CountOf(vLyrLines));
                    ASSERT_EQ(line.pieces[0].text, vLyrLines[k]);
                    ASSERT_TRUE(line.pieces[0].beginTime == vTime[k]);
                }
                ASSERT_TRUE(lyrLines.size() == CountOf(vLyrLines));
            }

            // test on saving
            if (!vWriteTested[i]) {
                data.properties().artist = "artist";
                data.properties().title = "title";
                ASSERT_TRUE(data.save() == ERR_OK);
                vWriteTested[i] = true;
            } else {
                ASSERT_TRUE(strcmp(data.properties().artist.c_str(), "artist") == 0);
                ASSERT_TRUE(strcmp(data.properties().title.c_str(), "title") == 0);
            }
        }
    }

    setDefaultLyricsEncoding(orgDefEncoding);
    removeTempMediaFile();
}

#endif
