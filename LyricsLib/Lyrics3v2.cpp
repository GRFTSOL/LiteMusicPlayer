// Lyrics3v2.cpp: implementation of the CLyrics3v2 class.
//
//////////////////////////////////////////////////////////////////////

// ID3v2 standards: https://id3.org/Lyrics3v2

#include "MLLib.h"
#include "Lyrics3v2.h"
#include "../MediaTags/ID3/ID3v1.h"
#include "LrcTag.h"

#define _ID3_V1_LEN        128

void setId3v1(char *szId3v1, const char *szTitle, const char *szArtist, const char * szAlbum, const char * szYear, const char *szComment, uint8_t byTrack, uint8_t byGenre)
{
    char    *szPtr;

    memset(szId3v1, 0, ID3_V1_LEN);
    memcpy(szId3v1, "TAG", ID3_V1_LEN_ID);

    szPtr = szId3v1 + ID3_V1_LEN_ID;
    if (szTitle)
        strcpy_safe(szPtr, ID3_V1_LEN_TITLE, szTitle);

    szPtr += ID3_V1_LEN_TITLE;
    if (szArtist)
        strcpy_safe(szPtr, ID3_V1_LEN_ARTIST, szArtist);

    szPtr += ID3_V1_LEN_ARTIST;
    if (szAlbum)
        strcpy_safe(szPtr, ID3_V1_LEN_ALBUM, szAlbum);

    szPtr += ID3_V1_LEN_ALBUM;
    if (szYear)
        strcpy_safe(szPtr, ID3_V1_LEN_YEAR, szYear);

    szPtr += ID3_V1_LEN_YEAR;
    if (szComment)
        strcpy_safe(szPtr, ID3_V1_LEN_COMMENT, szComment);

    szPtr += ID3_V1_LEN_COMMENT;
    *szPtr = (char)byTrack;

    szPtr += ID3_V1_LEN_TRACK;
    *szPtr = (char)byGenre;
}


CLyrics3v2::CLyrics3v2()
{

}

CLyrics3v2::~CLyrics3v2()
{

}

#define ERR_GOTO_FAILED(p)        { nErr=p; goto FAILED; }

