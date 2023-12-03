/********************************************************************
    Created  :    2002/01/04    21:40
    FileName :    LrcParser.h
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#pragma once

#include "LyricsData.h"

#define    SZ_LRC_AR        "ar:"
#define    SZ_LRC_TI        "ti:"
#define    SZ_LRC_AL        "al:"
#define    SZ_LRC_BY        "by:"
#define    SZ_LRC_OFFSET    "offset:"
#define    SZ_LRC_LENGTH    "length:"
#define    SZ_LRC_ID        "id:"

#define    SZ_TXT_AR        "Artist:"
#define    SZ_TXT_TI        "Title:"
#define    SZ_TXT_AL        "Album:"
#define    SZ_TXT_ID        "ID:"


string formatLrcTimeTag(int time, bool isLongTimeFmt);
string convertBinLyricsToUtf8(const StringView &data, bool useSpecifiedEncoding, CharEncodingType encoding);
void setDefaultLyricsEncoding(CharEncodingType encoding);
CharEncodingType getDefaultLyricsEncoding();

int parseLyricsBinary(const StringView &lyrics, bool useSpecifiedEncoding,
    CharEncodingType encoding, RawLyrics &rawLyricsOut);
int parseLyricsString(const StringView &lyrics, bool isAddTimeStampForTextLine, RawLyrics &rawLyricsOut);
string toLyricsBinary(const RawLyrics &rawLyrics);
string toLyricsString(const RawLyrics &rawLyrics, bool isIncTags, bool isToTxtFormat);
string toLyricsString(const RawLyrics &rawLyrics);

bool compressLyrics(string &lyrics);

class LyrTagParser {
public:
    LyrTagParser(LyricsProperties &props);

    // 从 utf-8 编码的 content 中读取 tags.
    void parseTxtAllTags(const StringView &content);
    void parseLrcAllTags(const StringView &content);

    bool parseTxtTag(const StringView &line);
    bool parseLrcTag(const StringView &line);

    bool isTxtTag(const StringView &line);
    bool isLrcTag(const StringView &line);

    void toTxtTags(string &headTagsOut, string &tailTagsOut, RawLyrics *rawLyrics = nullptr);
    string toLrcTags();

    string joinTagsLyrics(const StringView &lyrics, bool isLrcFormat);

    // 替换歌词文件中的 tag，自适应各种编码，保存的文件编码为 utf-8
    bool replaceInFile(cstr_t fileName, bool isLrcFormat);

    struct Item {
        StringView                  lrcName;
        StringView                  txtName;
        string                      *value;
    };

    void setLyrContentType(LyricsContentType lct) { _props.lyrContentType = lct; }
    bool shouldReadLyrContentType() const
        { return _props.lyrContentType == LCT_UNKNOWN; }

protected:
    void setValue(Item &item, StringView value);

protected:
    vector<Item>                _items;
    LyricsProperties            &_props;
    string                      _unused;
    string                      _offsetTime;

};
