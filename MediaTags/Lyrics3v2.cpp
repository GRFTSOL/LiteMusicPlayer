// ID3v2 standards: https://id3.org/Lyrics3v2

#include "Lyrics3v2.h"
#include "ID3/ID3v1.h"
#include "LrcParser.h"


#define _ID3_V1_LEN         128

void setId3v1(char *szId3v1, const char *szTitle, const char *szArtist, const char * szAlbum, const char * szYear, const char *szComment, uint8_t byTrack, uint8_t byGenre) {
    char *szPtr;

    memset(szId3v1, 0, ID3_V1_LEN);
    memcpy(szId3v1, "TAG", ID3_V1_LEN_ID);

    szPtr = szId3v1 + ID3_V1_LEN_ID;
    if (szTitle) {
        strcpy_safe(szPtr, ID3_V1_LEN_TITLE, szTitle);
    }

    szPtr += ID3_V1_LEN_TITLE;
    if (szArtist) {
        strcpy_safe(szPtr, ID3_V1_LEN_ARTIST, szArtist);
    }

    szPtr += ID3_V1_LEN_ARTIST;
    if (szAlbum) {
        strcpy_safe(szPtr, ID3_V1_LEN_ALBUM, szAlbum);
    }

    szPtr += ID3_V1_LEN_ALBUM;
    if (szYear) {
        strcpy_safe(szPtr, ID3_V1_LEN_YEAR, szYear);
    }

    szPtr += ID3_V1_LEN_YEAR;
    if (szComment) {
        strcpy_safe(szPtr, ID3_V1_LEN_COMMENT, szComment);
    }

    szPtr += ID3_V1_LEN_COMMENT;
    *szPtr = (char)byTrack;

    szPtr += ID3_V1_LEN_TRACK;
    *szPtr = (char)byGenre;
}


Lyrics3v2::Lyrics3v2() {

}

Lyrics3v2::~Lyrics3v2() {

}

#define ERR_GOTO_FAILED(p)  { ret=p; goto FAILED; }

