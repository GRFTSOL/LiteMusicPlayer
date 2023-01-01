#include "ID3v1.h"


const char *ID3_v1_genre_description[] = {
    "Blues",                    // 00
    "Classic Rock",             // 01
    "Country",                  // 02
    "Dance",                    // 03
    "Disco",                    // 04
    "Funk",                     // 05
    "Grunge",                   // 06
    "Hip-Hop",                  // 07
    "Jazz",                     // 08
    "Metal",                    // 09
    "New Age",                  // 10
    "Oldies",                   // 11
    "Other",                    // 12
    "Pop",                      // 13
    "Rhythm and Blues",         // 14
    "Rap",                      // 15
    "Reggae",                   // 16
    "Rock",                     // 17
    "Techno",                   // 18
    "Industrial",               // 19
    "Alternative",              // 20
    "Ska",                      // 21
    "Death Metal",              // 22
    "Pranks",                   // 23
    "Soundtrack",               // 24
    "Euro-Techno",              // 25
    "Ambient",                  // 26
    "Trip-Hop",                 // 27
    "Vocal",                    // 28
    "Jazz & Funk",              // 29
    "Fusion",                   // 30
    "Trance",                   // 31
    "Classical",                // 32
    "Instrumental",             // 33
    "Acid",                     // 34
    "House",                    // 35
    "Game",                     // 36
    "Sound clip",               // 37
    "Gospel",                   // 38
    "Noise",                    // 39
    "Alternative Rock",         // 40
    "Bass",                     // 41
    "Soul",                     // 42
    "Punk",                     // 43
    "Space",                    // 44
    "Meditative",               // 45
    "Instrumental Pop",         // 46
    "Instrumental Rock",        // 47
    "Ethnic",                   // 48
    "Gothic",                   // 49
    "Darkwave",                 // 50
    "Techno-Industrial",        // 51
    "Electronic",               // 52
    "Pop-Folk",                 // 53
    "Eurodance",                // 54
    "Dream",                    // 55
    "Southern Rock",            // 56
    "Comedy",                   // 57
    "Cult",                     // 58
    "Gangsta",                  // 59
    "Top 40",                   // 60
    "Christian Rap",            // 61
    "Pop/Funk",                 // 62
    "Jungle music",             // 63
    "Native US",                // 64
    "Cabaret",                  // 65
    "New Wave",                 // 66
    "Psychedelic",              // 67
    "Rave",                     // 68
    "Showtunes",                // 69
    "Trailer",                  // 70
    "Lo-Fi",                    // 71
    "Tribal",                   // 72
    "Acid Punk",                // 73
    "Acid Jazz",                // 74
    "Polka",                    // 75
    "Retro",                    // 76
    "Musical",                  // 77
    "Rock ’n’ Roll",            // 78
    "Hard Rock",                // 79
    "Folk",                     // 80
    "Folk-Rock",                // 81
    "National Folk",            // 82
    "Swing",                    // 83
    "Fast Fusion",              // 84
    "Bebop",                    // 85
    "Latin",                    // 86
    "Revival",                  // 87
    "Celtic",                   // 88
    "Bluegrass",                // 89
    "Avantgarde",               // 90
    "Gothic Rock",              // 91
    "Progressive Rock",         // 92
    "Psychedelic Rock",         // 93
    "Symphonic Rock",           // 94
    "Slow Rock",                // 95
    "Big Band",                 // 96
    "Chorus",                   // 97
    "Easy Listening",           // 98
    "Acoustic",                 // 99
    "Humour",                   // 100
    "Speech",                   // 101
    "Chanson",                  // 102
    "Opera",                    // 103
    "Chamber Music",            // 104
    "Sonata",                   // 105
    "Symphony",                 // 106
    "Booty Bass",               // 107
    "Primus",                   // 108
    "Porn Groove",              // 109
    "Satire",                   // 110
    "Slow Jam",                 // 111
    "Club",                     // 112
    "Tango",                    // 113
    "Samba",                    // 114
    "Folklore",                 // 115
    "Ballad",                   // 116
    "Power Ballad",             // 117
    "Rhythmic Soul",            // 118
    "Freestyle",                // 119
    "Duet",                     // 120
    "Punk Rock",                // 121
    "Drum Solo",                // 122
    "A cappella",               // 123
    "Euro-House",               // 124
    "Dance Hall",               // 125
    "Goa music",                // 126
    "Drum & Bass",              // 127
    "Club-House",               // 128
    "Hardcore Techno",          // 129
    "Terror",                   // 130
    "Indie",                    // 131
    "BritPop",                  // 132
    "Negerpunk",                // 133
    "Polsk Punk",               // 134
    "Beat",                     // 135
    "Christian Gangsta Rap",    // 136
    "Heavy Metal",              // 137
    "Black Metal",              // 138
    "Crossover",                // 139
    "Contemporary Christian",   // 140
    "Christian Rock",           // 141
    "Merengue",                 // 142
    "Salsa",                    // 143
    "Thrash Metal",             // 144
    "Anime",                    // 145
    "Jpop",                     // 146
    "Synthpop",                 // 147
    "Christmas",                // 148
    "Art Rock",                 // 149
    "Baroque",                  // 150
    "Bhangra",                  // 151
    "Big beat",                 // 152
    "Breakbeat",                // 153
    "Chillout",                 // 154
    "Downtempo",                // 155
    "Dub",                      // 156
    "EBM",                      // 157
    "Eclectic",                 // 158
    "Electro",                  // 159
    "Electroclash",             // 160
    "Emo",                      // 161
    "Experimental",             // 162
    "Garage",                   // 163
    "Global",                   // 164
    "IDM",                      // 165
    "Illbient",                 // 166
    "Industro-Goth",            // 167
    "Jam Band",                 // 168
    "Krautrock",                // 169
    "Leftfield",                // 170
    "Lounge",                   // 171
    "Math Rock",                // 172
    "New Romantic",             // 173
    "Nu-Breakz",                // 174
    "Post-Punk",                // 175
    "Post-Rock",                // 176
    "Psytrance",                // 177
    "Shoegaze",                 // 178
    "Space Rock",               // 179
    "Trop Rock",                // 180
    "World Music",              // 181
    "Neoclassical",             // 182
    "Audiobook",                // 183
    "Audio Theatre",            // 184
    "Neue Deutsche Welle",      // 185
    "Podcast",                  // 186
    "Indie-Rock",               // 187
    "G-Funk",                   // 188
    "Dubstep",                  // 189
    "Garage Rock",              // 190
    "Psybient",                 // 191
};

