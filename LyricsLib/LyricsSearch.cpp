#include "LyricsSearch.h"
#include "MLData.h"
#include "HelperFun.h"
#include "LyricsKeywordFilter.h"


//////////////////////////////////////////////////////////////////////////
// CLyricsSearchParameter
CLyricsSearchParameter::CLyricsSearchParameter(cstr_t szSongFile, cstr_t szArtist, cstr_t szTitle, bool bOnlySearchBestMatch) {
    this->szSongFile = szSongFile;
    this->szArtist = szArtist;
    this->szTitle = szTitle;
    this->bOnlySearchBestMatch = bOnlySearchBestMatch;
    this->nMatchValueOfBest = 0;

    CLyricsKeywordFilter::filter(szArtist, strArtistFiltered);
    CLyricsKeywordFilter::filter(szTitle, strTitleFiltered);

    if (!isEmptyString(szSongFile)) {
        strSongTitle = fileGetTitle(szSongFile);
        strSongDir = fileGetPath(szSongFile);
    }
}

//                           TXT,                 LRC                     SRT
const int __vArTiSame[] = { MATCH_VALUE_OK, MATCH_VALUE_OK + 20, MATCH_VALUE_OK + 10 };
const int __vTiSame[] = { MATCH_VALUE_OK - 30, MATCH_VALUE_OK - 10, MATCH_VALUE_OK - 20 };

inline int fileTypeToMatchValueCol(MLFileType fileType) {
    int col = 0;
    if (fileType == FT_LYRICS_LRC) {
        col = 1;
    } else if (fileType == FT_SUBTITLE_SRT) {
        col = 2;
    }

    return col;
}

int CLyricsSearchParameter::calMatchValueByName(cstr_t szLyricsDir, cstr_t szLyricsFileName, MLFileType fileType) {
    assert(endsWith(szLyricsDir, PATH_SEP_STR));

    bool isFileTitleSame = false;
    bool isDirSame = false;

    string strLyrArtist, strLyrTitle;
    analyseLyricsFileNameEx(strLyrArtist, strLyrTitle, szLyricsFileName);

    // Calculate match by artist and title.
    int matchValue = calMatchValueByTitle(strLyrArtist.c_str(), strLyrTitle.c_str(), fileType);

    // Is file name same with song file name?
    if (strSongTitle.size() > 0) {
        isFileTitleSame = strIsISame(fileGetTitle(szSongFile).c_str(), fileGetTitle(szLyricsFileName).c_str());
    }

    // Continue ?
    if (!isFileTitleSame && matchValue == 0) {
        return 0;
    }

    // Is file dir same with song file name?
    if (strSongDir.size() > 0) {
        isDirSame = strIsISame(strSongDir.c_str(), szLyricsDir);
    }

    if (matchValue > 0) {
        // Title same.
        if (isFileTitleSame) matchValue += 1;
        if (isDirSame) matchValue += 1;
    }

    if (isFileTitleSame && matchValue < MATCH_VALUE_OK) {
        // File name same.
        matchValue = __vArTiSame[fileTypeToMatchValueCol(fileType)];
        if (isDirSame) {
            matchValue += 2;
        }
    }

    return matchValue;
}

int CLyricsSearchParameter::calMatchValueByTitle(cstr_t szLyrArtist, cstr_t szLyrTitle, MLFileType fileType) {
    bool isTitleSame = false;
    bool isArtistSame = false;

    // Is Title same?
    CLyricsKeywordFilter::filter(szLyrTitle, strLyrTiFiltered);
    isTitleSame = strIsSame(strLyrTiFiltered.c_str(), strTitleFiltered.c_str());

    // Continue ?
    if (!isTitleSame) {
        return 0;
    }

    // Is Artist same?
    CLyricsKeywordFilter::filter(szLyrArtist, strLyrArFiltered);
    isArtistSame = strIsSame(strLyrArFiltered.c_str(), strArtistFiltered.c_str());

    int matchValue;
    if (isArtistSame) {
        matchValue = __vArTiSame[fileTypeToMatchValueCol(fileType)];
    } else {
        matchValue = __vTiSame[fileTypeToMatchValueCol(fileType)];
    }

    bool isTitleAllSame = strIsISame(szTitle, szLyrTitle);
    bool isArtistAllSame = strIsISame(szArtist, szLyrArtist);

    if (isTitleAllSame) matchValue += 1;
    if (isArtistAllSame) matchValue += 1;

    return matchValue;
}

bool CLyricsSearchParameter::isBestMatchLyricsFound() {
    return nMatchValueOfBest >= MATCH_VALUE_BETTER;
}

//////////////////////////////////////////////////////////////////////

