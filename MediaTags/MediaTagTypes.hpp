//
//  MediaTagTypes.hpp
//  Taglib
//
//  Created by henry_xiao on 2023/1/1.
//

#ifndef MediaTagTypes_hpp
#define MediaTagTypes_hpp

#include "../Utils/Utils.h"


enum LRC_SOURCE_TYPE {
    LST_NONE                    = 0,
    LST_UNKNOWN                 = 0x1 << 1,
    LST_FILE                    = 0x1 << 2,
    LST_ID3V2_SYLT              = 0x1 << 3,
    LST_ID3V2_USLT              = 0x1 << 4,
    LST_LYRICS3V2               = 0x1 << 5,
    LST_ID3V2_LYRICS            = 0x1 << 6,
    LST_M4A_LYRICS              = 0x1 << 7, // Embedded lyrics in M4A file.
    LST_KAR_LYRICS              = 0x1 << 8, // Embedded lyrics in KAR file.

    LST_ID3V2                   = LST_ID3V2_SYLT | LST_ID3V2_USLT | LST_ID3V2_LYRICS,
};

LRC_SOURCE_TYPE lyrSrcTypeFromName(cstr_t szLrcSource);
cstr_t lyrSrcTypeToName(LRC_SOURCE_TYPE lst);
cstr_t lyrSrcTypeToDesc(LRC_SOURCE_TYPE lst);

bool getEmbeddedLyricsNameInfo(cstr_t szName, string &language, int &index);

#define SZ_SONG_ID3V2_SYLT      "song://id3v2/sylt/"
#define SZ_SONG_ID3V2_USLT      "song://id3v2/uslt/"
#define SZ_SONG_LYRICS3V2       "song://lyrics3v2"
#define SZ_SONG_ID3V2_LYRICS    "song://id3v2/lyrics(nonstandard)"
#define SZ_SONG_M4A_LYRICS      "song://m4a/lyrics"
#define SZ_SONG_KAR_LYRICS      "song://kar/lyrics"

#define NONE_LYRCS              ""

#define SZ_SONG_ID3V2_SYLT_DESC     _TLM("Embedded lyrics for Windows Media Player (ID3v2 synchronized lyrics)")
#define SZ_SONG_ID3V2_USLT_DESC     _TLM("Embedded lyrics for iPod, iTunes (ID3v2 unsynchronized lyrics)")
#define SZ_SONG_LYRICS3V2_DESC      _TLM("Embedded lyrics for MiniLyrics (Lyrics3v2 synchronized lyrics)")
#define SZ_SONG_ID3V2_LYRICS_DESC   _TLM("Embedded ID3v2 lyrics (Nonstandard)")
#define SZ_SONG_M4A_LYRICS_DESC     _TLM("Embedded lyrics for iPod, iTunes (For &AAC (M4A) file)")
#define SZ_SONG_KAR_LYRICS_DESC     _TLM("Embedded lyrics for Karaoke (KAR file)")
#define SZ_SONG_NO_LYRICS_DESC      _TLM("No suitable lyrics for the song file")

enum TagType {
    TT_ID3V1,
    TT_ID3V2,
    TT_APE,
    TT_M4A,
};

struct BasicMediaTags {
    string                      artist, title, album, year, genre, trackNo, comments;
    uint32_t                    mediaLength;

    BasicMediaTags() { mediaLength = 0; }
};

struct ExtendedMediaInfo {
    bool                        isValid = false;
    int                         bitRate = 0;
    uint8_t                     channels = 0;
    uint8_t                     bitsPerSample = 0;
    uint32_t                    sampleRate = 0;
};

#endif /* MediaTagTypes_hpp */
