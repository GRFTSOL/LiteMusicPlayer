/********************************************************************
    Created  :    2002年1月3日 17:54:54
    FileName :    MLData.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "MLLib.h"
#include "MLData.h"
#include "LyricsSearch.h"
#include "HelperFun.h"
#include "LrcParser.h"
#ifndef _IPHONE
#include "SncParser.h"
#include "SrtParser.h"
#endif // #ifndef _IPHONE
#include <algorithm>
#include "LrcTag.h"
#include "../MediaTags/MidiTag.h"
#include "../Skin/SkinTypes.h"


#define ROW_TIME_MIN_SPAN            300    // 300 MS

// 歌曲开始和结束的那一段空闲时间（没有歌词显示）
#define    LYRICS_BEGIN_TIME            15000    // 15 s
#define    LYRICS_END_TIME                8000    // 10 s


MLFileType lyricsConentTypeToFileType(LYRICS_CONTENT_TYPE lyrContentType) {
    if (lyrContentType == LCT_LRC) {
        return FT_LYRICS_LRC;
    } else if (lyrContentType == LCT_TXT) {
        return FT_LYRICS_TXT;
    } else {
        return FT_UNKNOWN;
    }
}

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



#ifdef _ID3V2_SUPPORT
CMLData::ID3v2LyricsSaveCacheInfo::ID3v2LyricsSaveCacheInfo() {
    clear();
}

void CMLData::ID3v2LyricsSaveCacheInfo::clear() {
    m_strLanguage = "eng";
    m_strLrcDescription = "Lyrics were Saved by MiniLyrics.";
    m_bAllSyllableIsNewLine = true;
}
#endif // #ifdef _ID3V2_SUPPORT

CMLData::CMLData() : m_arrLyricsLines(false) {
    m_md5OrgLyrContent = md5ToString("", 0);
    m_isUsedSpecifiedEncoding = false;

    m_bKaraokeStyle = false;

    m_lrcSourceType = LST_FILE;

    m_nMediaLength = 0;
    m_nTimeElapsed = 0;
    m_nGlobalOffsetTime = 0;
}

CMLData::~CMLData() {
    close();
}

int CMLData::openLyrics(cstr_t szSongFile, int nMediaLength, cstr_t szLrcSource, bool bUseSpecifiedEncoding, CharEncodingType encodingSpecified) {
    int nRet = ERR_OK;
    LRC_SOURCE_TYPE lrcSourceType;

    close();

    lrcSourceType = lyrSrcTypeFromName(szLrcSource);
    m_isUsedSpecifiedEncoding = bUseSpecifiedEncoding;

    if (lrcSourceType == LST_LYRICS3V2) {
        string strLyrics;
        CLyrics3v2 lyrics3v2;

        nRet = lyrics3v2.readLyrcsInfo(szSongFile, strLyrics);
        if (nRet == ERR_OK) {
            nRet = openLyricsInSongOfLyrics3v2(strLyrics.c_str(), strLyrics.size(), bUseSpecifiedEncoding, encodingSpecified);
        }
    } else if (lrcSourceType == LST_ID3V2_SYLT
        || lrcSourceType == LST_ID3V2_USLT
        || lrcSourceType == LST_ID3V2_LYRICS) {
        // open ID3v2 lyrics
        CID3v2IF id3v2(encodingSpecified);

        nRet = id3v2.open(szSongFile, false, false);
        if (nRet != ERR_OK) {
            return nRet;
        }

        if (lrcSourceType == LST_ID3V2_USLT) {
            ID3v2UnsynchLyrics lyrics;
            nRet = id3v2.getUnsyncLyrics(lyrics, szLrcSource);
            if (nRet != ERR_OK) {
                return nRet;
            }
            nRet = openLyricsInSongOfID3v2USLT(lyrics, bUseSpecifiedEncoding, encodingSpecified);
        } else if (lrcSourceType == LST_ID3V2_SYLT) {
            ID3v2SynchLyrics lyrics;
            nRet = id3v2.getSyncLyrics(lyrics, szLrcSource);
            if (nRet != ERR_OK) {
                return nRet;
            }
            nRet = openLyricsInSongOfID3v2SYLT(lyrics, bUseSpecifiedEncoding, encodingSpecified);
        } else if (lrcSourceType == LST_ID3V2_LYRICS) {
            ID3v2TextUserDefined lyrics;
            nRet = id3v2.getUserDefLyrics(lyrics);
            if (nRet != ERR_OK) {
                return nRet;
            }

            if (lyrics.m_strValue.empty()) {
                setCustomErrorDesc(_TLM("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
                nRet = ERR_CUSTOM_ERROR;
            } else {
                nRet = fromString(lyrics.m_strValue.c_str());
            }
        } else {
            assert(0);
        }
    } else if (lrcSourceType == LST_M4A_LYRICS) {
        CM4aTag tag;
        nRet = tag.open(szSongFile, false);
        if (nRet != ERR_OK) {
            return nRet;
        }

        string strLyrics;
        nRet = tag.getLyrics(strLyrics);
        if (nRet != ERR_OK) {
            return nRet;
        }
        if (strLyrics.empty()) {
            setCustomErrorDesc(_TLM("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
            nRet = ERR_CUSTOM_ERROR;
        } else {
            nRet = fromString(strLyrics.c_str());
        }
    } else if (lrcSourceType == LST_KAR_LYRICS) {
        CMidiTag tag;
        nRet = tag.open(szSongFile);
        if (nRet != ERR_OK) {
            return nRet;
        }

        nRet = tag.getLyrics(m_arrFileLines);
        if (nRet != ERR_OK) {
            return nRet;
        }
        m_arrLyricsLines.insert(m_arrLyricsLines.begin(), m_arrFileLines.begin(), m_arrFileLines.end());
        setKaraokeStyle(true);
        setLyrContentType(LCT_KARAOKE);

        checkLyricsLineTime();
        fixExtendedLrcTimeStamps();
    } else {
        nRet = openLyricsFile(szLrcSource, bUseSpecifiedEncoding, encodingSpecified);
    }

    m_nMediaLength = nMediaLength;

    m_strSongFile = szSongFile;
    m_strLrcSource = szLrcSource;
    m_lrcSourceType = lrcSourceType;
    m_isUsedSpecifiedEncoding = bUseSpecifiedEncoding;

    onLyricsOpened();

    return nRet;
}

int CMLData::newLyrics(cstr_t szSongFile, int nMediaLength) {
    close();

    m_strSongFile = szSongFile;
    m_nMediaLength = nMediaLength;

    return ERR_OK;
}

int CMLData::openLyrics(cstr_t szSongFile, int nMediaLength, uint8_t *szLrc, int nLen) {
    CLrcParser parser(this);

    int nRet = parser.parseString(szLrc, nLen, false);
    if (nRet != ERR_OK) {
        return nRet;
    }

    setLyrContentType(parser.getLyrContentType());

    m_strSongFile = szSongFile;
    m_nMediaLength = nMediaLength;
    m_lrcSourceType = LST_ID3V2_USLT;
    m_isUsedSpecifiedEncoding = false;

    onLyricsOpened();

    return ERR_OK;
}

#ifdef _IPHONE
int CMLData::openLyricsContent(cstr_t szSongFile, int nMediaLength, cstr_t szLyrContent) {
    m_strSongFile = szSongFile;
    m_nMediaLength = nMediaLength;
    m_lrcSourceType = LST_ID3V2_USLT;
    m_isUsedSpecifiedEncoding = false;

    int nRet = fromString(szLyrContent);
    if (nRet != ERR_OK) {
        return nRet;
    }

    onLyricsOpened();

    return ERR_OK;
}
#endif // #ifdef _IPHONE

//
// 重新打开歌词
//
int CMLData::reopenLyrics(bool bUseSpecifiedEncoding, CharEncodingType encodingSpecified) {
    string strLrcSource, strSongFile;

    strLrcSource = m_strLrcSource;
    strSongFile = m_strSongFile;

    return openLyrics(strSongFile.c_str(), m_nMediaLength, strLrcSource.c_str(), bUseSpecifiedEncoding, encodingSpecified);
}

int CMLData::openLyricsFile(cstr_t szFile, bool bUseSpecifiedEncoding, CharEncodingType encodingSpecified) {
    close();

    CLyricsParser *pParser = nullptr;
    if (fileIsExtSame(szFile, ".txt")
        || fileIsExtSame(szFile, ".lrc")) {
        pParser = new CLrcParser(this);
    }
#ifndef _IPHONE
    else if (fileIsExtSame(szFile, ".srt")) {
        pParser = new CSrtParser(this);
    }
#endif // #ifndef _IPHONE
    else {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    m_strLrcSource = szFile;

    int nRet = pParser->parseFile(bUseSpecifiedEncoding, encodingSpecified);
    if (nRet == ERR_OK) {
        setLyrContentType(pParser->getLyrContentType());
    }

    delete pParser;

    filterContents();

    return nRet;
}

#ifdef _ID3V2_SUPPORT
int CMLData::openLyricsInSongOfID3v2USLT(ID3v2UnsynchLyrics &lyrics, bool bUseSpecifiedEncoding, CharEncodingType encodingSpecified) {
    int nRet = ERR_OK;

    close();

    m_id3v2SaveCacheInfo.m_strLanguage = lyrics.m_szLanguage;
    m_id3v2SaveCacheInfo.m_strLrcDescription = lyrics.m_strContentDesc.c_str();

    nRet = fromString(lyrics.m_strLyrics.c_str());
    if (nRet != ERR_OK) {
        return nRet;
    }

    return ERR_OK;
}

int CMLData::openLyricsInSongOfID3v2SYLT(ID3v2SynchLyrics &lyrics, bool bUseSpecifiedEncoding, CharEncodingType encodingSpecified) {
    close();

    m_id3v2SaveCacheInfo.m_strLanguage = lyrics.m_szLanguage;
    m_id3v2SaveCacheInfo.m_bAllSyllableIsNewLine = lyrics.m_bAllSyllableIsNewLine;

    // read id3v2 SYLT lyrics information
    CLrcTag lrcTag;
    lrcTag.readFromText(lyrics.m_strContentDesc.c_str(), lyrics.m_strContentDesc.size());

    m_id3v2SaveCacheInfo.m_strLrcDescription = lyrics.m_strContentDesc.c_str();

    LyricsLine *pLine = nullptr;
    for (ID3v2SynchLyrics::LIST_SYLRC::iterator it = lyrics.m_vSynLyrics.begin();
    it != lyrics.m_vSynLyrics.end(); it++)
        {
        LrcSyllable &syllable = *it;

        if (syllable.bNewLine) {
            if (pLine && pLine->vFrags.size() > 1) {
                m_bKaraokeStyle = true;
            }
            pLine = nullptr;
            continue;
        }

        if (!pLine || lyrics.m_bAllSyllableIsNewLine) {
            pLine = newLyricsLine(syllable.nTime, -1);
            addLine(pLine);
        }

        pLine->appendPiece(syllable.nTime, -1, syllable.strText.c_str(),
            syllable.strText.size(), false, true);
    }

    m_lyrProperties = lrcTag;
    m_lyrProperties.setOffsetTime(0);
    setLyrContentType(LCT_LRC);

    checkLyricsLineTime();

    fixExtendedLrcTimeStamps();

    return ERR_OK;
}

int CMLData::openLyricsInSongOfLyrics3v2(const char *buffer, size_t len, bool bUseSpecifiedEncoding, CharEncodingType encodingSpecified) {
    int nRet;

    close();

    if (len == 0) {
        setCustomErrorDesc(_TLM("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
        return ERR_CUSTOM_ERROR;
    }

    //
    // Use CLrcParser to parse lyrics
    //
    CLrcParser parser(this);

    nRet = parser.parseString((uint8_t *)buffer, len, false, bUseSpecifiedEncoding, encodingSpecified);
    if (nRet == ERR_OK) {
        setLyrContentType(parser.getLyrContentType());
        m_lrcSourceType = LST_LYRICS3V2;

        filterContents();
    }

    return nRet;
}
#endif // #ifdef _ID3V2_SUPPORT

void CMLData::onLyricsOpened() {
    resetModificationFlag();

    if (m_arrLyricsLines.empty()) {
        return;
    }

    assert(getLyrContentType() != LCT_UNKNOWN);

    filterContents();

    if (getLyrContentType() == LCT_TXT) {
        m_lyrScrollActionRecorder.eventsDataFromString(
            getTxtLyrScrollEvents(),
            getRawLyrics());

        if (!m_lyrScrollActionRecorder.hasActions() && m_nMediaLength > 0) {
            // We need to the simple actions to make it scroll.
            m_lyrScrollActionRecorder.initCreateAutoEvents(m_arrLyricsLines);
            m_lyrScrollActionRecorder.scrollToLine(m_arrLyricsLines, 0, 10 * 1000, false);
            m_lyrScrollActionRecorder.scrollToLine(m_arrLyricsLines, (int)m_arrLyricsLines.size(), m_nMediaLength, true);
        }
    }
}

int CMLData::save() {
    if (!hasLyricsOpened()) {
        return ERR_NOLYRICS_OPENED;
    }

    if (!isModified()) {
        return ERR_OK;
    }

    if (m_lrcSourceType == LST_LYRICS3V2) {
        return saveLyricsInSongOfLyrics3v2();
    } else if ((m_lrcSourceType & LST_ID3V2) != 0) {
        VecStrings vLyrNames;
        vLyrNames.push_back(m_strLrcSource);
        return saveLyricsInSongOfID3v2(vLyrNames);
    } else if (m_lrcSourceType == LST_M4A_LYRICS) {
        return saveLyricsInSongOfM4a();
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

#ifdef _ID3V2_SUPPORT
int CMLData::saveLyricsInSongOfID3v2(VecStrings &vLyrNames) {
    if (vLyrNames.empty()) {
        return ERR_OK;
    }

    CID3v2IF id3v2(ED_UTF8);

    int nRet = id3v2.open(m_strSongFile.c_str(), true, true);
    if (nRet != ERR_OK) {
        return nRet;
    }

    string strLyrics;
    CharEncodingType encoding;
    toString(strLyrics, g_profile.getBool("KeepTimeStampsInTxtLyr", false) ? FT_LYRICS_LRC : FT_LYRICS_TXT, true);
    if (isAnsiStr(strLyrics.c_str())) {
        encoding = ED_SYSDEF;
    } else {
        encoding = ED_UNICODE;
    }

    for (size_t i = 0; i < vLyrNames.size(); ++i) {
        cstr_t szLyrName = vLyrNames[i].c_str();
        LRC_SOURCE_TYPE lst = lyrSrcTypeFromName(szLyrName);

        if (lst == LST_ID3V2_USLT) {
            nRet = id3v2.setUnsynchLyrics(szLyrName, "", strLyrics.c_str());
            if (nRet != ERR_OK) {
                return nRet;
            }
        } else if (lst == LST_ID3V2_SYLT && getLyrContentType() != LCT_TXT) {
            ID3v2SynchLyrics lyrics;

            toID3v2SynchLyrics(lyrics, encoding);

            nRet = id3v2.setSynchLyrics(szLyrName, lyrics);
            if (nRet != ERR_OK) {
                return nRet;
            }
        } else if (lst == LST_ID3V2_LYRICS) {
            nRet = id3v2.setUserDefLyrics(strLyrics.c_str());
            if (nRet != ERR_OK) {
                return nRet;
            }
        }
    }

    nRet = id3v2.save();
    if (nRet == ERR_OK) {
        resetModificationFlag();
    }

    return nRet;
}

int CMLData::saveLyricsInSongOfLyrics3v2() {
    // save lyrics3v2 lyrics in mp3 file
    string str;
    toString(str, FT_LYRICS_LRC, true);

    if (!isAnsiStr(str.c_str())) {
        str.insert(0, SZ_FE_UTF8);
    }

    CLyrics3v2 lyrics3v2;
    int nRet = lyrics3v2.writeLyrcsInfo(m_strSongFile.c_str(), (const char *)str.c_str(), (int)str.size() * sizeof(char), true);
    if (nRet == ERR_OK && m_lrcSourceType == LST_LYRICS3V2) {
        resetModificationFlag();
    }

    return nRet;
}

int CMLData::saveLyricsInSongOfM4a() {
    // save lyrics in m4a files
    string str;
    toString(str, g_profile.getBool("KeepTimeStampsInTxtLyr", false) ? FT_LYRICS_LRC : FT_LYRICS_TXT, true);

    CM4aTag tag;
    int nRet = tag.open(m_strSongFile.c_str(), true, false);
    if (nRet == ERR_OK) {
        int nRet = tag.setLyrics(str.c_str());
        if (nRet == ERR_OK) {
            nRet = tag.saveClose();
            if (nRet == ERR_OK && m_lrcSourceType == LST_M4A_LYRICS) {
                resetModificationFlag();
            }
        }
    }

    return nRet;
}
#endif // #ifdef _ID3V2_SUPPORT

// COMMENT:
//        将CMLData中的数据保存为文件szFile，文件的类型，由szFile的扩展名决定。
int CMLData::saveAsFile(cstr_t file, bool &bUseNewFileName) {
    if (!hasLyricsOpened()) {
        return ERR_NOLYRICS_OPENED;
    }

    // 基类接口
    CLyricsParser *pParser;

    bUseNewFileName = false;

    // 根据文件的扩展名来判断文件类型，然后调用不同的歌词分析类来分析歌词。
    if (fileIsExtSame(file, ".lrc") ) {
        pParser = new CLrcParser(this);
        if (getLyrContentType() <= LCT_LRC) {
            bUseNewFileName = true;
        }
    } else if (fileIsExtSame(file, ".txt") ) {
        pParser = new CLrcParser(this);
        if (getLyrContentType() <= LCT_TXT) {
            bUseNewFileName = true;
        }
    }
#ifndef _IPHONE
    else if (fileIsExtSame(file, ".srt") ) {
        pParser = new CSrtParser(this);
        if (getLyrContentType() <= LCT_TXT) {
            bUseNewFileName = true;
        }
    } else if (fileIsExtSame(file, ".snc") ) {
        pParser = new CSncParser(this);
    }
#endif // #ifndef _IPHONE
    else {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    // 分析歌词
    int nRet = pParser->saveAsFile(file);
    if (nRet == ERR_OK && bUseNewFileName) {
        resetModificationFlag();

        // 保存歌词文件名
        m_strLrcSource = file;
    }

    delete pParser;

    return nRet;
}

#ifdef _ID3V2_SUPPORT
int CMLData::updateTagToID3v2DescTxt() {
    CLrcTag lrcTag(m_lyrPropertiesOrg, m_lyrProperties);

#ifdef UNICODE
    lrcTag.writeToText(m_id3v2SaveCacheInfo.m_strLrcDescription, false);
#else
    // UTF8
    lrcTag.writeToText(m_id3v2SaveCacheInfo.m_strLrcDescription, false);
#endif

    return ERR_OK;
}
#endif // #ifdef _ID3V2_SUPPORT

bool CMLData::saveTag() {
    assert(!isContentModified());
    assert(fileIsExtSame(m_strLrcSource.c_str(), ".txt")
        || fileIsExtSame(m_strLrcSource.c_str(), ".lrc"));

    CLrcTag lrcTag(m_lyrPropertiesOrg, m_lyrProperties);

    if (lrcTag.writeToFile(m_strLrcSource.c_str(), getLyrContentType() != LCT_TXT) != ERR_OK) {
        return false;
    }

    m_isUsedSpecifiedEncoding = false;
    m_lyrPropertiesOrg = m_lyrProperties;

    return true;
}

void CMLData::resetModificationFlag() {
    string strContent;

    m_lyrPropertiesOrg = m_lyrProperties;
    toString(strContent, FT_LYRICS_LRC, false);
    m_md5OrgLyrContent = md5ToString(strContent.c_str(), strContent.size());

    m_isUsedSpecifiedEncoding = false;
}

LyricsLine *CMLData::getLyricsLine(int nLine) {
    if ((int)m_arrLyricsLines.size() <= nLine || nLine < 0) {
        return nullptr;
    }

    return m_arrLyricsLines[nLine];
}

// COMMENT:
//        检查歌词数据的时间顺序的正确性
//        查找所有的歌词显示时间为-1的“歌词段”，进行计算。
void CMLData::checkLyricsLineTime() {
    int nLineCount;
    int nBegLine = 0;
    int nBegFrag = 0;
    int nBegTime = 0;
    int nEndLine = -1;
    int nEndFrag = 0;
    int nEndTime = 0;
    int nLine = 0;
    int nFrag = 0;
    bool bFind = false;

    if (getLyricsLinesCount() == 0) {
        return;
    }

    if (getLyricsLine(0)->vFrags.size() == 0) {
        return;
    }

    while (1) {
        // 查找开始歌词段
        bFind = false;
        do {
            LyricsPiece *pPiece;
            LyricsLine *pLine;

            pLine = getLyricsLine(nLine);
            assert(pLine);

            pPiece = pLine->vFrags[nFrag];
            if (pPiece->nBegTime == -1) {
                pPiece = prevFrag(nLine, nFrag);
                if (pPiece) {
                    assert(pPiece->nEndTime != -1);
                    nBegTime = pPiece->nEndTime;
                } else {
                    // 一定是第一行，开始时间设置为 15 s
                    nBegTime = LYRICS_BEGIN_TIME;
                }
                nBegLine = nLine;
                nBegFrag = nFrag;
                bFind = true;
                break;
            }

            if (pLine->vFrags[nFrag]->nEndTime == -1) {
                nBegTime = pLine->vFrags[nFrag]->nBegTime;
                nBegLine = nLine;
                nBegFrag = nFrag;
                bFind = true;
                break;
            }

            nLine = nBegLine;
            nFrag = nBegFrag;
        }
        while (nextFrag(nLine, nFrag, nBegLine, nBegFrag));

        if (!bFind) { // 所有的歌词段的时间都是正确的。
            break;
        }

        // 查找结束歌词段
        while (1) {
            LyricsLine *pLine;

            if (!nextFrag(nLine, nFrag, nEndLine, nEndFrag)) {
                // 所有的歌词都没有时间标记，则最后的时间定为歌曲时间 － LYRICS_END_TIME
                nEndTime = getMediaLength() - LYRICS_END_TIME - getOffsetTime();
                if (nBegTime >= nEndTime) {
                    nEndTime = getMediaLength() - getOffsetTime();
                }
                if (nBegTime >= nEndTime) {
                    nEndTime = nBegTime + 10 * 1000;
                }
                nEndLine = nLine;
                nEndFrag = nFrag;
                break;
            }

            pLine = getLyricsLine(nEndLine);
            assert(pLine);

            if (pLine->vFrags[nEndFrag]->nBegTime != -1) {
                nEndTime = pLine->vFrags[nEndFrag]->nBegTime;
                nEndLine = nLine;
                nEndFrag = nFrag;
                break;
            }

            if (pLine->vFrags[nEndFrag]->nEndTime != -1) {
                nEndTime = pLine->vFrags[nEndFrag]->nEndTime;
                break;
            }

            nLine = nEndLine;
            nFrag = nEndFrag;
        }
        if (nBegTime > nEndTime) {
            nBegTime = nEndTime - 2 * 1000;
        }


        // 计算在开始段到最后段歌词之间的时间。
        calFragTimeBetweenLines(nBegLine, nBegFrag, nEndLine, nEndFrag, nBegTime, nEndTime);
    }

    // set time stamp of lyrics line.
    nLineCount = (int)m_arrLyricsLines.size();
    for (int i = 0; i < nLineCount; i++) {
        LyricsLine *pLine = m_arrLyricsLines[i];
        assert(pLine->vFrags.size() != 0);
        if (pLine->nBegTime == -1) {
            pLine->nBegTime = pLine->vFrags[0]->nBegTime;
        }
        if (pLine->nEndTime == -1) {
            pLine->nEndTime = pLine->vFrags[pLine->vFrags.size() - 1]->nEndTime;
        }
    }

    // If a lyrics line is 0 length, recover to display multiple line at same time.
    for (int i = 0; i < m_arrLyricsLines.size(); i++) {
        LyricsLine *pLine = m_arrLyricsLines[i];
        if (pLine->nBegTime != pLine->nEndTime || pLine->vFrags.size() != 1) {
            continue;
        }

        int nextLine = i;
        for (; nextLine < m_arrLyricsLines.size(); nextLine++) {
            LyricsLine *pNext = m_arrLyricsLines[nextLine];
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

void CMLData::fixExtendedLrcTimeStamps() {
    //
    // For Extended lrc lyrics, the end time of ending lyrics frag is incorrect.
    //
    if (!m_bKaraokeStyle) {
        return;
    }

    for (int i = 0; i < (int)getLyricsLinesCount(); i++) {
        LyricsLine *pLine = getLyricsLine(i);

        if (pLine->vFrags.size() < 1) {
            continue;
        }

        int n = (int32_t)pLine->vFrags.size() - 1;
        LyricsPiece *pPiece;
        pPiece = pLine->vFrags[n];
        if (pPiece->bTempEndTime) {
            LyricsLine *pNextLine;
            if (i + 1 < (int)getLyricsLinesCount()) {
                pNextLine = getLyricsLine(i + 1);
            } else {
                pNextLine = nullptr;
            }
            if (pNextLine) {
                if (pPiece->nBegTime + 500 < pNextLine->nBegTime) {
                    pPiece->nEndTime = pPiece->nBegTime + 500;
                }
            } else {
                pPiece->nEndTime = pPiece->nBegTime + 500;
            }
        }
    }
}

void CMLData::clearLyrics() {
    m_arrLyricsLines.erase(m_arrLyricsLines.begin(), m_arrLyricsLines.end());
    m_arrFileLines.clear();
}

void CMLData::close() {
    m_lyrPropertiesOrg.clear();
    m_lyrProperties.clear();
    assert(getLyrContentType() == LCT_UNKNOWN);
    m_md5OrgLyrContent = md5ToString((const char *)"", 0);
    m_nTimeElapsed = 0;

    m_bKaraokeStyle = false;

    m_lrcSourceType = LST_FILE;
    m_strSongFile.resize(0);;
    m_strLrcSource.resize(0);;

#ifdef _ID3V2_SUPPORT
    m_id3v2SaveCacheInfo.clear();
#endif // #ifdef _ID3V2_SUPPORT
    clearLyrics();
}

bool CMLData::hasLyricsOpened() {
    return getLyrContentType() != LCT_UNKNOWN;
}

void CMLData::addLine(LyricsLine *pFileLine) {
    if (pFileLine->bLyricsLine) {
        m_arrLyricsLines.push_back(pFileLine);
    }

    m_arrFileLines.push_back(pFileLine);
}

// COMMENT:
//        将歌词行按照时间排序插入
void CMLData::addLineByTime(LyricsLine *pLine) {
    if (pLine->bLyricsLine) {
        // 将歌词行插入 m_arrLyricsLines
        CLyricsLines::iterator itPos;

        int i, nLineCount;

        for (i = 0; i < (int)pLine->vFrags.size(); i++) {
            pLine->vFrags[i]->nDrawWidth = 0;
        }

        nLineCount = (int32_t)m_arrLyricsLines.size();
        for (i = 0; i < nLineCount; i++) {
            if (m_arrLyricsLines[i]->nBegTime > pLine->nBegTime) {
                break;
            }
        }

        if (i != nLineCount) {
            // 找到了位置
            itPos = find(m_arrFileLines.begin(), m_arrFileLines.end(), m_arrLyricsLines[i]);

            if (itPos == m_arrFileLines.end()) {
                // error occurs!
                assert(0);
                m_arrFileLines.push_back(pLine);
            } else {
                m_arrFileLines.insert(itPos, pLine);
            }

            // 插入歌词部分
            LyricsLine *pLinePrev;
            if (i > 0) {
                // reset the time of prev line.
                pLinePrev = m_arrLyricsLines[i - 1];
                if (pLinePrev->nEndTime != -1) {
                    if (pLinePrev->vFrags.size() == 1 && pLinePrev->vFrags[0]->bTempEndTime) {
                        pLinePrev->vFrags[0]->nEndTime = pLine->nBegTime;
                        pLinePrev->nEndTime = pLine->nBegTime;
                    }
                }
            }
            m_arrLyricsLines.insert(m_arrLyricsLines.begin() + i, pLine);
        } else {
            // 未找到位置，则插入末尾
            m_arrLyricsLines.push_back(pLine);
            m_arrFileLines.push_back(pLine);
        }
    } else {
        m_arrFileLines.push_back(pLine);
    }
}

string CMLData::getSuggestedLyricsFileName() {
    // Combine file name
    string strFileName;

    strFileName = formatMediaTitle(m_lyrProperties.m_strArtist.c_str(), m_lyrProperties.m_strTitle.c_str());
    strFileName = fileNameFilterInvalidChars(strFileName.c_str());

    if (strFileName.empty()) {
        strFileName = fileGetTitle(m_strSongFile.c_str());
    }

    if (m_lyrProperties.m_lyrContentType == LCT_LRC) {
        strFileName += ".lrc";
    } else {
        strFileName += ".txt";
    }

    return strFileName;
}

cstr_t CMLData::getLyricsFileName() {
    return m_strLrcSource.c_str();
}

// COMMENT:
//        设置时间偏移
void CMLData::SetPlayElapsedTime(int nTimeElapsed) {
    m_nTimeElapsed = nTimeElapsed + m_lyrProperties.getOffsetTime() - m_nGlobalOffsetTime;
}

// COMMENT:
//        取得
int CMLData::getPlayElapsedTime() {
    return m_nTimeElapsed;
}

// COMMENT:
//        取得当前正在演唱的歌词行
// RETURN:
//        歌词行的行号
int CMLData::getCurPlayLine(const CLyricsLines &lyrLines) {
    //// ERROR:在多线程中使用，可能会存在问题。
    static int nLine = 0;

    auto nLineCount = (int)lyrLines.size();

    if (nLineCount == 0) {
        return -1;
    }

    if (nLine >= nLineCount) {
        nLine = 0;
    }

    // 找到前一行
    for (; nLine >= 0; nLine--) {
        LyricsLine *pLyricLine = lyrLines[nLine];
        assert(pLyricLine);

        if (pLyricLine->nEndTime != -1 && pLyricLine->nEndTime < getPlayElapsedTime()) {
            nLine++;
            break; // ok.
        }
    }
    if (nLine == -1) {
        nLine = 0;
    }

    // 找到下一行
    for (; nLine < nLineCount; nLine++) {
        LyricsLine *pLyricLine = lyrLines[nLine];
        if (pLyricLine->nEndTime > getPlayElapsedTime()) {
            break; // ok.
        }
    }
    //    if (nLine > 0)
    //        nLine--;
    if (nLine >= nLineCount) {
        nLine = nLineCount - 1;
    }

    /*    for (int i = nLine - 1; i >= 0; i--)
    {
        if (lyrLines[nLine]->nEndTime != lyrLines[i]->nEndTime)
        {
            return i + 1;
        }
    }*/

    assert(nLine != -1);
    return nLine;
}

