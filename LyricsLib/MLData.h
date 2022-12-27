/********************************************************************
    Created :    2001/12/19    22:49
    FileName:    mldata.h
    Author  :    xhy
    Purpose :    各种歌词格式的歌词数据存储方式
*********************************************************************/

#pragma once


#ifndef _IPHONE
#define _ID3V2_SUPPORT
#endif

#include "../MediaTags/MediaTags.h"
#include "../MediaTags/LyricsData.h"

#include "../MLProtocol/MLUtils.h"
#include "LrcTag.h"

#include "LyrScrollActionRecorder.h"


MLFileType lyricsConentTypeToFileType(LYRICS_CONTENT_TYPE lyrContentType);

class CMLData {
public:
    CMLData();
    virtual ~CMLData();

    int newLyrics(cstr_t szSongFile, int nMediaLength);

    int openLyrics(cstr_t szSongFile, int nMediaLength, cstr_t szLrcSource, bool bUseSpecifiedEncoding = false, CharEncodingType encodingSpecified = ED_SYSDEF);
    int openLyrics(cstr_t szSongFile, int nMediaLength, uint8_t *szLrc, int nLen);
#ifdef _IPHONE
    int openLyricsContent(cstr_t szSongFile, int nMediaLength, cstr_t szLyrContent);
#endif
    int reopenLyrics(bool bUseSpecifiedEncoding, CharEncodingType encodingSpecified);

    int saveAsFile(cstr_t file, bool &bUseNewFileName);
    int save();

#ifdef _ID3V2_SUPPORT
    int saveLyricsInSongOfID3v2(VecStrings &vLyrNames);
    int saveLyricsInSongOfLyrics3v2();
    int saveLyricsInSongOfM4a();
#endif

    void toString(string &str, MLFileType lrcFormat = FT_LYRICS_LRC, bool bIncTags = false);
    // int fromString(const void *lpLrc, int nLen, bool bAddTimeStampForTextLine = false, bool bUseSpecifiedEncoding = false, CharEncodingType encodingSpecified = ED_SYSDEF);
    int fromString(cstr_t szLyrics);


    int getPlayElapsedTime();
    void SetPlayElapsedTime (int nTimeElapsed);

    int getCurPlayLine(const CLyricsLines &lyrLines);

    uint32_t getLyricsLinesCount() { return (uint32_t)m_arrLyricsLines.size(); }
    LyricsLine *getLyricsLine(int nLine);
    CLyricsLines &getRawLyrics() { return m_arrLyricsLines; }
    CLyricsLines &getFileLines() { return m_arrFileLines; }

    void copyLyricsLines(CLyricsLines &lyrLines);

    void setKaraokeStyle(bool bKaraoke = true) { m_bKaraokeStyle = bKaraoke; }
    bool isKaraokeShowStyle() { return m_bKaraokeStyle; }

    // 检查歌词数据的时间顺序的正确性
    void checkLyricsLineTime();

    void fixExtendedLrcTimeStamps();

    // 计算两个歌词段之间的所有歌词段开始时间和结束时间的值
    bool calFragTimeBetweenFrags(int nLine, int nBegFrag, int nEndFrag, int nBegTime, int nEndTime);

    // 计算两个歌词行之间的所有歌词行的开始时间和结束时间的值
    bool calFragTimeBetweenLines(int nBegLine, int nBegFrag, int nEndLine, int nEndFrag, int nBegTime, int nEndTime);

    // 添加歌词行
    void addLine(LyricsLine *pFileLine);
    void addLineByTime(LyricsLine *pLine);

    void addTextInLyrics(VecStrings &vText);

    bool removeLyricsLine(int nLine);

    void addAdInLyrics(int nBegTimeOffset, cstr_t szText, cstr_t szLink);

    bool lyricsLineToText(LyricsLine *pLine, string &strLyricsLine);

    int filterContents(void);

public:
    //
    // Interface for Lyrics Scroll Action Recorder (CLyrScrollActionRecorder).
    //
    void clearRecordedLyrScrollActions(CLyricsLines &lyrLines) {
        assert(getLyrContentType() == LCT_TXT);
        m_lyrScrollActionRecorder.initCreateAutoEvents(lyrLines);
        m_lyrScrollActionRecorder.initCreateAutoEvents(getRawLyrics());
    }

    bool isUsingLyrScrollActionRecorder()
        { return getLyrContentType() == LCT_TXT && m_lyrScrollActionRecorder.hasActions(); }

    void updateTimeTagByLyrScrollActions(CLyricsLines &lyrLines)
        { assert(getLyrContentType() == LCT_TXT); m_lyrScrollActionRecorder.updateTimeTagByEvents(lyrLines); }

    void lyrScrollActionsToTag() {
        if (getLyrContentType() == LCT_TXT) {
            m_lyrProperties.setTxtLyrScrollEvents(m_lyrScrollActionRecorder.eventsDataToString().c_str());
        }
    }

