#pragma once

#include "../Utils/Utils.h"


// TXT < LRC < karaoke , precision of timestamps is higher
enum LyricsContentType {
    LCT_UNKNOWN                 = 0,
    LCT_TXT                     = 1 << 1,
    LCT_LRC                     = 1 << 2,
    LCT_KARAOKE                 = 1 << 4, // karaoke content, full synced.

    LCT_ALL                     = LCT_TXT | LCT_LRC | LCT_KARAOKE,
};

LyricsContentType getLyricsContentTypeByFileExt(cstr_t fileName);

struct LyricsProperties {

    bool equal(const LyricsProperties &other) const;
    void clear();

    void setOffsetTime(int nOffsetTime);
    void setOffsetTime(cstr_t szOfffsetTime) { _strTimeOffset = szOfffsetTime; _timeOffset = atoi(szOfffsetTime); }
    const string &getOffsetTimeStr() { return _strTimeOffset; }
    int getOffsetTime() const { return _timeOffset; }

    void setMediaLength(int nMediaLength);
    int getMediaLengthInt() const;

    string                      artist;
    string                      title;
    string                      album;
    string                      by;
    string                      mediaLength;
    string                      id;
    LyricsContentType           lyrContentType = LCT_UNKNOWN;

protected:
    // offset time of timestamps in lyrics, in ms
    int                         _timeOffset = 0;
    string                      _strTimeOffset;    // Also, it will store TxtLyrScrollEvents

};

struct LyricsPiece {
    int16_t                     drawWidth = 0;         // The width of lyrics in Graph

    int                         beginTime = -1;
    int                         endTime = -1;
    string                      text;

    LyricsPiece(int beginTime, int endTime, const StringView &text) : beginTime(beginTime), endTime(endTime), text(text.data, text.len) {
    }

};

using VecLyricsPieces = vector<LyricsPiece>;

struct LyricsLine {
    bool                        isTempLine = false;
    bool                        isLyricsLine = true;

    // For lyrics line:
    VecLyricsPieces             pieces;
    int                         beginTime = -1;
    int                         endTime = -1;

    // For Non-lyrics line:
    string                      content;         // content of the row

    LyricsLine(int beginTime, int endTime) : beginTime(beginTime), endTime(endTime) {
        isLyricsLine = true;
        isTempLine = false;
    }

    void appendPiece(int beginTime, int endTime, const StringView &text);

    const string joinPiecesText() const {
        string buf;
        for (auto &piece : pieces) {
            buf += piece.text;
        }
        return buf;
    }

    int beginTimeOfPiece() const { assert(!pieces.empty()); return pieces.front().beginTime; }

};

using VecLyrLines = std::vector<LyricsLine>;

class LyricsLines {
public:
    using iterator = VecLyrLines::iterator;
    using const_iterator = VecLyrLines::const_iterator;

    LyricsLines() { }
    virtual ~LyricsLines() { clear(); }

    virtual void clear();
    void clearDrawContextWidth();

    int getCountWithSameTimeStamps(int index);

    void addLyricsLine(const StringView &text);
    void addNoneLyricsLine(const StringView &text);

    void push_back(const LyricsLine &line) { _lines.push_back(line); }
    void push_back(LyricsLine &&line) { _lines.push_back(line); }

    bool empty() const { return _lines.empty(); }
    size_t size() const { return _lines.size(); }

    VecLyrLines &lines() { return _lines; }
    const VecLyrLines &lines() const { return _lines; }

    iterator begin() { return _lines.begin(); }
    iterator end() { return _lines.end(); }

    const_iterator begin() const { return _lines.begin(); }
    const_iterator end() const { return _lines.end(); }

    LyricsLine &operator[](size_t index) { return _lines[index]; }
    const LyricsLine &operator[](size_t index) const { return _lines[index]; }

protected:
    VecLyrLines                 _lines;

};

/**
 * 从源歌词(各种嵌入文件的歌词、歌词文件等格式)解析出来的原始歌词内容
 *
 * * 包含了带时间标签的歌词和不带时间标签的行（空行、文字行）
 * * 歌词的属性都被提取出来放在 _props 中了.
 */
class RawLyrics : public LyricsLines {
public:
    void clear() override;

    LyricsLines toLyricsLinesOnly(int durationMs = 0) const;
    void analyzeContentType();

    LyricsProperties &properties() { return _props; }
    const LyricsProperties &properties() const { return _props; }

    LyricsContentType contentType() const { return _props.lyrContentType; }

protected:
    LyricsProperties            _props;

};

void formatPlayTime(int nmsTime, char szBuf[]);

void getArtistTitleFromFileName(string &strArtist, string &strTitle, cstr_t szFile);

/**
* Format media title in format of "artist - title"
**/
string formatMediaTitle(cstr_t szArtist, cstr_t szTitle);
