#include "ID3v1.h"


#define ID3_NR_OF_V1_GENRES 148

const char *ID3_v1_genre_description[ID3_NR_OF_V1_GENRES] = {
  "Blues",             //0
  "Classic Rock",      //1
  "Country",           //2
  "Dance",             //3
  "Disco",             //4
  "Funk",              //5
  "Grunge",            //6
  "Hip-Hop",           //7
  "Jazz",              //8
  "Metal",             //9
  "new Age",           //10
  "Oldies",            //11
  "Other",             //12
  "Pop",               //13
  "R&B",               //14
  "Rap",               //15
  "Reggae",            //16
  "Rock",              //17
  "Techno",            //18
  "Industrial",        //19
  "Alternative",       //20
  "Ska",               //21
  "Death Metal",       //22
  "Pranks",            //23
  "Soundtrack",        //24
  "Euro-Techno",       //25
  "Ambient",           //26
  "Trip-Hop",          //27
  "Vocal",             //28
  "Jazz+Funk",         //29
  "Fusion",            //30
  "Trance",            //31
  "Classical",         //32
  "Instrumental",      //33
  "Acid",              //34
  "House",             //35
  "Game",              //36
  "Sound Clip",        //37
  "Gospel",            //38
  "Noise",             //39
  "AlternRock",        //40
  "Bass",              //41
  "Soul",              //42
  "Punk",              //43
  "Space",             //44
  "Meditative",        //45
  "Instrumental Pop",  //46
  "Instrumental Rock", //47
  "Ethnic",            //48
  "Gothic",            //49
  "Darkwave",          //50
  "Techno-Industrial", //51
  "Electronic",        //52
  "Pop-Folk",          //53
  "Eurodance",         //54
  "Dream",             //55
  "Southern Rock",     //56
  "Comedy",            //57
  "Cult",              //58
  "Gangsta",           //59
  "Top 40",            //60
  "Christian Rap",     //61
  "Pop/Funk",          //62
  "Jungle",            //63
  "Native American",   //64
  "Cabaret",           //65
  "new Wave",          //66
  "Psychadelic",       //67
  "Rave",              //68
  "Showtunes",         //69
  "Trailer",           //70
  "Lo-Fi",             //71
  "Tribal",            //72
  "Acid Punk",         //73
  "Acid Jazz",         //74
  "Polka",             //75
  "Retro",             //76
  "Musical",           //77
  "Rock & Roll",       //78
  "Hard Rock",         //79
    // following are winamp extentions
  "Folk",                  //80
  "Folk-Rock",             //81
  "National Folk",         //82
  "Swing",                 //83
  "Fast Fusion",           //84
  "Bebob",                 //85
  "Latin",                 //86
  "Revival",               //87
  "Celtic",                //88
  "Bluegrass",             //89
  "Avantgarde",            //90
  "Gothic Rock",           //91
  "Progressive Rock",      //92
  "Psychedelic Rock",      //93
  "Symphonic Rock",        //94
  "Slow Rock",             //95
  "Big Band",              //96
  "Chorus",                //97
  "Easy Listening",        //98
  "Acoustic",              //99
  "Humour",                //100
  "Speech",                //101
  "Chanson",               //102
  "Opera",                 //103
  "Chamber Music",         //104
  "Sonata",                //105
  "Symphony",              //106
  "Booty Bass",            //107
  "Primus",                //108
  "Porn Groove",           //109
  "Satire",                //110
  "Slow Jam",              //111
  "Club",                  //112
  "Tango",                 //113
  "Samba",                 //114
  "Folklore",              //115
  "Ballad",                //116
  "Power Ballad",          //117
  "Rhythmic Soul",         //118
  "Freestyle",             //119
  "Duet",                  //120
  "Punk Rock",             //121
  "Drum Solo",             //122
  "A capella",             //123
  "Euro-House",            //124
  "Dance Hall",            //125
  "Goa",                   //126
  "Drum & Bass",           //127
  "Club-House",            //128
  "Hardcore",              //129
  "Terror",                //130
  "Indie",                 //131
  "Britpop",               //132
  "Negerpunk",             //133
  "Polsk Punk",            //134
  "Beat",                  //135
  "Christian Gangsta Rap", //136
  "Heavy Metal",           //137
  "Black Metal",           //138
  "Crossover",             //139
  "Contemporary Christian",//140
  "Christian Rock ",       //141
  "Merengue",              //142
  "Salsa",                 //143
  "Trash Metal",           //144
  "Anime",                 //145
  "JPop",                  //146
  "Synthpop"               //147
};

//
// 去掉字符串两端的空格
void trimStrA(char *szString) {
    char * szBeg;
    szBeg = szString;

    while (*szBeg == ' ') {
        szBeg++;
    }

    auto nLen = strlen(szBeg);
    if (nLen > 0) {
        while (nLen > 0 && szBeg[nLen - 1] == ' ') {
            nLen--;
        }
        szBeg[nLen] = '\0';
    }

    if (szBeg != szString) {
        memmove(szString, szBeg, sizeof(char) * (nLen + 1));
    }
}



CID3v1::CID3v1() {

}

CID3v1::~CID3v1() {

}

cstr_t *CID3v1::getSupportedExtArray() {
    static cstr_t arr[] = { ".mp3", ".mp2", nullptr };
    return arr;
}

