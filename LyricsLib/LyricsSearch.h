#pragma once

#include "../MLProtocol/MLUtils.h"


#ifndef _IPHONE
#define _ID3V2_SUPPORT
#endif

#ifdef _ID3V2_SUPPORT
#include "Lyrics3v2.h"
#include "../MediaTags/MediaTags.h"


#endif // #ifdef _ID3V2_SUPPORT

#define MATCH_VALUE_MIN     30
#define MATCH_VALUE_OK      60
#define MATCH_VALUE_BETTER  70
#define MATCH_VALUE_MAX     100
#define MATCH_VALUE_EMBEDDED_LRC 95

class CLyricsSearchParameter {
public:
    cstr_t                      szSongFile, szArtist, szTitle;

    string                      strArtistFiltered, strTitleFiltered;
    string                      strSongTitle;
    string                      strSongDir;

    bool                        bOnlySearchBestMatch;
    int                         nMatchValueOfBest;
    string                      strBestMatchLyrics;

    // Used for cache memory alloc
    string                      strLyrArFiltered, strLyrTiFiltered;

public:
    CLyricsSearchParameter(cstr_t szSongFile, cstr_t szArtist, cstr_t szTitle, bool bOnlySearchBestMatch = false);

    int calMatchValueByName(cstr_t szLyricsDir, cstr_t szLyricsFileName, MLFileType fileType);

    int calMatchValueByTitle(cstr_t szLyrArtist, cstr_t szLyrTitle, MLFileType fileType);

    bool isBestMatchLyricsFound();

};

struct LrcSearchResult {
    string                      strUrl;
    string                      strSaveFileName;
    string                      strArtist;
    string                      strTitle;
    string                      strAlbum;
    string                      strUploader;
    float                       nMatchValue;
    float                       fRate;              // rate info of lyrics
    int                         nRateCount;         //
    int                         nDownloads;         // total downloads of lyrics

    LrcSearchResult() {
        nMatchValue = 0;
        fRate = 0.0;
        nRateCount = 0;
        nDownloads = 0;
    }
};
bool operator<=(const LrcSearchResult &l1, const LrcSearchResult &l2);
bool operator<(const LrcSearchResult &l1, const LrcSearchResult &l2);

class V_LRCSEARCHRESULT : public list<LrcSearchResult> {
public:
    bool addResult(const LrcSearchResult &Result);
    iterator getTheBestMatchLyrics(bool bCheckExist = false);

    bool isAllTxtLyrics() const;

    void updateLyricsName();

    LrcSearchResult *searchByUrl(cstr_t szUrl);

    LrcSearchResult *getByOrder(int n);
    void deleteByOrder(int n);

};

void searchMatchLyricsInDir(cstr_t szDir, CLyricsSearchParameter &searchParam, V_LRCSEARCHRESULT &vLyrics, bool bIncludeSub);

int searchEmbeddedLyrics(cstr_t szSongFile, V_LRCSEARCHRESULT &vLyrics, bool bOnlyListAvailable = false);
int searchEmbeddedLyrics(cstr_t szSongFile, uint32_t &lrcSourceType);
