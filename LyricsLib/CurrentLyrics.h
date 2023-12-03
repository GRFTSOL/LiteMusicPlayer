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


class CurrentLyrics {
public:
    CurrentLyrics();
    virtual ~CurrentLyrics();

    int newLyrics(cstr_t szSongFile, int nMediaLength);

    int openLyrics(cstr_t szSongFile, int nMediaLength, cstr_t szLrcSource, bool bUseSpecifiedEncoding = false, CharEncodingType encodingSpecified = ED_SYSDEF);
    int reopenWithEncoding(CharEncodingType encodingSpecified);

    int saveAsFile(cstr_t file, bool &bUseNewFileName);
    int save();

    string toString(bool isIncTags = true);
    int fromString(cstr_t szLyrics);

    int getPlayElapsedTime();
    void SetPlayElapsedTime(int nTimeElapsed);

    int getCurPlayLine(const LyricsLines &lyrLines);

    LyricsLines &getLyricsLines() { return m_lyricsLines; }
    RawLyrics &getRawLines() { return m_rawLyrics; }

    void setKaraokeStyle(bool bKaraoke = true) { m_bKaraokeStyle = bKaraoke; }
    bool isKaraokeShowStyle() { return m_bKaraokeStyle; }

public:
    string getSuggestedLyricsFileName();
    cstr_t getLyricsFileName();

    LyricsProperties &properties() { return m_rawLyrics.properties(); }

    LRC_SOURCE_TYPE getLyricsSourceType() { return m_lrcSourceType; }

    LyricsContentType getLyrContentType()
        { return m_rawLyrics.properties().lyrContentType; }
    void setLyrContentType(LyricsContentType lyrContentType)
        { m_rawLyrics.properties().lyrContentType = lyrContentType; }

    bool doesChooseNewFileName();

    // 取得偏移时间量
    int getOffsetTime() { return m_rawLyrics.properties().getOffsetTime(); }
    void setOffsetTime(int nOffsetTime) { m_rawLyrics.properties().setOffsetTime(nOffsetTime); }

    cstr_t getMediaSource() { return m_strSongFile.c_str(); }
    int getMediaLength() { return m_nMediaLength; }

    bool hasLyricsOpened();
    bool isModified();
    bool isContentModified();
    void close();
    void clearLyrics();

protected:
    int openLyricsFile(cstr_t szFile, bool bUseSpecifiedEncoding = false, CharEncodingType encodingSpecified = ED_SYSDEF);

    bool saveTag();

    void resetModificationFlag();

    int filterContents(void);

protected:
    // All the file lines of the lyrics file.
    RawLyrics                   m_rawLyrics;

    // Only lyrics lines are saved here, it's just a sub set of m_rawLyrics.
    LyricsLines                 m_lyricsLines;

    // 歌词文件
    string                      m_strLrcSource;
    string                      m_strSongFile;

    // lyrics source type: file, id3v2, lyrics3v2
    LRC_SOURCE_TYPE             m_lrcSourceType;

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
