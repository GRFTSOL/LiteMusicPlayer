/********************************************************************
    Created  :    2002/01/04    21:41
    FileName :    LyricsLocalSearch.h
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#pragma once

#include "../LyricsLib/LyricsSearch.h"


//
// search local lyrics
//
class CLyricsLocalSearch
{
public:
    void init();
    void quit();

    // search match lyrics
    void searchAllMatchLyrics(cstr_t szSongFile, cstr_t szArtist, cstr_t szTitle, V_LRCSEARCHRESULT &vLyrics);

    // search best match lyrics
    bool getBestMatchLyrics(cstr_t szSongFile, cstr_t szArtist, cstr_t szTitle, string &strLyrFile);

    // song://lyrics3v2
    // song://id3v2/sylt/xxx
    // song://id3v2/uslt/xxx
    static string getAssociateFileKeyword(cstr_t szSongFile, cstr_t szFulltitle);
    bool getAssociatedLyrics(cstr_t szAssociateFileKeyword, char * szLyricFile, int nMaxBuff);
    bool isAssociatedLyrics(cstr_t szAssociateFileKeyword);
    bool isAssociatedWithNoneLyrics(cstr_t szAssociateKeyword);
    bool associateLyrics(cstr_t szAssociateFileKeyword, cstr_t szLyricFile);
    bool cancelAssociate(cstr_t szAssociateFileKeyword);
    bool isLyricsExist(cstr_t szLyricSource);

    //
    // search folder
    //
    int getSearchFolerCount();
    bool removeFolder(int nIndex);
    string getFolder(int nIndex);
    bool setFolder(cstr_t szFolder);
    void saveLyricFolderCfg();

protected:
    void searchLyrics(CLyricsSearchParameter &searchParam, V_LRCSEARCHRESULT &vLyrics);

    void loadLyricsAssociation();
    void saveLyricsAssociation();

    string toAssociateKeyword(cstr_t szKeyword)
    {
#ifdef _WIN32_DESKTOP
        // Use keyword without driver letter info "C:"
        if (szKeyword[0] && szKeyword[1] == ':')
        {
            return toLower(szKeyword + 2);
        }
#endif
        return toLower(szKeyword);
    }

    void loadLyricFolderCfg();

public:
    CLyricsLocalSearch();
    virtual ~CLyricsLocalSearch();

protected:
    VecStrings    m_vLyricsFolders;

    //
    // Lyrics association map:
    //
    // song file <--->  lyrics file
    //
    // To support removable disk (mp3 file, lyrics are in removable disk):
    // 1) song file is in lower case
    // 2) if song file is in same driver with lyrics file, "x:" will be removed, but 
    //    the "x:" in lyrics file should be kept.
    // 3) if song file do not have driver info,  but lyrics file doesn't exists there, 
    //    Check whether lyrics is in the $Product$ program driver.
    // 4) "Lyrics" folder should be verified dynamically.
    //
    typedef map<string, string> SONG_LYRIC_MAP;

    SONG_LYRIC_MAP            m_mapLyricsAssociate;
    std::mutex                    m_mutex;

    bool                    m_bSaved;
};