//
// 去掉字符串两端的空格
static char *trimTagValue(char *str) {
    char *p = str;

    while (*p == ' ') {
        p++;
    }

    auto len = strlen(p);
    if (len > 0) {
        while (len > 0 && p[len - 1] == ' ') {
            len--;
        }
        p[len] = '\0';
    }

    return p;
}



CID3v1::CID3v1() {

}

CID3v1::~CID3v1() {

}

cstr_t *CID3v1::getSupportedExtArray() {
    static cstr_t arr[] = { ".mp3", ".mp2", nullptr };
    return arr;
}

int CID3v1::getTag(cstr_t szFile, BasicMediaTags &tags) {
    FILE *fp;
    int nRet;

    fp = fopen(szFile, "rb");
    if (!fp) {
        return ERR_OPEN_FILE;
    }

    nRet = getTag(fp, tags);
    fclose(fp);

    return nRet;
}

static string readStringTag(char *&p, int size) {
    char buf[256];

    memcpy(buf, p, size);
    p += size;

    return trimTagValue(buf);
}

int CID3v1::getTag(FILE *fp, BasicMediaTags &tags) {
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

    char *p = szID3v1 + ID3_V1_LEN_ID;
    tags.title = readStringTag(p, ID3_V1_LEN_TITLE);
    tags.artist = readStringTag(p, ID3_V1_LEN_ARTIST);
    tags.album = readStringTag(p, ID3_V1_LEN_ALBUM);
    tags.year = readStringTag(p, ID3_V1_LEN_YEAR);
    tags.comments = readStringTag(p, ID3_V1_LEN_COMMENT);

    auto track = (uint8_t)*p; p++;
    auto genre = (uint8_t)*p;

    tags.trackNo = itos(track);
    tags.genre = getGenreDescription(genre);

    return ERR_OK;
}

int CID3v1::saveTag(cstr_t szFile, const BasicMediaTags &tags) {
    FILE *fp;
    int nRet;

    fp = fopen(szFile, "r+b");
    if (!fp) {
        return ERR_OPEN_FILE;
    }

    nRet = saveTag(fp, tags);
    fclose(fp);

    return nRet;
}

static int min_fun(int a, int b) {
    return a <= b ? a : b;
}

static void copyTagValue(char *buf, int size, const string &value) {
    if ((int)value.size() < size) {
        size = (int)value.size();
    }

    memcpy(buf, value.c_str(), size);
}

int CID3v1::saveTag(FILE *fp, const BasicMediaTags &tags) {
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
    memcpy(data.id3ID, "TAG", 3);
    copyTagValue(data.title, CountOf(data.title), tags.title);
    copyTagValue(data.artist, CountOf(data.artist), tags.artist);
    copyTagValue(data.album, CountOf(data.album), tags.album);
    copyTagValue(data.year, CountOf(data.year), tags.year);
    copyTagValue(data.comments, CountOf(data.comments), tags.comments);

    data.track = atoi(tags.trackNo.c_str());
    data.genre = getGenreIndex(tags.genre.c_str());

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
