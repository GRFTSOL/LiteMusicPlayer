#include "LyrTimestamps.h"


const char *getBase64Set();

namespace LyrTimestamps {

#define HEADER_TIME_STAMP_V0            '@'
#define CHAR_NO_TIME_STAMP              '_'
#define CHAR_MAX_TIME_VALUE             '^'
#define CHAR_NEGATIVE_TIME_VALUE        '-'
#define TIME_STAMP_UNIT                 100
#define MAX_DURATION                    64

bool isTimeStamps(cstr_t timestamps) {
    return timestamps[0] == HEADER_TIME_STAMP_V0;
}

void encodeTimeStamps(int duration, string &timeStamps) {
    const char *charSet = getBase64Set();

    if (duration < 0) {
        timeStamps += CHAR_NEGATIVE_TIME_VALUE;
        duration = -duration;
    }

    // duration can't be too large.
    if (duration > 10 * 60 * 1000) {
        duration = 6 * 1000;
    }

    duration /= TIME_STAMP_UNIT;

    while (duration >= 0) {
        if (duration >= MAX_DURATION) {
            timeStamps += CHAR_MAX_TIME_VALUE;
        } else {
            timeStamps += charSet[duration];
        }

        duration -= MAX_DURATION;
    }
}

cstr_t decodeTimeStamps(cstr_t timestamps, int &duration) {
    assert(*timestamps != CHAR_NO_TIME_STAMP);

    duration = 0;

    bool isNegative = (*timestamps == CHAR_NEGATIVE_TIME_VALUE);
    if (isNegative) {
        timestamps++;
    }

    while (*timestamps == CHAR_MAX_TIME_VALUE) {
        duration += MAX_DURATION;
        timestamps++;
    }

    const char *charSet = getBase64Set();
    int i = 0;
    for (i = 0; i < MAX_DURATION; i++) {
        if (charSet[i] == *timestamps) {
            duration += i;
            break;
        }
    }
    assert(i < MAX_DURATION);

    duration *= TIME_STAMP_UNIT;

    if (isNegative) {
        duration = -duration;
    }

    return timestamps + 1;
}

bool parse(cstr_t timestamps, RawLyrics &rawLyrics) {
    // Header
    if (timestamps[0] != HEADER_TIME_STAMP_V0) {
        return false;
    }
    timestamps++;

    // offset time
    int timeOffset = 0;
    timestamps = decodeTimeStamps(timestamps, timeOffset);

    // Time of every line
    int timeAboveLine = 0;
    for (auto &line : rawLyrics) {
        auto &piece = line.pieces[0];
        assert(line.isLyricsLine && piece.beginTime == -1);

        if (*timestamps == CHAR_NO_TIME_STAMP) {
            timestamps++;
            // Just remove this line from RawLyrics (will be kept in m_arrFileLines).
            line.isLyricsLine = false;
            line.content = piece.text;
            line.pieces.clear();
        } else {
            int duration = 0;
            timestamps = decodeTimeStamps(timestamps, duration);
            timeAboveLine = piece.beginTime = line.beginTime = timeAboveLine + duration;
        }
    }

    rawLyrics.properties().setOffsetTime(timeOffset);
    rawLyrics.properties().lyrContentType = LCT_LRC;

    return true;
}

string toString(const RawLyrics &rawLyrics) {
    string strTimeStamps;

    // Header
    strTimeStamps.clear();
    strTimeStamps += HEADER_TIME_STAMP_V0;

    // offset time
    encodeTimeStamps(rawLyrics.properties().getOffsetTime(), strTimeStamps);

    // Time of every line
    int timeAboveLine = 0;
    for (auto &line :rawLyrics) {
        if (line.isLyricsLine) {
            int duration = line.beginTime - timeAboveLine + TIME_STAMP_UNIT / 2;
            encodeTimeStamps(duration, strTimeStamps);
            // Round the above line.
            timeAboveLine += duration / TIME_STAMP_UNIT * TIME_STAMP_UNIT;
        } else {
            strTimeStamps += CHAR_NO_TIME_STAMP;
        }
    }

    return strTimeStamps;
}

} // namespace LyrTimestamps
