#ifndef MediaTags_ID3_ID3v1_h
#define MediaTags_ID3_ID3v1_h

#pragma once

#include "../MediaTagTypes.hpp"


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

class CID3v1 {
public:
    CID3v1();
    virtual ~CID3v1();

#pragma pack(push)
#pragma pack(1)
    struct ID3v1Data {
        char                        id3ID[ID3_V1_LEN_ID];
        char                        title[ID3_V1_LEN_TITLE];
        char                        artist[ID3_V1_LEN_ARTIST];
        char                        album[ID3_V1_LEN_ALBUM];
        char                        year[ID3_V1_LEN_YEAR];
        char                        comments[ID3_V1_LEN_COMMENT];
        uint8_t                     track;
        uint8_t                     genre;
    };
#pragma pack(pop)

    bool hasTag(FILE *fp);

    int getTag(cstr_t szFile, BasicMediaTags &tags);
    int getTag(FILE *fp, BasicMediaTags &tags);

    int saveTag(cstr_t szFile, const BasicMediaTags &tags);
    int saveTag(FILE *fp, const BasicMediaTags &tags);

    static cstr_t* GetAllGenreDescription();
    static unsigned int GetGenreCount();
    static cstr_t getGenreDescription(unsigned int nGenre);
    static int getGenreIndex(cstr_t szGenre);

};

#endif // !defined(MediaTags_ID3_ID3v1_h)