int CID3v1::getTag(cstr_t szFile, ID3V1 *id3v1) {
    FILE *fp;
    int nRet;

    fp = fopen(szFile, "rb");
    if (!fp) {
        return ERR_OPEN_FILE;
    }

    nRet = getTag(fp, id3v1);
    fclose(fp);

    return nRet;
}

int CID3v1::getTag(FILE *fp, ID3V1 *id3v1) {
    memset(id3v1, 0, sizeof(ID3V1));

    if (fseek(fp, -ID3_V1_LEN, SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    char szID3v1[ID3_V1_LEN];
    if (fread(szID3v1, 1, ID3_V1_LEN, fp) != ID3_V1_LEN) {
        return ERR_READ_FILE;
    }

    if (memcmp(szID3v1, "TAG", ID3_V1_LEN_ID) != 0) {
        return ERR_NOT_FOUND;
    }

    char *szPtr;

    szPtr = szID3v1 + ID3_V1_LEN_ID;
    memcpy(id3v1->szTitle, szPtr, ID3_V1_LEN_TITLE);
    trimStrA(id3v1->szTitle);

    szPtr += ID3_V1_LEN_TITLE;
    memcpy(id3v1->szArtist, szPtr, ID3_V1_LEN_ARTIST);
    trimStrA(id3v1->szArtist);

    szPtr += ID3_V1_LEN_ARTIST;
    memcpy(id3v1->szAlbum, szPtr, ID3_V1_LEN_ALBUM);
    trimStrA(id3v1->szAlbum);

    szPtr += ID3_V1_LEN_ALBUM;
    memcpy(id3v1->szYear, szPtr, ID3_V1_LEN_YEAR);
    trimStrA(id3v1->szYear);

    szPtr += ID3_V1_LEN_YEAR;
    memcpy(id3v1->szComment, szPtr, ID3_V1_LEN_COMMENT);
    trimStrA(id3v1->szComment);

    szPtr += ID3_V1_LEN_COMMENT;
    id3v1->byTrack = (uint8_t)*szPtr;

    szPtr += ID3_V1_LEN_TRACK;
    id3v1->byGenre = (uint8_t)*szPtr;

    return ERR_OK;
}

int CID3v1::saveTag(cstr_t szFile, ID3V1 *id3v1) {
    FILE *fp;
    int nRet;

    fp = fopen(szFile, "r+b");
    if (!fp) {
        return ERR_OPEN_FILE;
    }

    nRet = saveTag(fp, id3v1);
    fclose(fp);

    return nRet;
}

static int min_fun(int a, int b) {
    return a <= b ? a : b;
}

int CID3v1::saveTag(FILE *fp, ID3V1 *id3v1) {
    if (fseek(fp, 0, SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    if (ftell(fp) >= sizeof(ID3v1Data)) {
        if (fseek(fp, -ID3_V1_LEN, SEEK_END) != 0) {
            return ERR_SEEK_FILE;
        }

        char szID3v1[ID3_V1_LEN];
        if (fread(szID3v1, 1, ID3_V1_LEN, fp) != ID3_V1_LEN) {
            return ERR_READ_FILE;
        }

        if (memcmp(szID3v1, "TAG", ID3_V1_LEN_ID) != 0) {
            fseek(fp, 0, SEEK_END);
        } else {
            fseek(fp, -ID3_V1_LEN, SEEK_END);
        }
    }

    ID3v1Data data;

    memset(&data, 0, sizeof(data));
    memcpy(data.szID3ID, "TAG", 3);
    memcpy(data.szTitle, id3v1->szTitle, min_fun((int)strlen(id3v1->szTitle), CountOf(data.szTitle)));
    memcpy(data.szArtist, id3v1->szArtist, min_fun((int)strlen(id3v1->szArtist), CountOf(data.szArtist)));
    memcpy(data.szAlbum, id3v1->szAlbum, min_fun((int)strlen(id3v1->szAlbum), CountOf(data.szAlbum)));
    memcpy(data.szYear, id3v1->szYear, min_fun((int)strlen(id3v1->szYear), CountOf(data.szYear)));
    memcpy(data.szComment, id3v1->szComment, min_fun((int)strlen(id3v1->szComment), CountOf(data.szComment)));
    data.byTrack = id3v1->byTrack;
    data.byGenre = id3v1->byGenre;

    if (fwrite(&data, 1, sizeof(ID3v1Data), fp) != sizeof(ID3v1Data)) {
        return ERR_WRITE_FILE;
    }

    return ERR_OK;
}

cstr_t* CID3v1::GetAllGenreDescription() {
    return ID3_v1_genre_description;
}

unsigned int CID3v1::GetGenreCount() {
    return CountOf(ID3_v1_genre_description);
}

cstr_t CID3v1::getGenreDescription(unsigned int nGenre) {
    if (nGenre < CountOf(ID3_v1_genre_description)) {
        return ID3_v1_genre_description[nGenre];
    } else {
        return "";
    }
}

int CID3v1::getGenreIndex(cstr_t szGenre) {
    for (int i = 0; i < CountOf(ID3_v1_genre_description); i++) {
        if (strcmp(ID3_v1_genre_description[i], szGenre) == 0) {
            return i;
        }
    }

    return -1;
}
