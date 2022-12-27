#ifndef MediaTags_ID3_ID3v1_h
#define MediaTags_ID3_ID3v1_h

#pragma once

#include "../../Utils/Utils.h"


enum ID3_V1Lengths {
    ID3_V1_LEN                  = 128,
    ID3_V1_LEN_ID               =   3,
    ID3_V1_LEN_TITLE            =  30,
    ID3_V1_LEN_ARTIST           =  30,
    ID3_V1_LEN_ALBUM            =  30,
    ID3_V1_LEN_YEAR             =   4,
    ID3_V1_LEN_COMMENT          =  29,
    ID3_V1_LEN_TRACK            =   1,
    ID3_V1_LEN_GENRE            =   1
};

struct ID3V1 {
    char                        szTitle[ID3_V1_LEN_TITLE + 1];
    char                        szArtist[ID3_V1_LEN_ARTIST + 1];
    char                        szAlbum[ID3_V1_LEN_ALBUM + 1];
    char                        szYear[ID3_V1_LEN_YEAR + 1];
    char                        szComment[ID3_V1_LEN_COMMENT + 1];
    uint8_t                     byTrack;
    uint8_t                     byGenre;
};

class CID3v1 {
public:
    CID3v1();
    virtual ~CID3v1();

#pragma pack(push)
#pragma pack(1)
    struct ID3v1Data {
        char                        szID3ID[ID3_V1_LEN_ID];
        char                        szTitle[ID3_V1_LEN_TITLE];
        char                        szArtist[ID3_V1_LEN_ARTIST];
        char                        szAlbum[ID3_V1_LEN_ALBUM];
        char                        szYear[ID3_V1_LEN_YEAR];
        char                        szComment[ID3_V1_LEN_COMMENT];
        uint8_t                     byTrack;
        uint8_t                     byGenre;
    };
#pragma pack(pop)

    static cstr_t *getSupportedExtArray();

    int getTag(cstr_t szFile, ID3V1 *id3v1);
    int getTag(FILE *fp, ID3V1 *id3v1);

    int saveTag(cstr_t szFile, ID3V1 *id3v1);
    int saveTag(FILE *fp, ID3V1 *id3v1);

    static cstr_t* GetAllGenreDescription();
    static unsigned int GetGenreCount();
    static cstr_t getGenreDescription(unsigned int nGenre);
    static int getGenreIndex(cstr_t szGenre);

};

#endif // !defined(MediaTags_ID3_ID3v1_h)