bool CMLData::doesChooseNewFileName() {
    return (m_strLrcSource.empty() || getLyrContentType() != m_lyrPropertiesOrg.m_lyrContentType);
}

bool CMLData::isModified() {
    // No lyrics opened.
    if (!hasLyricsOpened()) {
        return false;
    }

    return !m_lyrProperties.equal(m_lyrPropertiesOrg) || isContentModified();
}

bool CMLData::isOnlyAddLyrScrollActions() {
    if (getLyrContentType() != LCT_TXT) {
        return false;
    }
    if (isContentModified()) {
        return false;
    }

    return (isEmptyString(m_lyrPropertiesOrg.getTxtLyrScrollEvents())
        && !isEmptyString(m_lyrProperties.getTxtLyrScrollEvents()));
}

bool CMLData::isContentModified() {
    string strContent;

    toString(strContent, FT_LYRICS_LRC, false);
    string md5 = md5ToString(strContent.c_str(), strContent.size());

    return md5 != m_md5OrgLyrContent || m_isUsedSpecifiedEncoding;
}

//    COMMENT:
//        计算两个歌词段之间的所有歌词段开始时间和结束时间的值
//    INPUT:
//        nLine, nBegFrag    -    开始的位置，这个位置的时间可能还没有被计算出来。
//        nLine, nEndFrag    -    结束的位置，这个位置的时间可能还没有被计算出来。
bool CMLData::calFragTimeBetweenFrags(int nLine, int nBegFrag, int nEndFrag, int nBegTime, int nEndTime) {
    int nFragCount = nEndFrag - nBegFrag + 1;
    assert(nFragCount > 0);

    LyricsLine *pLine = getLyricsLine(nLine);
    assert(pLine);
    assert((unsigned int)nEndFrag < pLine->vFrags.size());

    for (int m = nBegFrag; m <= nEndFrag; m++) {
        pLine->vFrags[m]->nBegTime = nBegTime
        + ((nEndTime - nBegTime) * (m - nBegFrag)) / nFragCount;
        pLine->vFrags[m]->nEndTime = nBegTime +
        ((nEndTime - nBegTime) * (m - nBegFrag + 1)) / nFragCount;
    }

    return true;
}

