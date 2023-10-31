#include  <algorithm>
#include "LyricsData.h"


LyricsContentType getLyricsContentTypeByFileExt(cstr_t fileName) {
    auto ext = fileGetExt(fileName);
    if (isEmptyString(ext)) {
        return LCT_UNKNOWN;
    }

    if (strcasecmp(ext, ".lrc") == 0) {
        return LCT_LRC;
    } else if (strcasecmp(ext, ".txt") == 0) {
        return LCT_TXT;
    } else if (strcasecmp(ext, ".snc") == 0) {
        return LCT_LRC;
    } else if (strcasecmp(ext, ".srt") == 0) {
        return LCT_LRC;
    }

    return LCT_UNKNOWN;
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

bool LyricsProperties::equal(const LyricsProperties &other) const {
    return artist == other.artist &&
        title == other.title &&
        album == other.album &&
        by == other.by &&
        id == other.id &&
        _strTimeOffset == other._strTimeOffset &&
        mediaLength == other.mediaLength;
}

void LyricsProperties::clear() {
    title.resize(0);
    artist.resize(0);
    album.resize(0);
    by.resize(0);
    _strTimeOffset.resize(0);
    mediaLength.resize(0);
    id.resize(0);
    _timeOffset = 0;
    lyrContentType = LCT_UNKNOWN;
}

void LyricsProperties::setOffsetTime(int nOffsetTime) {
    _timeOffset = nOffsetTime;
    _strTimeOffset = std::to_string(nOffsetTime);
}

void LyricsProperties::setMediaLength(int nMediaLength) {
    if (nMediaLength > 0) {
        char szBuf[64] = { 0 };
        formatPlayTime(nMediaLength * 1000, szBuf);
        mediaLength = szBuf;
    }
}

int LyricsProperties::getMediaLengthInt() const {
    if (mediaLength.size() < 4) {
        return 0;
    }

    //snprintf(szBuf, 32, "%d:%02d:%02d", nHour, nMinute, nSec);
    // snprintf(szBuf, 32, "%02d:%02d", nMinute, nSec);

    int n1, n2, n3;
    cstr_t p = mediaLength.c_str();
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

void LyricsLine::appendPiece(int beginTime, int endTime, const StringView &text) {
    pieces.push_back(LyricsPiece(beginTime, endTime, text));
}

//////////////////////////////////////////////////////////////////////////

void LyricsLines::clear() {
    _lines.clear();
}

void LyricsLines::clearDrawContextWidth() {
    for (auto &line : _lines) {
        for (auto &piece : line.pieces) {
            piece.drawWidth = 0;
        }
    }
}

int LyricsLines::getCountWithSameTimeStamps(int index) {
    if (index < 0 || index >= size()) {
        return 0;
    }

    auto &line = _lines[index];
    assert(line.isLyricsLine);
    auto begTime = line.pieces.front().beginTime;

    for (int i = index + 1; i < size(); i++) {
        auto &line = _lines[i];
        assert(line.isLyricsLine);
        if (begTime != line.pieces.front().beginTime) {
            return i - index;
        }
    }

    return (int)size() - index;
}

void LyricsLines::addLyricsLine(const StringView &text) {
    LyricsLine line(-1, -1);
    line.appendPiece(-1, -1, text);
    push_back(line);
}

void LyricsLines::addNoneLyricsLine(const StringView &text) {
    LyricsLine line(-1, -1);
    line.isLyricsLine = false;
    text.toString(line.content);
    push_back(line);
}

void RawLyrics::analyzeContentType() {
    LyricsContentType ct = LCT_UNKNOWN;

    for (auto &line : _lines) {
        if (line.isLyricsLine) {
            if (line.pieces.size() > 1) {
                ct = LCT_KARAOKE;
                break;
            }

            if (line.pieces[0].beginTime != -1) {
                ct = LCT_LRC;
            } else if (ct < LCT_TXT) {
                ct = LCT_TXT;
            }
        }
    }

    _props.lyrContentType = ct;
}

struct LyricsLineLessThan {
    bool operator()(const LyricsLine &a, const LyricsLine &b) const {
        assert(!a.pieces.empty());
        assert(!b.pieces.empty());
        auto ta = a.beginTime >= 0 ? a.beginTime : a.pieces.front().beginTime;
        auto tb = b.beginTime >= 0 ? b.beginTime : b.pieces.front().beginTime;
        return ta < tb;
    }
};

void averageTimeline(int durationMs, LyricsLines &lyrLinesOut) {
    size_t countChars = 0;

    for (auto &line : lyrLinesOut) {
        assert(line.isLyricsLine);
        for (auto &piece : line.pieces) {
            countChars += piece.text.size();
        }

        countChars++; // 每行多算一个字符(空行也是至少有一个字符)
    }

    double durationOfChar = durationMs / (double)countChars;
    double t = 0;

    for (auto &line : lyrLinesOut) {
        for (auto &piece : line.pieces) {
            piece.beginTime = t;
            t += piece.text.size() * durationOfChar;
        }

        t += durationOfChar;
    }
}

void RawLyrics::clear() {
    _lines.clear();
    _props.clear();
}

LyricsLines RawLyrics::toLyricsLinesOnly(int durationMs) const {
    LyricsLines lyrLinesOut;

    auto ct = _props.lyrContentType;
    if (ct == LCT_TXT) {
        if (!_props.artist.empty() && !_props.title.empty()) {
            // 插入 Artist - title
            LyricsLine line(-1, -1);
            line.appendPiece(-1, -1, _props.artist + " - " + _props.title);
            lyrLinesOut.push_back(line);
        } else if (!_props.title.empty()) {
            LyricsLine line(-1, -1);
            line.appendPiece(-1, -1, _props.title);
            lyrLinesOut.push_back(line);
        }
    }

    for (auto &line : _lines) {
        if (line.isLyricsLine) {
            lyrLinesOut.push_back(line);
        }
    }

    ct = _props.lyrContentType;
    if (ct == LCT_TXT) {
        // 文本歌词，按照文字长度平均分配时间.
        if (durationMs <= 0) {
            durationMs = 60 * 1000 * 5; // 假设歌曲缺省为 5 分钟
        }

        averageTimeline(durationMs, lyrLinesOut);
    } else if (!lyrLinesOut.empty()) {
        // 按照时间顺序排序
        std::stable_sort(lyrLinesOut.begin(), lyrLinesOut.end(), LyricsLineLessThan());

        int maxTime = lyrLinesOut.lines().back().pieces.back().beginTime;
        if (durationMs <= lyrLinesOut.lines().back().pieces.back().beginTime) {
            durationMs = maxTime + 10 * 1000;
        }
    }

    // 修正时间的规则:
    // * 后一行时间不能和前一行的结束时间重叠，如果重叠，则将此行时间压缩.
    for (size_t i = 0; i < lyrLinesOut.size(); i++) {
        auto &line = lyrLinesOut[i];

        // 多行可能有相同的开始时间和结束时间.
        int count = lyrLinesOut.getCountWithSameTimeStamps((int)i);
        int nextBeginTime = durationMs;
        if (i + count < lyrLinesOut.size()) {
            nextBeginTime = lyrLinesOut[i + count].beginTimeOfPiece();
        }

        LyricsPiece *prevPiece = nullptr;
        for (auto &piece : line.pieces) {
            // 所有时间都不能超过下一行的开始时间
            if (piece.beginTime > nextBeginTime) {
                piece.beginTime = nextBeginTime;
            }
            if (piece.endTime > nextBeginTime) {
                piece.endTime = nextBeginTime;
            }

            if (prevPiece && prevPiece->endTime == -1) {
                // 设置结束时间
                prevPiece->endTime = piece.beginTime;
            }
            prevPiece = &piece;
        }
        if (prevPiece->endTime == -1) {
            prevPiece->endTime = nextBeginTime;
        }

        // 设置行的开始和结束时间
        line.beginTime = line.beginTimeOfPiece();
        line.endTime = prevPiece->endTime;
    }

    return lyrLinesOut;
}
