#include "Media.h"


Media::Media() : IMediaInfo() {
    timeAdded = 0;
}

Media::~Media() {
}

bool Media::isEqual(const Media *other) const {
    return  ID == other->ID &&
            url == other->url &&
            artist == other->artist &&
            album == other->album &&
            title == other->title &&
            trackNumb == other->trackNumb &&
            year == other->year &&
            genre == other->genre &&
            comments == other->comments &&
            duration == other->duration &&
            fileSize == other->fileSize &&
            timeAdded == other->timeAdded &&
            timePlayed == other->timePlayed &&
            rating == other->rating &&
            isFileDeleted == other->isFileDeleted &&
            countPlayed == other->countPlayed &&
            lyricsFile == other->lyricsFile &&
            musicHash == other->musicHash &&
            format == other->format &&
            bitRate == other->bitRate &&
            channels == other->channels &&
            bitsPerSample == other->bitsPerSample &&
            sampleRate == other->sampleRate;
}

void Media::setAttribute(MediaAttribute mediaAttr, cstr_t szValue) {
    switch (mediaAttr) {
        case MA_ARTIST : artist = szValue; break;
        case MA_ALBUM : album = szValue; break;
        case MA_TITLE : title = szValue; break;
        case MA_TRACK_NUMB : trackNumb = atoi(szValue); break;
        case MA_YEAR : year = atoi(szValue); break;
        case MA_GENRE : genre = szValue; break;
        case MA_COMMENT : comments = szValue; break;
        case MA_BITRATE : bitRate = atoi(szValue); break;
        case MA_DURATION : duration = atoi(szValue); break;
        case MA_FILESIZE : fileSize = atoi(szValue); break;
        case MA_RATING : rating = atoi(szValue); break;
        case MA_TIMES_PLAYED: countPlayed = atoi(szValue); break;
        case MA_LYRICS_FILE : lyricsFile = szValue; break;

        // the following info will not stored in media library
        case MA_FORMAT : format = szValue; break;
        case MA_BITS_PER_SAMPLE : bitsPerSample = atoi(szValue); break;
        case MA_CHANNELS : channels = atoi(szValue); break;
        case MA_SAMPLE_RATE : sampleRate = atoi(szValue); break;
    default:
        assert(0);
        break;
    }
}

void Media::setAttribute(MediaAttribute mediaAttr, int64_t value) {
    switch (mediaAttr) {
        case MA_TRACK_NUMB : trackNumb = value; break;
        case MA_YEAR : year = value; break;
        case MA_BITRATE : bitRate = (int)value; break;
        case MA_DURATION : duration = (int)value; break;
        case MA_FILESIZE : fileSize = (int)value; break;
        case MA_TIME_ADDED : timeAdded = value; break;
        case MA_TIME_PLAYED : timePlayed = value; break;
        case MA_RATING : rating = value; break;
        case MA_TIMES_PLAYED: countPlayed = (int)value; break;

        // the following info will not stored in media library
        case MA_BITS_PER_SAMPLE : bitsPerSample = (uint8_t)value; break;
        case MA_CHANNELS : channels = (uint8_t)value; break;
        case MA_SAMPLE_RATE : sampleRate = (int)value; break;
    default:
        assert(0);
        break;
    }
}

void long2XStr(int value, string &str) {
    str.reserve(32);
    int size = snprintf(str.data(), str.capacity(), "%d", value);
    str.resize(size);
}

void time2XStr(time_t time, string &str) {
    if (time == 0) {
        str.assign("-");
    } else {
        DateTime date((int64_t)time);
        str.assign(date.toDateTimeString().c_str());
    }
}

void Media::getAttribute(MediaAttribute mediaAttr, string &value) {
    value.clear();

    switch (mediaAttr) {
        case MA_ARTIST : value.assign(artist.c_str()); break;
        case MA_ALBUM : value.assign(album.c_str()); break;
        case MA_TITLE : value.assign(title.c_str()); break;
        case MA_TRACK_NUMB : if (trackNumb != -1) long2XStr(trackNumb, value); break;
        case MA_YEAR : if (year != -1) long2XStr(year, value); break;
        case MA_GENRE : value.assign(genre.c_str()); break;
        case MA_COMMENT : value.assign(comments.c_str()); break;
        case MA_BITRATE : long2XStr(bitRate, value); break;
        case MA_DURATION : long2XStr(duration, value); break;
        case MA_FILESIZE : long2XStr((int)fileSize, value); break;
        case MA_TIME_ADDED : time2XStr(timeAdded, value); break;
        case MA_TIME_PLAYED : time2XStr(timePlayed, value); break;
        case MA_RATING : long2XStr(rating, value); break;
        case MA_TIMES_PLAYED: long2XStr(countPlayed, value); break;
        case MA_LYRICS_FILE : value.assign(lyricsFile); break;

        // the following info will not stored in media library
        case MA_FORMAT : value.assign(format.c_str()); break;
        case MA_BITS_PER_SAMPLE : long2XStr(bitsPerSample, value); break;
        case MA_CHANNELS : long2XStr(channels, value); break;
        case MA_SAMPLE_RATE : long2XStr(sampleRate, value); break;
    default:
        assert(0);
        break;
    }
}