//    COMMENT:
//        计算两个歌词行之间的所有歌词行的开始时间和结束时间的值
//    INPUT:
//        nBegLine, nBegFrag    -    开始的位置，这个位置的时间可能还没有被计算出来。
//        nEndLine, nEndFrag    -    结束的位置，这个位置的时间可能还没有被计算出来。
bool CMLData::calFragTimeBetweenLines(int nBegLine, int nBegFrag, int nEndLine, int nEndFrag, int nBegTime, int nEndTime) {
    LyricsLine *pLine = nullptr;

    assert(nEndTime >= nBegTime);
    assert(nEndLine >= nBegLine);
    assert(nBegLine < getLyricsLinesCount());
    assert(nEndLine < getLyricsLinesCount());

    if (nBegLine >= getLyricsLinesCount()) {
        return false;
    }
    if (nEndLine >= getLyricsLinesCount()) {
        return false;
    }

    // 如果所有的“歌词段”在一行内，则每一段的时间采用平均值
    if (nBegLine == nEndLine) {
        pLine = getLyricsLine(nBegLine);
        assert(pLine);

        calFragTimeBetweenFrags(nBegLine, nBegFrag, nEndFrag, nBegTime, nEndTime);

        //        if (pLine->nBegTime == -1)
        pLine->nBegTime = pLine->vFrags[0]->nBegTime;
        //        if (pLine->nEndTime == -1)
        pLine->nEndTime = pLine->vFrags[pLine->vFrags.size() - 1]->nEndTime;
    } else {
        // 计算最开始的行
        pLine = getLyricsLine(nBegLine);
        assert(pLine);

        //        if (pLine->nEndTime == -1)
        pLine->nEndTime = nBegTime +
        (nEndTime - nBegTime) / (nEndLine - nBegLine + 1);

        calFragTimeBetweenFrags(nBegLine, nBegFrag, (int)pLine->vFrags.size() - 1, nBegTime, pLine->nEndTime);

        //        if (pLine->nBegTime == -1)
        pLine->nBegTime = pLine->vFrags[0]->nBegTime;

        // “歌词段”不在一行内，每一行的时间采用平均值；在一行内再采用平均值
        for (int i = nBegLine + 1; i < nEndLine; i++) {
            pLine = getLyricsLine(i);
            assert(pLine);

            pLine->nBegTime = nBegTime +
            (nEndTime - nBegTime) * (i - nBegLine) / (nEndLine - nBegLine + 1);
            pLine->nEndTime = nBegTime +
            (nEndTime - nBegTime) * (i - nBegLine + 1) / (nEndLine - nBegLine + 1);

            calFragTimeBetweenFrags(i, 0, (int)pLine->vFrags.size() - 1, pLine->nBegTime, pLine->nEndTime);
        }

        // 计算结束歌词行的时间
        pLine = getLyricsLine(nEndLine);
        assert(pLine);
        pLine->nBegTime = nBegTime +
        (nEndTime - nBegTime) * (nEndLine - nBegLine) / (nEndLine - nBegLine + 1);

        calFragTimeBetweenFrags(nEndLine, 0, nEndFrag, pLine->nBegTime, nEndTime);

        //        if (pLine->nEndTime == -1)
        pLine->nEndTime = pLine->vFrags[pLine->vFrags.size() - 1]->nEndTime;
    }

    return true;
}

