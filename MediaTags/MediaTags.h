#pragma once

#include "ID3/ID3v1.h"
#include "ID3v2IF.h"
#include "M4aTag.h"


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

#define SZ_SONG_ID3V2_SYLT  "song://id3v2/sylt/"
#define SZ_SONG_ID3V2_USLT  "song://id3v2/uslt/"
#define SZ_SONG_LYRICS3V2   "song://lyrics3v2"
#define SZ_SONG_ID3V2_LYRICS    "song://id3v2/lyrics(nonstandard)   "
#define SZ_SONG_M4A_LYRICS                                          "song://m4a/lyrics"
#define SZ_SONG_KAR_LYRICS                                          "song://kar/lyrics"

#define NONE_LYRCS          ""

#define SZ_SONG_ID3V2_SYLT_DESC    _TLM("Embedded lyrics for Windows Media Player (ID3v2 synchronized lyrics)   ")
#define SZ_SONG_ID3V2_USLT_DESC    _TLM("Embedded lyrics for iPod, iTunes (ID3v2 unsynchronized lyrics)         ")
#define SZ_SONG_LYRICS3V2_DESC    _TLM("Embedded lyrics for MiniLyrics (Lyrics3v2 synchronized lyrics)          ")
#define SZ_SONG_ID3V2_LYRICS_DESC    _TLM("Embedded ID3v2 lyrics (Nonstandard)                                  ")
#define SZ_SONG_M4A_LYRICS_DESC    _TLM("Embedded lyrics for iPod, iTunes (For &AAC (M4A)                       file)")
#define SZ_SONG_KAR_LYRICS_DESC    _TLM("Embedded lyrics for Karaoke (KAR file)                                 ")
#define SZ_SONG_NO_LYRICS_DESC    _TLM("No suitable lyrics for the song file")

enum TagType {
    TT_ID3V1,
    TT_ID3V2,
    TT_APE,
    TT_M4A,
};

class TagFields {
public:
    string                      name;
    string                      strValue;

};
typedef list<TagFields> ListTagFields;

class CTag {
public:
    ListTagFields               m_listFields;
    TagType                     m_tagType;
    bool                        m_bRemovable;       // Can the tag be removed
    bool                        m_bEmpty;           // Is this an empty tag?

};

class CListTags: public list<CTag *> {
public:
    CListTags();
    virtual ~CListTags();

};


class MediaTags {
public:
    MediaTags(void);
    virtual ~MediaTags(void);

public:
    // pMediaLength: in ms
    static int getTagFast(cstr_t szFile, string *pArtist, string *pTitle, string *pAlbum,
        string *pYear, string *pGenre, string *pTrackNo, uint32_t *pMediaLength);

    static int getAllTags(cstr_t szFile, CListTags &listTags);

    static bool isMediaTypeSupported(cstr_t szFile);

    static int getEmbeddedLyrics(cstr_t szFile, VecStrings &vLyricsNames);

    static bool isEmbeddedLyricsSupported(cstr_t szFile);
    static bool isID3v2TagSupported(cstr_t szFile);
    static bool isM4aTagSupported(cstr_t szFile);
    static bool isKarTagSupported(cstr_t szFile);

    static int removeEmbeddedLyrics(cstr_t szMediaFile, VecStrings &vLyrNamesToRemove, int *succeededCount = nullptr);

};
