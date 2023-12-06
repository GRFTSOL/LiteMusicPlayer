/********************************************************************
    Created  :    2002年1月2日 22:54:43
    FileName :    LyricsLocalSearch.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "LyricsLocalSearch.h"
#include "../LyricsLib/HelperFun.h"


cstr_t SZ_FOLDER_PREFIX = "LyricFolder";

#define SZ_S2L_HEADER_V10   "MPS2LV1.0"
#define LEN_S2L_HEADER_V10  9

#define SZ_S2L_HEADER       "MPS2LV1.1"
#define LEN_S2L_HEADER      9

// rule: 1) http://xxxx
//       2) without the extention of song file.
bool isShoutcastMedia(cstr_t szMedia) {
    if (strncasecmp(szMedia, "http:", 5) == 0) {
        return true;
    } else if (strncasecmp(szMedia, "uvox:", 5) == 0) {
        return true;
    }

    return false;
}

bool isShoutcastAssociateKeyName(cstr_t szMedia) {
    if (strncasecmp(szMedia, "http:", 5) == 0) {
        cstr_t p = strstr(szMedia + 5, "/[shoutcast]");
        if (p) {
            return true;
        }
    } else if (strncasecmp(szMedia, "uvox:", 5) == 0) {
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////

CLyricsLocalSearch::CLyricsLocalSearch() {
    m_bSaved = true;
}

CLyricsLocalSearch::~CLyricsLocalSearch() {
}

void CLyricsLocalSearch::loadLyricFolderCfg() {
    for (int i = 0; ; i ++) {
        char szKeyName[MAX_PATH];

        // get folder name
        snprintf(szKeyName, CountOf(szKeyName), "%s%d", SZ_FOLDER_PREFIX, i);
        string strFolder = CMLProfile::getDir(SZ_SECT_SEARCH_FOLDER, szKeyName, "");
        if (strFolder.size()) {
            m_vLyricsFolders.push_back(strFolder);
        } else {
            break;
        }
    }
}

void CLyricsLocalSearch::saveLyricFolderCfg() {
    if (m_bSaved) {
        return;
    }

    m_bSaved = true;

    char szKeyName[MAX_PATH];

    for (uint32_t i = 0; i < m_vLyricsFolders.size(); i++) {
        // SAVE folder settings
        snprintf(szKeyName, CountOf(szKeyName), "%s%d", SZ_FOLDER_PREFIX, i);
        CMLProfile::writeDir(SZ_SECT_SEARCH_FOLDER, szKeyName, m_vLyricsFolders[i].c_str());
    }

    // set end positions.
    snprintf(szKeyName, CountOf(szKeyName), "%s%d", SZ_FOLDER_PREFIX, (int)m_vLyricsFolders.size());
    g_profile.writeString(SZ_SECT_SEARCH_FOLDER, szKeyName, "");
}

void CLyricsLocalSearch::searchAllMatchLyrics(cstr_t szSongFile, cstr_t szArtist, cstr_t szTitle, ListLyrSearchResults &vLyrics) {
    CLyricsSearchParameter searchParam(szSongFile, szArtist, szTitle);
    searchLyrics(searchParam, vLyrics);
    //
    //     if (vLyrics.size() == 0 && !isEmptyString(szSongFile))
    //     {
    //         string strArtist, strTitle;
    //         getArtistTitleFromFileName(strArtist, strTitle, szSongFile);
    //         if (strTitle.empty()
    //             || strcasecmp(szArtist, strArtist.c_str()) == 0 && strcasecmp(szTitle, strTitle.c_str()) == 0)
    //             return;
    //
    //         CLyricsSearchParameter searchParam(szSongFile, szArtist, szTitle);
    //         if (searchParam.strTitleFiltered.size() > 0)
    //             searchLyrics(searchParam, &vLyrics);
    //     }
}

// search lyrics by artist and title.
void CLyricsLocalSearch::searchLyrics(CLyricsSearchParameter &searchParam, ListLyrSearchResults &vLyrics) {
    searchEmbeddedLyrics(searchParam.szSongFile, vLyrics, searchParam.bOnlySearchBestMatch);
    if (searchParam.bOnlySearchBestMatch) {
        ListLyrSearchResults::iterator it = vLyrics.getTheBestMatchLyrics();
        if (it != vLyrics.end()) {
            LrcSearchResult &result = *it;
            if (result.nMatchValue >= MATCH_VALUE_EMBEDDED_LRC) {
                searchParam.strBestMatchLyrics = result.strUrl.c_str();
                searchParam.nMatchValueOfBest = (int)result.nMatchValue;
                return;
            }
        }
    }

    string strDir;

    // search lyrics in the same folder of song file.
    {
        if (!isEmptyString(searchParam.szSongFile)) {
            strDir = fileGetPath(searchParam.szSongFile);
            searchMatchLyricsInDir(strDir.c_str(), searchParam, vLyrics, false);
            if (searchParam.isBestMatchLyricsFound()) {
                return;
            }
        }
    }

    // search in lyrics download folder
    {
        strDir = g_LyricsDownloader.getDefSavePath();
        dirStringAddSep(strDir);

        searchMatchLyricsInDir(strDir.c_str(), searchParam, vLyrics, true);
        if (searchParam.isBestMatchLyricsFound()) {
            return;
        }
    }

    // search in the folder that user specified.
    for (int i = getSearchFolerCount() - 1; i >= 0; i --) {
        strDir = m_vLyricsFolders[i].c_str();
        dirStringAddSep(strDir);

        searchMatchLyricsInDir(strDir.c_str(), searchParam, vLyrics, true);
        if (searchParam.isBestMatchLyricsFound()) {
            return;
        }
    }

    // Finally use the best match lyrics in vLyrics.
    ListLyrSearchResults::iterator it = vLyrics.getTheBestMatchLyrics();
    if (it != vLyrics.end()) {
        LrcSearchResult &result = *it;
        if (result.nMatchValue >= MATCH_VALUE_OK) {
            searchParam.strBestMatchLyrics = result.strUrl.c_str();
            searchParam.nMatchValueOfBest = (int)result.nMatchValue;
            return;
        }
    }
}

string CLyricsLocalSearch::getFolder(int nIndex) {
    if (nIndex >= 0 && nIndex < m_vLyricsFolders.size()) {
        return m_vLyricsFolders[nIndex];
    } else {
        return "";
    }
}

bool CLyricsLocalSearch::setFolder(cstr_t szFolder) {
    for (int i = getSearchFolerCount() - 1; i >= 0; i --) {
        if (strcasecmp(m_vLyricsFolders[i].c_str(), szFolder) == 0) {
            return false;
        }
    }

    m_bSaved = false;
    m_vLyricsFolders.push_back(szFolder);

    return true;
}

int CLyricsLocalSearch::getSearchFolerCount() {
    return (int)m_vLyricsFolders.size();
}

bool CLyricsLocalSearch::removeFolder(int nIndex) {
    if (nIndex >= getSearchFolerCount() || nIndex < 0) {
        return false;
    }

    m_bSaved = false;

    m_vLyricsFolders.erase(m_vLyricsFolders.begin() + nIndex);

    return true;
}

void CLyricsLocalSearch::init() {
    loadLyricFolderCfg();

    loadLyricsAssociation();
}

void CLyricsLocalSearch::quit() {
    saveLyricFolderCfg();
    m_vLyricsFolders.clear();
}

char * getDiskImageFileExtEnd(cstr_t szFile) {
    static cstr_t vImageExts[] = { ".cue", ".tak", ".wv" };

    for (int i = 0; i < CountOf(vImageExts); i++) {
        char * p = stristr(szFile, vImageExts[i]);
        if (p) {
            return p + strlen(vImageExts[i]);
        }
    }

    return nullptr;
}

string CLyricsLocalSearch::getAssociateFileKeyword(cstr_t szSongFile, cstr_t szFulltitle) {
    string strAssociateKey;

    if (isEmptyString(szSongFile)) {
        return szFulltitle;
    }

    strAssociateKey = szSongFile;

    char * szEndPos = getDiskImageFileExtEnd(szSongFile);
    if (szEndPos) {
        // to associate a .cue, .tak track correctly, the title will be appended to it.
        if (strcmp(szEndPos, " - ") == 0
            || strcmp(szEndPos, ",") == 0) {
            *szEndPos = '\0';
        }
        strAssociateKey += szFulltitle;
    } else if (isShoutcastMedia(szSongFile)) {
        // to associate a shoutcast track correctly, the title will be appended to it.
        strAssociateKey += "/[shoutcast]";
        strAssociateKey += szFulltitle;
    }

    return strAssociateKey;
}

bool CLyricsLocalSearch::associateLyrics(cstr_t szAssociateFileKeyword, cstr_t szLyricFile) {
    assert(szAssociateFileKeyword);

    if (isEmptyString(szAssociateFileKeyword)) {
        return false;
    }

    SONG_LYRIC_MAP::iterator it;
    string strKey = toAssociateKeyword(szAssociateFileKeyword);

    {
        MutexAutolock lock(m_mutex);
        // Is already associated and same?
        it = m_mapLyricsAssociate.find(strKey);
        if (it != m_mapLyricsAssociate.end()) {
            if (strcmp((*it).second.c_str(), szLyricFile) == 0) {
                return false;
            }
        }
        m_mapLyricsAssociate[strKey] = szLyricFile;
    }

    // save??
    saveLyricsAssociation();

    return true;
}

bool CLyricsLocalSearch::cancelAssociate(cstr_t szAssociateFileKeyword) {
    {
        MutexAutolock autoLock(m_mutex);
        auto itLyric = m_mapLyricsAssociate.find(toAssociateKeyword(szAssociateFileKeyword));
        if (itLyric != m_mapLyricsAssociate.end()) {
            m_mapLyricsAssociate.erase(itLyric);
            return true;
        }
    }

    saveLyricsAssociation();

    return false;
}

bool CLyricsLocalSearch::isAssociatedLyrics(cstr_t szAssociateFileKeyword) {
    assert(szAssociateFileKeyword);
    SONG_LYRIC_MAP::iterator itLyric;
    MutexAutolock autoLock(m_mutex);

    itLyric = m_mapLyricsAssociate.find(toAssociateKeyword(szAssociateFileKeyword));
    if (itLyric != m_mapLyricsAssociate.end()) {
        return true;
    }

    return false;
}

bool CLyricsLocalSearch::isAssociatedWithNoneLyrics(cstr_t szAssociateKeyword) {
    char szLyricsFile[MAX_PATH];

    if (getAssociatedLyrics(szAssociateKeyword, szLyricsFile, MAX_PATH)) {
        if (strcmp(NONE_LYRCS, szLyricsFile) == 0) {
            return true;
        }
    }
    return false;
}

inline bool filePathIsBeginWithDriver(cstr_t szFile) {
    return szFile[0] && szFile[1] == ':';
}

bool CLyricsLocalSearch::getAssociatedLyrics(cstr_t szAssociateFileKeyword, char * szLyricFile, int nMaxBuff) {
    assert(szLyricFile);
    emptyStr(szLyricFile);

    SONG_LYRIC_MAP::iterator itLyric;
    MutexAutolock autoLock(m_mutex);

    itLyric = m_mapLyricsAssociate.find(toAssociateKeyword(szAssociateFileKeyword));
    if (itLyric != m_mapLyricsAssociate.end()) {
        strcpy_safe(szLyricFile, nMaxBuff, (*itLyric).second.c_str());

        if (isLyricsExist(szLyricFile)) {
            return true;
        }

        if (filePathIsBeginWithDriver(szAssociateFileKeyword)
            && filePathIsBeginWithDriver(szLyricFile)) {
            szLyricFile[0] = szAssociateFileKeyword[0];
            return isLyricsExist(szLyricFile);
        }
    }

    return false;
}

bool CLyricsLocalSearch::isLyricsExist(cstr_t szLyricSource) {
    LRC_SOURCE_TYPE sourceType = lyrSrcTypeFromName(szLyricSource);

    if (sourceType == LST_FILE) {
        if (isFileExist(szLyricSource)) {
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

void CLyricsLocalSearch::loadLyricsAssociation() {
    SONG_LYRIC_MAP::iterator it;
    char szBuff[1024];
    char *szStr;
    int nLen, nLenBuff;
    FILE *fp;
    MutexAutolock autoLock(m_mutex);

    string file = getAppDataDir() + "MLyrics.S2L";
    fp = fopen(file.c_str(), "rb");
    if (fp == nullptr) {
        goto R_END;
    }

    if (!fgets(szBuff, CountOf(szBuff), fp)) {
        goto R_END;
    }

    if (strncmp(szBuff, SZ_S2L_HEADER, LEN_S2L_HEADER) != 0) {
        goto R_END;
    }

    while (fgets(szBuff, CountOf(szBuff), fp)) {
        szStr = szBuff;
        nLenBuff = (int)strlen(szBuff);

        // get song file name.
        szStr = readInt_t(szStr, nLen); if (nLen > nLenBuff - int(szStr - szBuff)) continue;
        if (*szStr == ',') szStr++; else continue;

        string mediaFn(szStr, nLen);
        szStr += nLen;
        if (*szStr != ',') {
            continue;
        }
        szStr++;

        // get lyrics file name.
        szStr = readInt_t(szStr, nLen); if (nLen > nLenBuff - int(szStr - szBuff)) continue;
        if (*szStr == ',') szStr++; else continue;

        string lyricsFn(szStr, nLen);

        m_mapLyricsAssociate[mediaFn.c_str()] = lyricsFn.c_str();
    }

R_END:
    if (fp) {
        fclose(fp);
    }
}

void CLyricsLocalSearch::saveLyricsAssociation() {
    SONG_LYRIC_MAP::iterator it;
    FILE *fp;
    string str, strUtf8;
    char szTemp[64];

    string file = getAppDataDir() + "MLyrics.S2L";

    fp = fopen(file.c_str(), "wb");
    if (fp == nullptr) {
        ERR_LOG1("Can't save lyrics association file: %s.", file.c_str());
        return;
    }

    if (fwrite(SZ_S2L_HEADER, 1, LEN_S2L_HEADER, fp) != LEN_S2L_HEADER) {
        return;
    }

    if (fwrite("\r\n", 1, 2, fp) != 2) {
        return;
    }

    MutexAutolock autoLock(m_mutex);

    for (it = m_mapLyricsAssociate.begin(); it != m_mapLyricsAssociate.end(); it++) {
        if (isShoutcastAssociateKeyName((*it).first.c_str())) {
            continue;
        }

        str.resize(0);

        //
        // write song file
        //
        strUtf8 = (*it).first.c_str();

        // len
        snprintf(szTemp, CountOf(szTemp), "%d", (int)strUtf8.size());
        str += szTemp; str += ",";

        // file
        str += strUtf8;
        str += ",";

        //
        // write lyrics file
        //
        strUtf8 = (*it).second.c_str();

        // len
        snprintf(szTemp, CountOf(szTemp), "%d", (int)strUtf8.size());
        str += szTemp; str += ",";

        // file
        str += strUtf8;
        str += "\n";

        fwrite(str.c_str(), 1, str.size(), fp);
    }

    fclose(fp);
    return;
}

// Return the best match lyrics only
bool CLyricsLocalSearch::getBestMatchLyrics(cstr_t szSongFile, cstr_t szArtist, cstr_t szTitle, string &strLyrFile) {
    string fullTitle = formatMediaTitle(szArtist, szTitle);
    char szLyricsFile[MAX_PATH];
    if (getAssociatedLyrics(getAssociateFileKeyword(szSongFile, fullTitle.c_str()).c_str(), szLyricsFile, MAX_PATH)) {
        strLyrFile = szLyricsFile;
        return true;
    }

    ListLyrSearchResults vLyrics;
    CLyricsSearchParameter searchParam(szSongFile, szArtist, szTitle, true);
    searchLyrics(searchParam, vLyrics);
    strLyrFile = searchParam.strBestMatchLyrics;
    return strLyrFile.size() > 0;
}