int searchLyrics3v2(FILE *fp, int &lenLyricsTag) {
    char szTemp[256];
    int nId3v1Len = 0;

    lenLyricsTag = 0;

    if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    if (fread(szTemp, 1, 3, fp) != 3) {
        return ERR_READ_FILE;
    }
    if (strncmp(szTemp, "TAG", 3) != 0) {
        // return 0;
        nId3v1Len = 0;
    } else {
        nId3v1Len = _ID3_V1_LEN;
    }

    // 9: LYRICS200
    if (fseek(fp, -(nId3v1Len + 9), SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    // 9: LYRICS200
    if (fread(szTemp, 1, 9, fp) != 9) {
        return ERR_READ_FILE;
    }
    if (strncmp(szTemp, "LYRICS200", 9) != 0) {
        return ERR_NOT_FIND_LRC3V2;
    }

    // 9: LYRICS200
    if (fseek(fp, -(nId3v1Len + 9 + 6), SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    // tag size
    if (fread(szTemp, 1, 6, fp) != 6) {
        return ERR_READ_FILE;
    }
    szTemp[6] = '\0';
    lenLyricsTag = atoi(szTemp);

    // 9: LYRICSBEGIN
    if (fseek(fp, -(nId3v1Len + 9 + 6 + lenLyricsTag), SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    // 9: LYRICSBEGIN
    if (fread(szTemp, 1, 11, fp) != 11) {
        return ERR_READ_FILE;
    }
    if (strncmp(szTemp, "LYRICSBEGIN", 11) != 0) {
        return ERR_BAD_LRC3V2_FORMAT;
    }

    return ERR_OK;
}

bool Lyrics3v2::exists(FILE *fp) const {
    int lenLyricsTag;
    return searchLyrics3v2(fp, lenLyricsTag) == ERR_OK;
}

int Lyrics3v2::readLyrcs(FILE *fp, string &strLyrics) {
    assert(fp);

    char szTemp[256];
    int lenLyricsTag = 0;

    int ret = searchLyrics3v2(fp, lenLyricsTag);
    if (ret != ERR_OK) {
        return ret;
    }

    for (int i = lenLyricsTag - 11; i > 0; ) {
        char fieldName[16];
        if (fread(fieldName, 1, 3, fp) != 3) {
            ERR_GOTO_FAILED(ERR_READ_FILE);
        }
        fieldName[3] = '\0';

        if (fread(szTemp, 1, 5, fp) != 5) {
            ERR_GOTO_FAILED(ERR_READ_FILE);
        }
        szTemp[6] = '\0';

        int nField = atoi(szTemp);
        i -= nField + 3 + 5;

        if (strncmp(fieldName, "IND", 3) == 0) {
            assert(nField < CountOf(szTemp) - 1);
            if (nField >= CountOf(szTemp) - 1) {
                return ERR_BAD_LRC3V2_FORMAT;
            }
            if (fread(szTemp, 1, nField, fp) != nField) {
                ERR_GOTO_FAILED(ERR_READ_FILE);
            }
            szTemp[nField] = '\0';
            // m_strInd = szTemp;
        } else if (strncmp(fieldName, "LYR", 3) == 0) {
            strLyrics.resize(nField);
            if (fread((char *)strLyrics.data(), 1, nField, fp) != nField) {
                ERR_GOTO_FAILED(ERR_READ_FILE);
            }
        } else {
            if (fseek(fp, nField, SEEK_CUR) != 0) {
                ERR_GOTO_FAILED(ERR_SEEK_FILE);
            }
        }
    }

    return ret;

FAILED:
    return ret;
}

int Lyrics3v2::writeLyrcs(FILE *fp, const RawLyrics &rawLyrics) {
    char szTemp[256];
    int nId3v1Len = 0;
    int lenLyricsTag = 0;
    int nOffset = 0;
    char szId3v1Tag[_ID3_V1_LEN + 1];
    int ret = ERR_OK;

    // ID3_V1_LEN = 128
    if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    if (fread(szTemp, 1, 3, fp) != 3) {
        return ERR_READ_FILE;
    }
    if (strncmp(szTemp, "TAG", 3) != 0) {
        //
        // write tag info

        memset(szId3v1Tag, 0, _ID3_V1_LEN);
        memcpy(szId3v1Tag, "TAG", 3);

        auto &props = rawLyrics.properties();

        setId3v1(szId3v1Tag, props.title.c_str(), props.artist.c_str(), props.album.c_str());

        nId3v1Len = 0;
    } else {
        if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0) {
            return ERR_SEEK_FILE;
        }
        if (fread(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN) {
            return ERR_READ_FILE;
        }
        nId3v1Len = _ID3_V1_LEN;
    }

    // 9: LYRICS200
    if (fseek(fp, -(nId3v1Len + 9), SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    // 9: LYRICS200
    if (fread(szTemp, 1, 9, fp) != 9) {
        return ERR_READ_FILE;
    }
    nOffset = nId3v1Len;
    if (strncmp(szTemp, "LYRICS200", 9) == 0) {
        if (fseek(fp, -(nId3v1Len + 9 + 6), SEEK_END) != 0) {
            return ERR_SEEK_FILE;
        }

        // tag size
        if (fread(szTemp, 1, 6, fp) != 6) {
            return ERR_READ_FILE;
        }
        szTemp[6] = '\0';
        lenLyricsTag = atoi(szTemp);

        nOffset += 9 + 6 + lenLyricsTag;

        // 9: LYRICSBEGIN
        if (fseek(fp, -nOffset, SEEK_END) != 0) {
            return ERR_SEEK_FILE;
        }

        // 9: LYRICSBEGIN
        if (fread(szTemp, 1, 11, fp) != 11) {
            return ERR_READ_FILE;
        }
        if (strncmp(szTemp, "LYRICSBEGIN", 11) != 0) {
            return ERR_BAD_LRC3V2_FORMAT;
        }
    }

    if (fseek(fp, -nOffset, SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    lenLyricsTag = 0;
    if (fwrite("LYRICSBEGIN", 1, 11, fp) != 11) {
        return ERR_WRITE_FILE;
    }
    lenLyricsTag += 11;

    if (fwrite("IND00003", 1, 8, fp) != 8) {
        return ERR_WRITE_FILE;
    }
    lenLyricsTag += 8;

    szTemp[0] = '1';
    // 1 for LRC, 0 for text
    szTemp[1] = rawLyrics.properties().lyrContentType >= LCT_LRC ? '1' : '0';
    szTemp[2] = '0';
    if (fwrite(szTemp, 1, 3, fp) != 3) {
        return ERR_WRITE_FILE;
    }
    lenLyricsTag += 3;

    if (fwrite("LYR", 1, 3, fp) != 3) {
        return ERR_WRITE_FILE;
    }
    lenLyricsTag += 3;

    string lyrics = toLyricsBinary(rawLyrics);
    snprintf(szTemp, CountOf(szTemp), "%05d", (int)lyrics.size());
    if (fwrite(szTemp, 1, 5, fp) != 5) {
        return ERR_WRITE_FILE;
    }
    lenLyricsTag += 5;

    if (fwrite(lyrics.c_str(), 1, lyrics.size(), fp) != lyrics.size()) {
        return ERR_WRITE_FILE;
    }
    lenLyricsTag += lyrics.size();

    snprintf(szTemp, CountOf(szTemp), "%06d", lenLyricsTag);
    if (fwrite(szTemp, 1, 6, fp) != 6) {
        return ERR_WRITE_FILE;
    }

    if (fwrite("LYRICS200", 1, 9, fp) != 9) {
        return ERR_WRITE_FILE;
    }

    if (fwrite(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN) {
        return ERR_WRITE_FILE;
    }

    filetruncate(fp, ftell(fp));

    return ret;
}

int Lyrics3v2::clearLyricsInfo(FILE *fp) {
    char szTemp[256];
    int nId3v1Len = 0;
    int lenLyricsTag = 0;
    int nOffset = 0;
    char szId3v1Tag[_ID3_V1_LEN + 1];
    int ret;

    // ID3_V1_LEN = 128
    if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0) {
        ERR_GOTO_FAILED(ERR_SEEK_FILE);
    }

    if (fread(szTemp, 1, 3, fp) != 3) {
        ERR_GOTO_FAILED(ERR_READ_FILE);
    }
    if (strncmp(szTemp, "TAG", 3) != 0) {
        nId3v1Len = 0;
    } else {
        if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0) {
            ERR_GOTO_FAILED(ERR_SEEK_FILE);
        }
        if (fread(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN) {
            ERR_GOTO_FAILED(ERR_READ_FILE);
        }
        nId3v1Len = _ID3_V1_LEN;
    }

    // 9: LYRICS200
    if (fseek(fp, -(nId3v1Len + 9), SEEK_END) != 0) {
        ERR_GOTO_FAILED(ERR_SEEK_FILE);
    }

    // 9: LYRICS200
    if (fread(szTemp, 1, 9, fp) != 9) {
        ERR_GOTO_FAILED(ERR_READ_FILE);
    }
    nOffset = nId3v1Len;
    if (strncmp(szTemp, "LYRICS200", 9) == 0) {
        if (fseek(fp, -(nId3v1Len + 9 + 6), SEEK_END) != 0) {
            ERR_GOTO_FAILED(ERR_SEEK_FILE);
        }

        // tag size
        if (fread(szTemp, 1, 6, fp) != 6) {
            ERR_GOTO_FAILED(ERR_READ_FILE);
        }
        szTemp[6] = '\0';
        lenLyricsTag = atoi(szTemp);

        nOffset += 9 + 6 + lenLyricsTag;

        // 9: LYRICSBEGIN
        if (fseek(fp, -nOffset, SEEK_END) != 0) {
            ERR_GOTO_FAILED(ERR_SEEK_FILE);
        }

        // 9: LYRICSBEGIN
        if (fread(szTemp, 1, 11, fp) != 11) {
            ERR_GOTO_FAILED(ERR_READ_FILE);
        }
        if (strncmp(szTemp, "LYRICSBEGIN", 11) != 0) {
            ERR_GOTO_FAILED(ERR_NOT_FIND_LRC3V2);
        }
    } else {
        ERR_GOTO_FAILED(ERR_NOT_FIND_LRC3V2);
    }

    if (fseek(fp, -nOffset, SEEK_END) != 0) {
        ERR_GOTO_FAILED(ERR_SEEK_FILE);
    }

    if (nId3v1Len) {
        if (fwrite(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN) {
            ERR_GOTO_FAILED(ERR_WRITE_FILE);
        }
    }

    filetruncate(fp, ftell(fp));

    return ERR_OK;

FAILED:
    return ret;
}
