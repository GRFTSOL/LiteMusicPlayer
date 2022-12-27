#pragma once

class CLyrics3v2 {
public:
    CLyrics3v2();
    virtual ~CLyrics3v2();

public:
    int readLyrcsInfo(cstr_t szMp3File, string &strLyrics);
    int writeLyrcsInfo(cstr_t szMp3File, const char *szLyrics, int nLen, bool bTimeStamp = true);

    int clearLyricsInfo(cstr_t szMp3File);

};

void setId3v1(char *szId3v1, const char *szTitle, const char *szArtist, const char * szAlbum, const char * szYear = nullptr, const char *szComment = nullptr, uint8_t byTrack = 0, uint8_t byGenre = 0xFF);