int CLyrics3v2::readLyrcsInfo(cstr_t szMp3File, string &strLyrics)
{
    FILE        *fp = nullptr;
    char        szTemp[256];
    int            nId3v1Len = 0;
    int            nField;
    int            nLyricsTagLen = 0;
    int            i;
    int            nErr = ERR_OK;

    fp = fopen(szMp3File, "rb");
    if (!fp)
        ERR_GOTO_FAILED(ERR_OPEN_FILE);

    // ID3_V1_LEN = 128
    if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    if (fread(szTemp, 1, 3, fp) != 3)
        ERR_GOTO_FAILED(ERR_READ_FILE);
    if (strncmp(szTemp, "TAG", 3) != 0)
    {
        // return 0;
        nId3v1Len = 0;
    }
    else
        nId3v1Len = _ID3_V1_LEN;

    // 9: LYRICS200
    if (fseek(fp, -(nId3v1Len + 9), SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    // 9: LYRICS200
    if (fread(szTemp, 1, 9, fp) != 9)
        ERR_GOTO_FAILED(ERR_READ_FILE);
    if (strncmp(szTemp, "LYRICS200", 9) != 0)
        ERR_GOTO_FAILED(ERR_NOT_FIND_LRC3V2);

    // 9: LYRICS200
    if (fseek(fp, -(nId3v1Len + 9 + 6), SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    // tag size
    if (fread(szTemp, 1, 6, fp) != 6)
        ERR_GOTO_FAILED(ERR_READ_FILE);
    szTemp[6] = '\0';
    nLyricsTagLen = atoi(szTemp);

    // 9: LYRICSBEGIN
    if (fseek(fp, -(nId3v1Len + 9 + 6 + nLyricsTagLen), SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    // 9: LYRICSBEGIN
    if (fread(szTemp, 1, 11, fp) != 11)
        ERR_GOTO_FAILED(ERR_READ_FILE);
    if (strncmp(szTemp, "LYRICSBEGIN", 11) != 0)
        ERR_GOTO_FAILED(ERR_BAD_LRC3V2_FORMAT);

    for (i = nLyricsTagLen - 11; i > 0; )
    {
        char        szFiledName[16];
        if (fread(szFiledName, 1, 3, fp) != 3)
            ERR_GOTO_FAILED(ERR_READ_FILE);
        szFiledName[3] = '\0';

        if (fread(szTemp, 1, 5, fp) != 5)
            ERR_GOTO_FAILED(ERR_READ_FILE);
        szTemp[6] = '\0';

        nField = atoi(szTemp);
        i -= nField + 3 + 5;

        if (strncmp(szFiledName, "IND", 3) == 0)
        {
            assert(nField < CountOf(szTemp) - 1);
            if (nField >= CountOf(szTemp) - 1)
                return ERR_BAD_LRC3V2_FORMAT;
            if (fread(szTemp, 1, nField, fp) != nField)
                ERR_GOTO_FAILED(ERR_READ_FILE);
            szTemp[nField] = '\0';
            // m_strInd = szTemp;
        }
        else if (strncmp(szFiledName, "LYR", 3) == 0)
        {
            strLyrics.resize(nField);
            if (fread((char *)strLyrics.data(), 1, nField, fp) != nField) {
                strLyrics.resize(strLyrics.size() - nField);
                ERR_GOTO_FAILED(ERR_READ_FILE);
            }
        }
        else
        {
            if (fseek(fp, nField, SEEK_CUR) != 0)
                ERR_GOTO_FAILED(ERR_SEEK_FILE);
        }
    }

    fclose(fp);

    return nErr;

FAILED:
    if (fp)
        fclose(fp);

    return nErr;
}

int CLyrics3v2::writeLyrcsInfo(cstr_t szMp3File, const char *szLyrics, int nLen, bool bTimeStamp)
{
    char        szTemp[256];
    int            nId3v1Len = 0;
    int            nLyricsTagLen = 0;
    int            nOffset = 0;
    char        szId3v1Tag[_ID3_V1_LEN + 1];
    int            nErr = ERR_OK;

    FILE *fp = fopen(szMp3File, "r+b");
    if (!fp)
    {
        setCustomErrorDesc(OSError().Description());
        ERR_GOTO_FAILED(ERR_CUSTOM_ERROR);
    }

    // ID3_V1_LEN = 128
    if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    if (fread(szTemp, 1, 3, fp) != 3)
        ERR_GOTO_FAILED(ERR_READ_FILE);
    if (strncmp(szTemp, "TAG", 3) != 0)
    {
        //
        // write tag info

        memset(szId3v1Tag, 0, _ID3_V1_LEN);
        memcpy(szId3v1Tag, "TAG", 3);

        //
        // 先从歌词标签中取得信息
        //
        CLrcTag        lrcTag;

        lrcTag.readFromText((uint8_t *)szLyrics, nLen);

        if (lrcTag.m_strArtist.empty())
            analyseLyricsFileNameEx(lrcTag.m_strArtist, lrcTag.m_strTitle, szMp3File);

        setId3v1(szId3v1Tag, lrcTag.m_strTitle.c_str(), lrcTag.m_strArtist.c_str(), lrcTag.m_strAlbum.c_str());

        nId3v1Len = 0;
    }
    else
    {
        if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
            ERR_GOTO_FAILED(ERR_SEEK_FILE);
        if (fread(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN)
            ERR_GOTO_FAILED(ERR_READ_FILE);
        nId3v1Len = _ID3_V1_LEN;
    }

    // 9: LYRICS200
    if (fseek(fp, -(nId3v1Len + 9), SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    // 9: LYRICS200
    if (fread(szTemp, 1, 9, fp) != 9)
        ERR_GOTO_FAILED(ERR_READ_FILE);
    nOffset = nId3v1Len;
    if (strncmp(szTemp, "LYRICS200", 9) == 0)
    {
        if (fseek(fp, -(nId3v1Len + 9 + 6), SEEK_END) != 0)
            ERR_GOTO_FAILED(ERR_SEEK_FILE);

        // tag size
        if (fread(szTemp, 1, 6, fp) != 6)
            ERR_GOTO_FAILED(ERR_READ_FILE);
        szTemp[6] = '\0';
        nLyricsTagLen = atoi(szTemp);

        nOffset += 9 + 6 + nLyricsTagLen;

        // 9: LYRICSBEGIN
        if (fseek(fp, -nOffset, SEEK_END) != 0)
            ERR_GOTO_FAILED(ERR_SEEK_FILE);

        // 9: LYRICSBEGIN
        if (fread(szTemp, 1, 11, fp) != 11)
            ERR_GOTO_FAILED(ERR_READ_FILE);
        if (strncmp(szTemp, "LYRICSBEGIN", 11) != 0)
            ERR_GOTO_FAILED(ERR_BAD_LRC3V2_FORMAT);
    }

    if (fseek(fp, -nOffset, SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    nLyricsTagLen = 0;
    if (fwrite("LYRICSBEGIN", 1, 11, fp) != 11)
        ERR_GOTO_FAILED(ERR_WRITE_FILE);
    nLyricsTagLen += 11;

    if (fwrite("IND00003", 1, 8, fp) != 8)
        ERR_GOTO_FAILED(ERR_WRITE_FILE);
    nLyricsTagLen += 8;

    if (bTimeStamp)
        szTemp[1] = '1';
    szTemp[0] = '1';
    szTemp[2] = '0';
    if (fwrite(szTemp, 1, 3, fp) != 3)
        ERR_GOTO_FAILED(ERR_WRITE_FILE);
    nLyricsTagLen += 3;

    if (fwrite("LYR", 1, 3, fp) != 3)
        ERR_GOTO_FAILED(ERR_WRITE_FILE);
    nLyricsTagLen += 3;

    if (nLen == -1)
        nLen = strlen(szLyrics);
    snprintf(szTemp, CountOf(szTemp), "%05d", nLen);
    if (fwrite(szTemp, 1, 5, fp) != 5)
        ERR_GOTO_FAILED(ERR_WRITE_FILE);
    nLyricsTagLen += 5;

    if (fwrite(szLyrics, 1, nLen, fp) != nLen)
        ERR_GOTO_FAILED(ERR_WRITE_FILE);
    nLyricsTagLen += nLen;

    snprintf(szTemp, CountOf(szTemp), "%06d", nLyricsTagLen);
    if (fwrite(szTemp, 1, 6, fp) != 6)
        ERR_GOTO_FAILED(ERR_WRITE_FILE);

    if (fwrite("LYRICS200", 1, 9, fp) != 9)
        ERR_GOTO_FAILED(ERR_WRITE_FILE);

    if (fwrite(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN)
        ERR_GOTO_FAILED(ERR_WRITE_FILE);

    filetruncate(fp, ftell(fp));

    fclose(fp);

    return nErr;

FAILED:
    if (fp)
        fclose(fp);

    return nErr;
}

int CLyrics3v2::clearLyricsInfo(cstr_t szMp3File)
{
    FILE        *fp = nullptr;
    char        szTemp[256];
    int            nId3v1Len = 0;
    int            nLyricsTagLen = 0;
    int            nOffset = 0;
    char        szId3v1Tag[_ID3_V1_LEN + 1];
    int            nErr;

    fp = fopen(szMp3File, "r+b");
    if (!fp)
        ERR_GOTO_FAILED(ERR_OPEN_FILE);

    // ID3_V1_LEN = 128
    if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    if (fread(szTemp, 1, 3, fp) != 3)
        ERR_GOTO_FAILED(ERR_READ_FILE);
    if (strncmp(szTemp, "TAG", 3) != 0)
    {
        nId3v1Len = 0;
    }
    else
    {
        if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
            ERR_GOTO_FAILED(ERR_SEEK_FILE);
        if (fread(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN)
            ERR_GOTO_FAILED(ERR_READ_FILE);
        nId3v1Len = _ID3_V1_LEN;
    }

    // 9: LYRICS200
    if (fseek(fp, -(nId3v1Len + 9), SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    // 9: LYRICS200
    if (fread(szTemp, 1, 9, fp) != 9)
        ERR_GOTO_FAILED(ERR_READ_FILE);
    nOffset = nId3v1Len;
    if (strncmp(szTemp, "LYRICS200", 9) == 0)
    {
        if (fseek(fp, -(nId3v1Len + 9 + 6), SEEK_END) != 0)
            ERR_GOTO_FAILED(ERR_SEEK_FILE);

        // tag size
        if (fread(szTemp, 1, 6, fp) != 6)
            ERR_GOTO_FAILED(ERR_READ_FILE);
        szTemp[6] = '\0';
        nLyricsTagLen = atoi(szTemp);

        nOffset += 9 + 6 + nLyricsTagLen;

        // 9: LYRICSBEGIN
        if (fseek(fp, -nOffset, SEEK_END) != 0)
            ERR_GOTO_FAILED(ERR_SEEK_FILE);

        // 9: LYRICSBEGIN
        if (fread(szTemp, 1, 11, fp) != 11)
            ERR_GOTO_FAILED(ERR_READ_FILE);
        if (strncmp(szTemp, "LYRICSBEGIN", 11) != 0)
            ERR_GOTO_FAILED(ERR_NOT_FIND_LRC3V2);
    }
    else
        ERR_GOTO_FAILED(ERR_NOT_FIND_LRC3V2);

    if (fseek(fp, -nOffset, SEEK_END) != 0)
        ERR_GOTO_FAILED(ERR_SEEK_FILE);

    if (nId3v1Len)
    {
        if (fwrite(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN)
            ERR_GOTO_FAILED(ERR_WRITE_FILE);
    }

    filetruncate(fp, ftell(fp));

    fclose(fp);

    return ERR_OK;

FAILED:
    if (fp)
        fclose(fp);

    return nErr;
}