//    COMMENT:
//        取得上一个“歌词段”
bool CMLData::prevFrag(int nLine, int nFrag, int &nPrevLine, int &nPrevFrag, LyricsPiece **ppPiece) {
    assert(nLine < getLyricsLinesCount());
    assert(nFrag >= 0);
    assert((unsigned int)nFrag < getLyricsLine(nLine)->vFrags.size());

    if (nFrag <= 0) {
        if (nLine <= 0) {
            nPrevLine = 0;
            nPrevFrag = 0;
            if (ppPiece) {
                *ppPiece = nullptr;
            }
            return false;
        }

        nPrevLine = nLine - 1;
        nPrevFrag = (int)getLyricsLine(nPrevLine)->vFrags.size() - 1;
    } else {
        nPrevFrag--;
        nPrevLine = nLine;
    }

    if (ppPiece) {
        *ppPiece = getLyricsLine(nPrevLine)->vFrags[nPrevFrag];
    }

    return true;
}

//    COMMENT:
//        取得上一个“歌词段”
LyricsPiece *CMLData::prevFrag(int nLine, int nFrag) {
    assert(nLine < getLyricsLinesCount());
    assert(nFrag >= 0);
    assert((unsigned int)nFrag < getLyricsLine(nLine)->vFrags.size());

    LyricsLine *pLine;

    if (nFrag <= 0) {
        nLine--;
    }

    pLine = getLyricsLine(nLine);
    if (!pLine || pLine->vFrags.size() == 0) {
        return nullptr;
    }

    if (nFrag <= 0) {
        nFrag = (int)pLine->vFrags.size() - 1;
    }

    return pLine->vFrags[nFrag];
}