bool operator<=(const LrcSearchResult &l1, const LrcSearchResult &l2) {
    if (l1.nMatchValue > l2.nMatchValue) {
        return true;
    } else if (l1.nMatchValue == l2.nMatchValue) {
        if (strcasecmp(l1.strArtist.c_str(), l2.strArtist.c_str()) >= 0) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool operator<(const LrcSearchResult &l1, const LrcSearchResult &l2) {
    return l1.nMatchValue > l2.nMatchValue;
}

//////////////////////////////////////////////////////////////////////
// ListLyrSearchResults
//////////////////////////////////////////////////////////////////////
bool ListLyrSearchResults::addResult(const LrcSearchResult &result) {
    for (iterator it = begin(); it != end(); it++) {
        if (strcasecmp((*it).strUrl.c_str(), result.strUrl.c_str()) == 0) {
            return true;
        }
    }

    push_back(result);
    return true;
}

bool ListLyrSearchResults::isAllTxtLyrics() const {
    for (const_iterator it = begin(); it != end(); ++it) {
        const LrcSearchResult &result = *it;
        if (!fileIsExtSame(result.strUrl.c_str(), ".txt")) {
            return false;
        }
    }
    return true;
}

ListLyrSearchResults::iterator ListLyrSearchResults::getTheBestMatchLyrics(bool bCheckExist) {
    float nValueMax = 0;
    iterator itMax = begin();
    for (iterator it = begin(); it != end(); it++) {
        LrcSearchResult &result = *it;
        if (bCheckExist && lyrSrcTypeFromName(result.strUrl.c_str()) == LST_FILE) {
            if (!isFileExist(result.strUrl.c_str())) {
                continue;
            }
        }

        if (result.nMatchValue == MATCH_VALUE_MAX) {
            return it;
        } else if (result.nMatchValue > nValueMax) {
            nValueMax = result.nMatchValue;
            itMax = it;
        }
    }

    return itMax;
}

void ListLyrSearchResults::updateLyricsName() {
    for (iterator it = begin(); it != end(); it++) {
        LrcSearchResult &result = *it;

        if (result.strSaveFileName.empty()) {
            result.strSaveFileName = fileGetName(result.strUrl.c_str());
        }
    }
}

LrcSearchResult *ListLyrSearchResults::searchByUrl(cstr_t szUrl) {
    for (iterator it = begin(); it != end(); it++) {
        LrcSearchResult &result = *it;

        if (strcasecmp(result.strUrl.c_str(), szUrl) == 0) {
            return &(*it);
        }
    }
    return nullptr;
}

LrcSearchResult *ListLyrSearchResults::getByOrder(int n) {
    for (iterator it = begin(); it != end(); it++, n--) {
        if (n == 0) {
            return &(*it);
        }
    }
    return nullptr;
}

void ListLyrSearchResults::deleteByOrder(int n) {
    for (iterator it = begin(); it != end(); it++, n--) {
        if (n == 0) {
            erase(it);
            break;
        }
    }
}

bool isSupportedOpenFileType(MLFileType fileType) {
    return fileType != FT_UNKNOWN && isFlagSet(FT_LYRICS_TXT | FT_SUBTITLE_SRT | FT_LYRICS_LRC, fileType);
}

//
// Return it, if:
// * Name is same as song file name.
// * Title & artist are match.
//
void searchMatchLyricsInDir(cstr_t szDir, CLyricsSearchParameter &searchParam, ListLyrSearchResults &vLyrics, bool bIncludeSub) {
    if (isEmptyString(szDir)) {
        return;
    }

    FileFind finder;
    if (!finder.openDir(szDir)) {
        ERR_LOG1("Failed to open dir: %s", szDir);
        return;
    }

    while (finder.findNext()) {
        if (finder.isCurDir()) {
            if (bIncludeSub) {
                string subDir = dirStringJoin(szDir, finder.getCurName());
                dirStringAddSep(subDir);
                searchMatchLyricsInDir(subDir.c_str(), searchParam, vLyrics, bIncludeSub);
                if (searchParam.isBestMatchLyricsFound()) {
                    return;
                }
            }
        } else {
            MLFileType fileType = GetLyricsFileType(finder.getCurName());
            if (!isSupportedOpenFileType(fileType)) {
                continue;
            }

            int nMatchValue = searchParam.calMatchValueByName(szDir, finder.getCurName(), fileType);
            if (nMatchValue > MATCH_VALUE_MIN) {
                if (searchParam.bOnlySearchBestMatch) {
                    if (nMatchValue > searchParam.nMatchValueOfBest) {
                        searchParam.strBestMatchLyrics = dirStringJoin(szDir, finder.getCurName());
                        searchParam.nMatchValueOfBest = nMatchValue;
                        if (searchParam.isBestMatchLyricsFound()) {
                            return;
                        }
                    }
                    continue;
                }

                LrcSearchResult LrcResult;
                LrcResult.nMatchValue = (float)nMatchValue;
                LrcResult.strUrl = dirStringJoin(szDir, finder.getCurName());
                LrcResult.strSaveFileName = fileGetTitle(finder.getCurName());
                vLyrics.addResult(LrcResult);
            }
        }
    }
}

string removePrefixOfAcckey(cstr_t szStr);

int searchEmbeddedLyrics(cstr_t szSongFile, ListLyrSearchResults &vLyrics, bool bOnlyListAvailable) {
    VecStrings vLyricsNames;
    int nRet = MediaTags::getEmbeddedLyrics(szSongFile, vLyricsNames);
    if (nRet != ERR_OK) {
        return nRet;
    }

    for (uint32_t i = 0; i < vLyricsNames.size(); i++) {
        string &lyrName = vLyricsNames[i];
        LRC_SOURCE_TYPE lst = lyrSrcTypeFromName(lyrName.c_str());

        LrcSearchResult result;
        result.strUrl = lyrName;
        result.strTitle = result.strSaveFileName = removePrefixOfAcckey(_TL(lyrSrcTypeToDesc(lst)));
        if (lst == LST_LYRICS3V2 || lst == LST_ID3V2_SYLT || lst == LST_ID3V2_LYRICS) {
            result.nMatchValue = MATCH_VALUE_EMBEDDED_LRC;
        } else {
            result.nMatchValue = MATCH_VALUE_BETTER - 4; // Larger than the text lyrics.
        }

        string language;
        int index;
        if (getEmbeddedLyricsNameInfo(lyrName.c_str(), language, index)) {
            string strIndex;
            if (index > 0) {
                strIndex = stringPrintf(": %d", index).c_str();
            }
            if (language == "eng") {
                result.strArtist = "English";
            } else if (language == "fre") {
                result.strArtist = "French";
            } else {
                result.strArtist = language;
            }
            result.strArtist += strIndex;
        }

        vLyrics.addResult(result);
    }

    return ERR_OK;
}

int searchEmbeddedLyrics(cstr_t szSongFile, uint32_t &lrcSourceType) {
    lrcSourceType = 0;

    VecStrings vLyricsNames;
    int nRet = MediaTags::getEmbeddedLyrics(szSongFile, vLyricsNames);
    if (nRet != ERR_OK) {
        return nRet;
    }

    for (uint32_t i = 0; i < vLyricsNames.size(); i++) {
        lrcSourceType |= lyrSrcTypeFromName(vLyricsNames[i].c_str());
    }

    return ERR_OK;
}

#if UNIT_TEST

#include "utils/unittest.h"

TEST(lrcSearch, CLyricsSearchParameter) {
    string testDir = getUnittestTempDir();
    string strMedia = dirStringJoin(testDir.c_str(), "artist - title.mp3");
    cstr_t szArtist = "artist";
    cstr_t szTitle = "title";

    cstr_t vLyrFileName[] = { "artist - title.lrc", "artist - 01 title.txt", "artist(who are you?) - 01 title.txt", "title.lrc", "artist.lrc", };
    int vMatchValue[] = { __vArTiSame[1] + 2 + 2, __vArTiSame[0] + 1 + 1, __vArTiSame[0] + 1, __vTiSame[1] + 1 + 1, 0 };

    CLyricsSearchParameter searchParam(strMedia.c_str(), szArtist, szTitle);

    for (int i = 0; i < CountOf(vLyrFileName); i++) {
        string strLyrFile = dirStringJoin(testDir.c_str(), vLyrFileName[i]);
        int matchValue = searchParam.calMatchValueByName(testDir.c_str(), vLyrFileName[i], GetLyricsFileType(strLyrFile.c_str()));
        ASSERT_TRUE(matchValue == vMatchValue[i]);
    }
}

TEST(lrcSearch, CLyricsSearchParameter2) {
    string testDir = getUnittestTempDir();
    string strMedia = dirStringJoin(testDir.c_str(), "just a media.mp3");
    cstr_t szArtist = "artist";
    cstr_t szTitle = "title";

    CLyricsSearchParameter searchParam(strMedia.c_str(), szArtist, szTitle);

    string strLyrFile = dirStringJoin(testDir.c_str(), "just a media.lrc");
    int matchValue = searchParam.calMatchValueByName(testDir.c_str(), strLyrFile.c_str(), GetLyricsFileType(strLyrFile.c_str()));
    ASSERT_TRUE(matchValue == __vArTiSame[1] + 2);

    deleteFile(strMedia.c_str());
    deleteFile(strLyrFile.c_str());
}

#endif
