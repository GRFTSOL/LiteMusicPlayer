#pragma once

#include "LyricsData.h"


class Lyrics3v2 {
public:
    Lyrics3v2();
    virtual ~Lyrics3v2();

public:
    bool exists(FILE *fp) const;

    int readLyrcs(FILE *fp, string &strLyrics);
    int writeLyrcs(FILE *fp, const RawLyrics &rawLyrics);

    int clearLyricsInfo(FILE *fp);

};

void setId3v1(char *szId3v1, const char *szTitle, const char *szArtist, const char * szAlbum, const char * szYear = nullptr, const char *szComment = nullptr, uint8_t byTrack = 0, uint8_t byGenre = 0xFF);