//    COMMENT:
//        取得下一个“歌词段”
bool CMLData::nextFrag(int nLine, int nFrag, int &nNextLine, int &nNextFrag, LyricsPiece **ppPiece) {
    assert(nLine >= 0 && nLine < getLyricsLinesCount());
    assert(nFrag >= 0);
    assert((unsigned int)nFrag < getLyricsLine(nLine)->vFrags.size());

    LyricsLine *pLine;
    nNextLine = nLine;
    pLine = getLyricsLine(nNextLine);
    if (!pLine) {
        return false;
    }

    nNextFrag = nFrag + 1;
    if ((unsigned int)nNextFrag >= pLine->vFrags.size()) {
        nNextLine = nLine + 1;
        pLine = getLyricsLine(nNextLine);
        if (!pLine || pLine->vFrags.size() <= 0) {
            nNextLine = 0;
            nNextFrag = 0;
            if (ppPiece) {
                *ppPiece = nullptr;
            }
            return false;
        }
        nNextFrag = 0;
    }

    if (ppPiece) {
        *ppPiece = getLyricsLine(nNextLine)->vFrags[nNextFrag];
    }

    return true;
}

//    COMMENT:
//        取得下一个“歌词段”
LyricsPiece *CMLData::nextFrag(int nLine, int nFrag) {
    assert(nLine < getLyricsLinesCount());
    assert(nFrag >= 0);
    assert((unsigned int)nFrag < getLyricsLine(nLine)->vFrags.size());

    nFrag ++;
    if ((unsigned int)nFrag >= getLyricsLine(nLine)->vFrags.size()) {
        nLine++;
        if (nLine >= getLyricsLinesCount()) {
            return nullptr;
        }
        nFrag = 0;
    }

    return getLyricsLine(nLine)->vFrags[nFrag];
}