    void lyrScrollToLine(CLyricsLines &lyrLines, int nLine, int nTime, bool bUpdateTimeTag = true) {
        assert(getLyrContentType() == LCT_TXT);
        m_lyrScrollActionRecorder.scrollToLine(lyrLines, nLine, nTime, bUpdateTimeTag);
    }

    bool isOnlyAddLyrScrollActions();

public:
    string getSuggestedLyricsFileName();
    cstr_t getLyricsFileName();
    cstr_t getSongFileName() { return m_strSongFile.c_str(); }

    LyricsProperties &properties() { return m_lyrProperties; }

    void setTxtLyrScrollEvents(cstr_t szScrEvents) { m_lyrProperties.setTxtLyrScrollEvents(szScrEvents); }
    cstr_t getTxtLyrScrollEvents() { return m_lyrProperties.getTxtLyrScrollEvents(); }

    LRC_SOURCE_TYPE getLyricsSourceType() { return m_lrcSourceType; }

    LYRICS_CONTENT_TYPE getLyrContentType() { return m_lyrProperties.m_lyrContentType; }
    void setLyrContentType(LYRICS_CONTENT_TYPE lyrContentType) { m_lyrProperties.m_lyrContentType = lyrContentType; }

    bool doesChooseNewFileName();

    // 取得偏移时间量
    int getOffsetTime() { return m_lyrProperties.getOffsetTime(); }
    void setOffsetTime(int nOffsetTime) { m_lyrProperties.setOffsetTime(nOffsetTime); }

    cstr_t getMediaSource() { return m_strSongFile.c_str(); }
    int getMediaLength() { return m_nMediaLength; }

    bool hasLyricsOpened();
    bool isModified();
    bool isContentModified();
    void close();
    void clearLyrics();

protected:
    int openLyricsFile(cstr_t szFile, bool bUseSpecifiedEncoding = false, CharEncodingType encodingSpecified = ED_SYSDEF);
#ifdef _ID3V2_SUPPORT
    int openLyricsInSongOfID3v2USLT(ID3v2UnsynchLyrics &lyrics, bool bUseSpecifiedEncoding = false, CharEncodingType encodingSpecified = ED_SYSDEF);
    int openLyricsInSongOfID3v2SYLT(ID3v2SynchLyrics &lyrics, bool bUseSpecifiedEncoding = false, CharEncodingType encodingSpecified = ED_SYSDEF);
    int openLyricsInSongOfLyrics3v2(const char *buffer, size_t len, bool bUseSpecifiedEncoding = false, CharEncodingType encodingSpecified = ED_SYSDEF);

    void toID3v2SynchLyrics(ID3v2SynchLyrics &lyrics, CharEncodingType encoding);
    void toID3v2UnsynchLyrics(ID3v2UnsynchLyrics &lyrics, CharEncodingType encoding);

    int updateTagToID3v2DescTxt();
#endif // #ifdef _ID3V2_SUPPORT

    void onLyricsOpened();

    bool saveTag();

    void resetModificationFlag();

    // 取得下一个“歌词段”
    LyricsPiece *nextFrag(int nLine, int nFrag);
    bool nextFrag(int nLine, int nFrag, int &nNextLine, int &nNextFrag, LyricsPiece **ppPiece = nullptr);

    // 取得上一个“歌词段”
    LyricsPiece *prevFrag(int nLine, int nFrag);
    bool prevFrag(int nLine, int nFrag, int &nPrevLine, int &nPrevFrag, LyricsPiece **ppPiece = nullptr);

public:

    // All the file lines of the lyrics file.
    CLyricsLines                m_arrFileLines;

    int                         m_nGlobalOffsetTime;

protected:
    // Only lyrics lines are saved here, it's just a sub copy of m_arrFileLines.
    CLyricsLines                m_arrLyricsLines;

protected:
    CLyrScrollActionRecorder    m_lyrScrollActionRecorder;

#ifdef _ID3V2_SUPPORT
    // save id3v2 lyrics information for cache.
    struct ID3v2LyricsSaveCacheInfo {
        ID3v2LyricsSaveCacheInfo();
        string                      m_strLrcDescription; // save id3v2 lyrics description
        string                      m_strLanguage;

        bool                        m_bAllSyllableIsNewLine;
        void clear();
    };
    ID3v2LyricsSaveCacheInfo    m_id3v2SaveCacheInfo;
#endif // #ifdef _ID3V2_SUPPORT

    // 歌词文件
    string                      m_strLrcSource;
    string                      m_strSongFile;

    // lyrics source type: file, id3v2, lyrics3v2
    LRC_SOURCE_TYPE             m_lrcSourceType;

    // lyrics properties: tag(artist, title...)
    LyricsProperties            m_lyrProperties;

    // original lyrics info.
    string                      m_md5OrgLyrContent; // used for lyrics modification compare.
    LyricsProperties            m_lyrPropertiesOrg;

    // 协助判断是否仅仅保存 tags
    bool                        m_isUsedSpecifiedEncoding;

    // lyrics display style
    bool                        m_bKaraokeStyle;

    // current playing position of lyrics
    int                         m_nTimeElapsed;

    // length of the current media file
    int                         m_nMediaLength;

};
