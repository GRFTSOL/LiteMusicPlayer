#pragma once

#include "../MediaTags/MediaTags.h"

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

    int calMatchValueByName(cstr_t szLyricsDir, cstr_t szLyricsFileName, LyricsContentType lct);

    int calMatchValueByTitle(cstr_t szLyrArtist, cstr_t szLyrTitle, LyricsContentType lct);

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

class ListLyrSearchResults : public list<LrcSearchResult> {
public:
    bool addResult(const LrcSearchResult &result);
    iterator getTheBestMatchLyrics(bool bCheckExist = false);

    bool isAllTxtLyrics() const;

    void updateLyricsName();

    LrcSearchResult *searchByUrl(cstr_t szUrl);

    LrcSearchResult *getByOrder(int n);
    void deleteByOrder(int n);

};

void searchMatchLyricsInDir(cstr_t szDir, CLyricsSearchParameter &searchParam, ListLyrSearchResults &vLyrics, bool bIncludeSub);

void searchEmbeddedLyrics(cstr_t szSongFile, ListLyrSearchResults &vLyrics, bool bOnlyListAvailable = false);