bool CMLData::removeLyricsLine(int nLine) {
    LyricsLine *pLine;

    pLine = getLyricsLine(nLine);
    if (!pLine) {
        return false;
    }

    int k;

    m_arrLyricsLines.erase(m_arrLyricsLines.begin() + nLine);
    for (k = 0; k < (int)m_arrFileLines.size(); k++) {
        if (m_arrFileLines[k] == pLine) {
            m_arrFileLines.erase(m_arrFileLines.begin() + k);
            break;
        }
    }

    delete pLine;

    return true;
}

void CMLData::addTextInLyrics(VecStrings &vText) {
    int nPrevEnd;
    if (m_arrLyricsLines.size()) {
        nPrevEnd = m_arrLyricsLines.back()->nEndTime;
    } else {
        nPrevEnd = getPlayElapsedTime();
    }

    for (int i = 0; i < (int)vText.size(); i++) {
        int nEndTime = nPrevEnd + 8 * 1000;

        LyricsLine *pLine = newLyricsLine(nPrevEnd, nEndTime);
        pLine->bTempLine = true;

        pLine->appendPiece(pLine->nBegTime, pLine->nEndTime, vText[i].c_str(), vText[i].size(), true, true);
        nPrevEnd = nEndTime;

        addLineByTime(pLine);
    }
}

void CMLData::addAdInLyrics(int nBegTimeOffset, cstr_t szText, cstr_t szLink) {
    int nBegTime;

    // if ad exists, don't add new.
    for (int i = 0; i < (int)m_arrLyricsLines.size(); i++) {
        LyricsLine *pLine = m_arrLyricsLines[i];
        if (pLine->isAdvertise() && pLine->szContent && strcmp(pLine->szContent, szLink) == 0) {
            return;
        }
    }

    nBegTime = nBegTimeOffset;

    if (nBegTime < 0) {
        nBegTime = 0;
    }

    // add ad at beginning of the lyrics
    LyricsLine *pLine = newLyricsLine(nBegTime, -1, szLink, -1);
    pLine->bAdvertise = true;
    pLine->bTempLine = true;
    pLine->appendPiece(nBegTime, -1, szText, -1, true, true);
    addLineByTime(pLine);

    // add ad at the end of the lyrics.
    pLine = getLyricsLine(getLyricsLinesCount() - 1);
    if (pLine) {
        nBegTime = pLine->nEndTime;
        if (nBegTime != -1) {
            nBegTime = pLine->nBegTime + 5 * 1000;
        }
        pLine = newLyricsLine(nBegTime, -1, szLink, -1);
        pLine->bAdvertise = true;
        pLine->bTempLine = true;
        pLine->appendPiece(nBegTime, -1, szText, -1, true, true);
        addLine(pLine);
    }

    checkLyricsLineTime();
}

