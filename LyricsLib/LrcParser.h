/********************************************************************
    Created  :    2002/01/04    21:40
    FileName :    LrcParser.h
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#pragma once

#include "LyricsParser.h"


class CLrcParser : public CLyricsParser
{
public:
    CLrcParser(CMLData *pMLData, bool bTurnOffPref = false);
    virtual ~CLrcParser();

public:
    int parseFile(bool bUseSpecifiedEncoding, CHAR_ENCODING encoding);
    int saveAsFile(cstr_t file);

    virtual LYRICS_CONTENT_TYPE getLyrContentType() { return m_lyrContentType; }

    int parseString(cstr_t szText, bool bAddTimeStampForTextLine = false);
    int parseString(uint8_t *szLrc, size_t nLen, bool bAddTimeStampForTextLine = false, bool bUseSpecifiedEncoding = false, CHAR_ENCODING encoding = ED_SYSDEF);

    void toString(string &str, bool bIncTags, bool bToTxtFormat);

protected:
    void toLrcString(string &str, bool bIncTags, bool bLongTimeFmt);
    void toTxtString(string &str, bool bIncTags);

    bool lyricsLineToLrcString(LyricsLine *pLine, bool bLongTimeFmt, string &strBuff);
    void lyricsLineToTxtString(LyricsLine *pLine, string &strBuff);

    void addLyrLines(vector<int> &vTimes, cstr_t szLyrLine);
    void addStrAsTimedLine(const char *szLine);
    void addStrAsUnknowLine(const char *szLine);

    void parseTxtTagInLyrics();

    void finalFixOnLyrics(bool bAddTimeStampForTextLine);

    void fixTextLrcTimeTag();
    void removeExtraBlankLinesOfTextLyr();

    // old
    void lRCCheckRowTime();

protected:
    LYRICS_CONTENT_TYPE         m_lyrContentType;

    // 上一次添加的歌词行的时间，用于添加没有时间标签的文本歌词行
    int                         m_nLatestTime;
    bool                        m_bTurnOffPref;

};