bool CMLData::lyricsLineToText(LyricsLine *pLine, string &strLyricsLine) {
    strLyricsLine.resize(0);
    if (!pLine) {
        return false;
    }

    if (pLine->bLyricsLine) {
        auto nCount = (int)pLine->vFrags.size();
        for (int i = 0; i < nCount; i++) {
            strLyricsLine += pLine->vFrags[i]->szLyric;
        }
    } else {
        if (pLine->szContent) {
            strLyricsLine = pLine->szContent;
        }
    }

    return true;
}

//
// 1、过滤歌词中的一些无法显示的字符：Tab,回车符号
// ?2、增加选项是否过滤行末、行首的空格
// 3、对HTML中的一些符号进行转义
int CMLData::filterContents(void) {
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
    for (uint32_t i = 0; i < m_arrLyricsLines.size(); i++) {
        for (uint32_t k = 0; k < m_arrLyricsLines[i]->vFrags.size(); k++) {
            char * szBeg = m_arrLyricsLines[i]->vFrags[k]->szLyric;

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
            szBeg[pos] = '\0';
            m_arrLyricsLines[i]->vFrags[k]->nLen = pos;
        }
    }

    return 0;
}

#ifdef _ID3V2_SUPPORT
void CMLData::toID3v2SynchLyrics(ID3v2SynchLyrics &lyrics, CharEncodingType encoding) {
    strcpy_safe(lyrics.m_szLanguage, CountOf(lyrics.m_szLanguage), m_id3v2SaveCacheInfo.m_strLanguage.c_str());
    lyrics.m_bTimeStampMs = true;

    CLrcTag lrcTag(m_lyrPropertiesOrg, m_lyrProperties);

    lrcTag.writeToText(m_id3v2SaveCacheInfo.m_strLrcDescription, true);

    lyrics.m_encodingType = charEncodingToID3v2EncType(encoding);
    lyrics.m_strContentDesc = m_id3v2SaveCacheInfo.m_strLrcDescription.c_str();

    // convert lyrics rows
    for (int i = 0; i < (int)m_arrLyricsLines.size(); i++) {
        LyricsLine *pLine = m_arrLyricsLines[i];

        if (pLine->bLyricsLine && !pLine->isTempLine()) {
            //
            // to compatible with windows media player, the Karaoke info has to be lost.
            //
            LrcSyllable syllable;
            syllable.bNewLine = false;
            syllable.nTime = pLine->nBegTime - m_lyrProperties.getOffsetTime();
            if (syllable.nTime < 0) {
                syllable.nTime = 0;
            }
            for (int k = 0; k < (int)pLine->vFrags.size(); k++) {
                LyricsPiece *pPiece = pLine->vFrags[k];

                syllable.strText += pPiece->szLyric;
            }
            lyrics.m_vSynLyrics.push_back(syllable);

            if (!m_id3v2SaveCacheInfo.m_bAllSyllableIsNewLine) {
                LrcSyllable syllable;
                syllable.bNewLine = true;
                lyrics.m_vSynLyrics.push_back(syllable);
            }
        }
    }
}

void CMLData::toID3v2UnsynchLyrics(ID3v2UnsynchLyrics &lyrics, CharEncodingType encoding) {
    string str;

    toString(str, g_profile.getBool("KeepTimeStampsInTxtLyr", false) ? FT_LYRICS_LRC : FT_LYRICS_TXT, true);

    strcpy_safe(lyrics.m_szLanguage, CountOf(lyrics.m_szLanguage), m_id3v2SaveCacheInfo.m_strLanguage.c_str());

    lyrics.m_encodingType = charEncodingToID3v2EncType(encoding);
    lyrics.m_strContentDesc = m_id3v2SaveCacheInfo.m_strLrcDescription.c_str();
    lyrics.m_strLyrics = str.c_str();
}
#endif // #ifdef _ID3V2_SUPPORT

void CMLData::toString(string &str, MLFileType lrcFormat, bool bIncTags) {
    bool bToLrcFormat = (lrcFormat == FT_LYRICS_LRC && getLyrContentType() != LCT_TXT);

    CLrcParser parser(this);

    parser.toString(str, bIncTags, !bToLrcFormat);
}

int CMLData::fromString(cstr_t szLyrics) {
    clearLyrics();

    CLrcParser parser(this);
    int nRet = parser.parseString(szLyrics, false);
    if (nRet == ERR_OK) {
        setLyrContentType(parser.getLyrContentType());

        filterContents();
    }

    return nRet;
}

void CMLData::copyLyricsLines(CLyricsLines &lyrLines) {
    int i;
    LyricsLine *pLine;

    lyrLines.clear();

    for (i = 0; i < (int)m_arrLyricsLines.size(); i++) {
        LyricsLine *pLineSrc = m_arrLyricsLines[i];

        pLine = duplicateLyricsLine(pLineSrc);
        lyrLines.push_back(pLine);
    }
}

#if UNIT_TEST

#include "utils/unittest.h"

string getTestDataFolder() {
    string testDataFolder = fileGetPath(__FILE__);
    dirStringAddSep(testDataFolder);
    testDataFolder += "TestData\\";
    return testDataFolder;
}

TEST(MLData, CMLData_OpenLrcFile) {
    auto testDataFolder = getTestDataFolder();

    cstr_t szSongFile = "artist - title.mp3";
    string        vstrLyrFile[] = {
        testDataFolder + "test(gb).lrc",
        testDataFolder + "test(ucs2).lrc",
        testDataFolder + "test(ucs2-be).lrc",
        testDataFolder + "test(utf8).lrc"
    };
    int nRet;
    CMLData data;

    for (int i = 0; i < CountOf(vstrLyrFile); i++) {
        nRet = data.openLyrics(szSongFile, 60 * 1000, vstrLyrFile[i].c_str());
        ASSERT_TRUE(nRet == ERR_OK);

        {
            // test with CLrcTag, tag same?
            CLrcTag tag;

            nRet = tag.readFromFile(vstrLyrFile[i].c_str());
            ASSERT_TRUE(nRet == ERR_OK);

            CLrcTag tagCompare(tag, data.properties());
            ASSERT_TRUE(!tagCompare.isTagChanged());
        }

        {
            // Lyrics content ok?
            // [00:01.00]L1
            // [01:01.01][01:00.01]L2
            // [02:01.10]L3
            cstr_t vLyrLines[] = { "L1", "L2", "L2", "L3" };
            int vTime[] = { 1*1000, 1*60*1000+10, (1*60+1)*1000+10, (2*60+1)*1000+100};

            CLyricsLines &lyrLines = data.getRawLyrics();

            for (int k = 0; k < lyrLines.size(); k++) {
                LyricsLine *pLine = lyrLines[k];

                ASSERT_TRUE(pLine->vFrags.size() == 1);
                ASSERT_TRUE(k < CountOf(vLyrLines));
                ASSERT_TRUE(strcmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
                ASSERT_TRUE(pLine->vFrags[0]->nBegTime == vTime[k]);
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
        string strFile = getAppDataDir();
        strFile += fileGetName(vstrSongFile[i].c_str());
        ASSERT_TRUE(copyFile(vstrSongFile[i].c_str(), strFile.c_str(), false));
    }
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
        string strFile = getAppDataDir();
        strFile += fileGetName(vstrSongFile[i].c_str());
        deleteFile(strFile.c_str());
    }
}

TEST(MLData, CMLData_OpenSaveID3v2_USLT) {
    auto testDataFolder = getTestDataFolder();
    int nRet;

    prepareMediaFile();

    string strWorkingFolder = getAppDataDir();

    //
    // test on embedded USLT lyrics.
    //

    string        vstrSongFile[] = {
        strWorkingFolder + "lyr-usly.mp3",
        strWorkingFolder + "lyr(wmp-chs).mp3",
        strWorkingFolder + "lyr-usly(chs).mp3",
        strWorkingFolder + "lyr(wmp-chs).mp3",
        strWorkingFolder + "lyr-chs.m4a",
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
            // loop twice, first for reading, second for writing.
            CMLData data;
            nRet = data.openLyrics(vstrSongFile[i].c_str(), 60 * 1000, vLyrFile[i]);
            ASSERT_TRUE(nRet == ERR_OK);

            {
                //
                // open, Lyrics content ok?
                //
                cstr_t vLyrLines[] = { "Line1", "Line2", "Line3" };
                cstr_t vLyrLinesChs[] = { "第一行", "第二行", "第三行" };

                CLyricsLines &lyrLines = data.getRawLyrics();

                for (int k = 0; k < lyrLines.size(); k++) {
                    LyricsLine *pLine = lyrLines[k];

                    ASSERT_TRUE(pLine->vFrags.size() == 1);
                    ASSERT_TRUE(k < CountOf(vLyrLines));
                    if (i < 2) {
                        ASSERT_TRUE(strcmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
                    } else {
                        ASSERT_TRUE(strcmp(pLine->vFrags[0]->szLyric, vLyrLinesChs[k]) == 0);
                    }
                }
                ASSERT_TRUE(lyrLines.size() == CountOf(vLyrLines));
            }

            // test on save uslt
            if (!vWriteTested[i]) {
                data.properties().m_strArtist = "artist";
                data.properties().m_strTitle = "title";
                ASSERT_TRUE(data.save() == ERR_OK);
                vWriteTested[i] = true;
            } else {
                ASSERT_TRUE(strcmp(data.properties().m_strArtist.c_str(), "artist") == 0);
                ASSERT_TRUE(strcmp(data.properties().m_strTitle.c_str(), "title") == 0);
            }
        }
    }

    removeTempMediaFile();
}

TEST(MLData, CMLData_OpenSaveID3v2_SYLT) {
    auto testDataFolder = getTestDataFolder();
    int nRet;
    CMLData data;

    prepareMediaFile();

    string strWorkingFolder = getAppDataDir();

    //
    // test on embedded SYLT lyrics.
    //
    string        vstrSongFileSylt[] = {
        strWorkingFolder + "lyr(wmp-chs).mp3",
        strWorkingFolder + "lyr(wmp-chs).mp3",
    };
    cstr_t        vLyrFileSylt[] = {
        "song://id3v2/sylt/eng",
        "song://id3v2/sylt/chi",
    };

    bool vWriteTested[] = { false, false, false, false };

    for (int kk = 0; kk < 2; kk++) {
        // loop twice, first for reading, second for writing.
        int nOffsetTime = 0;
        if (kk == 1) {
            nOffsetTime = 500;
        }

        for (int i = 0; i < CountOf(vstrSongFileSylt); i++) {
            nRet = data.openLyrics(vstrSongFileSylt[i].c_str(), 60 * 1000, vLyrFileSylt[i]);
            ASSERT_TRUE(nRet == ERR_OK);

            {
                // Lyrics content ok?
                cstr_t vLyrLines[] = { "Line1", "Line2", "Line3" };
                cstr_t vLyrLinesChs[] = { "第一行", "第二行", "第三行" };
                int vTime[] = { 1, 1*1000+9, 1*1000+18 };

                CLyricsLines &lyrLines = data.getRawLyrics();

                for (int k = 0; k < lyrLines.size(); k++) {
                    LyricsLine *pLine = lyrLines[k];

                    ASSERT_TRUE(pLine->vFrags.size() == 1);
                    ASSERT_TRUE(k < CountOf(vLyrLines));
                    if (i < 1) {
                        ASSERT_TRUE(strcmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
                    } else {
                        ASSERT_TRUE(strcmp(pLine->vFrags[0]->szLyric, vLyrLinesChs[k]) == 0);
                    }

                    ASSERT_TRUE(pLine->vFrags[0]->nBegTime == vTime[k] + nOffsetTime);
                }
                ASSERT_TRUE(lyrLines.size() == CountOf(vLyrLines));
            }

            // test on save SYLT
            if (!vWriteTested[i]) {
                data.properties().m_strArtist = "artist";
                data.properties().m_strTitle = "title";
                data.properties().setOffsetTime(-500);
                ASSERT_TRUE(data.save() == ERR_OK);
                vWriteTested[i] = true;
            } else {
                ASSERT_TRUE(strcmp(data.properties().m_strArtist.c_str(), "artist") == 0);
                ASSERT_TRUE(strcmp(data.properties().m_strTitle.c_str(), "title") == 0);
            }
        }
    }

    removeTempMediaFile();
}

TEST(MLData, CMLData_OpenSaveLyrics3v2) {
    auto testDataFolder = getTestDataFolder();
    int nRet;
    CMLData data;

    prepareMediaFile();

    string strWorkingFolder = getAppDataDir();

    string        vstrSongFileSylt[] = {
        strWorkingFolder + "lyrics3v2.mp3",
    };
    cstr_t        vLyrFileSylt[] = {
        SZ_SONG_LYRICS3V2,
    };
    bool vWriteTested[] = { false };

    for (int kk = 0; kk < 2; kk++) {
        // loop twice, first for reading, second for writing.
        int nOffsetTime = 0;
        if (kk == 1) {
            nOffsetTime = 500;
        }

        for (int i = 0; i < CountOf(vstrSongFileSylt); i++) {
            nRet = data.openLyrics(vstrSongFileSylt[i].c_str(), 60 * 1000, vLyrFileSylt[i]);
            ASSERT_TRUE(nRet == ERR_OK);

            {
                // Lyrics content ok?
                cstr_t vLyrLines[] = { "第一行", "第二行", "第三行" };
                int vTime[] = { 0, 1*1000+10, 1*1000+20 };

                CLyricsLines &lyrLines = data.getRawLyrics();

                for (int k = 0; k < lyrLines.size(); k++) {
                    LyricsLine *pLine = lyrLines[k];

                    ASSERT_TRUE(pLine->vFrags.size() == 1);
                    ASSERT_TRUE(k < CountOf(vLyrLines));
                    ASSERT_TRUE(strcmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
                    ASSERT_TRUE(pLine->vFrags[0]->nBegTime == vTime[k]);
                }
                ASSERT_TRUE(lyrLines.size() == CountOf(vLyrLines));
            }

            // test on saving
            if (!vWriteTested[i]) {
                data.properties().m_strArtist = "artist";
                data.properties().m_strTitle = "title";
                ASSERT_TRUE(data.save() == ERR_OK);
                vWriteTested[i] = true;
            } else {
                ASSERT_TRUE(strcmp(data.properties().m_strArtist.c_str(), "artist") == 0);
                ASSERT_TRUE(strcmp(data.properties().m_strTitle.c_str(), "title") == 0);
            }
        }
    }

    removeTempMediaFile();
}

#endif
