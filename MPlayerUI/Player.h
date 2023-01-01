#pragma once

#include "../MPlayerEngine/IMPlayer.h"
#include "../MPlayerEngine/MPTools.h"
#include "CurMediaAlbumArt.h"


#define LRC_TITLE_MAX_LEN   128

class CMPlayerEventHandler;
class CMPluginManager;

class CPlayer {
public:
    CPlayer();
    virtual ~CPlayer();

public:
    void onInit();
    void onQuit();

    // control player
    void play();
    void pause();
    void playPause();
    void stop();
    void prev();
    void next();
    void seekTo(uint32_t nMsPos);

    PLAYER_STATE getPlayerState();

    uint32_t getMediaLength();

    uint32_t getPlayPos();

    void setLoop(MP_LOOP_MODE loopMode);
    void setToNextLoopMode();
    void setShuffle(bool bValue);

    MP_LOOP_MODE getLoop();
    bool isShuffle();

    int getVolume();
    void setVolume(int nVol);

    bool isMute();
    void setMute(bool bValue);

    IMPlayer *getIMPlayer() { return m_spPlayer; }

    MLRESULT getMediaLibrary(IMediaLibrary **ppMediaLib)
        { return m_spPlayer->getMediaLibrary(ppMediaLib); }

    void clearPlaylist();
    void newCurrentPlaylist();

    string getCurrentPlaylistFile() { return m_strCurrentPlaylist; }

    void addDirToPlaylist(cstr_t szDir, bool bIncSubDir = true);
    void addToPlaylist(cstr_t szMedia);

    bool loadPlaylist(cstr_t szFile, bool bClear = true);
    bool isMediaInPlaylist(cstr_t szMedia);

    void addFileToMediaLib(cstr_t szMedia);
    void addDirToMediaLib(cstr_t szDir, bool bIncSubDir);

    bool isCurrentMediaFileExist();

    bool isMediaOpened();

    int getCurrentMediaIndex();

    void playMedia(int nPlaylistIndex);

    void getFileOpenDlgExtention(string &strExtentions) const;
    void getSupportedExtentions(vector<string> &vExtentions) const;
    bool isExtSupported(cstr_t szExt);
    bool isExtAudioFile(cstr_t szExt);
    static bool isExtPlaylistFile(cstr_t szExt);

    void setPlaylistModified(bool bModified) { m_bPlModified = bModified; }

    static string formatMediaTitle(IMedia *pMedia);

    void filterLowRatingMedia(IPlaylist *playlist);

    cstr_t getArtist() { return m_szArtist; }
    cstr_t getTitle() { return m_szTitle; }
    cstr_t getAlbum() { return m_szAlbum; }
    cstr_t getFullTitle() { return m_szFullTitle; }
    cstr_t getSrcMedia() { return m_szSrcMedia; }
    void getSrcMedia(char * lpszSourceMedia, int nBufSize)
        { strcpy_safe(lpszSourceMedia, nBufSize, m_szSrcMedia); }
    string getMediaKey();

    MLRESULT getCurrentMedia(IMedia **ppMedia);
    MLRESULT getCurrentPlaylist(IPlaylist **ppPlaylist);
    MLRESULT newPlaylist(IPlaylist **ppPlaylist);
    MLRESULT setCurrentPlaylist(IPlaylist *pPlaylist);
    MLRESULT setCurrentMediaInPlaylist(int nIndex);
    MLRESULT newMedia(IMedia **ppMedia, cstr_t szUrl);

    int unregisterVis(IVis *pVis);
    int registerVis(IVis *pVis);

    void getCurMediaAttribute(MediaAttribute mediaAttr, string &strValue);

    void saveCurrentPlaylist();
    void saveCurrentPlaylistAs(cstr_t szFile);

    int getMediaOfAlbumArt(IMedia *pMedia);

    void setLyrDrawUpdateFast(bool bFast);

    //
    // Use UI Seekbar time as playing time
    //
    void uISeekbarOnSeek(int nPos);
    void setUseSeekTimeAsPlayingTime(bool bUseSeekTimeAsPlayingTime) { m_bUseSeekTimeAsPlayingTime = bUseSeekTimeAsPlayingTime; }
    bool isUseSeekTimeAsPlayingTime() const { return m_bUseSeekTimeAsPlayingTime; }

    void onMediaChangedOfZikiPlayer();

    string getTitleFilterFile();
    bool setFieldValue(char szField[], int lenField, cstr_t szValue);

protected:
    char                        m_szSrcMedia[MAX_PATH];
    char                        m_szArtist[LRC_TITLE_MAX_LEN], m_szTitle[LRC_TITLE_MAX_LEN];
    char                        m_szAlbum[LRC_TITLE_MAX_LEN], m_szFullTitle[LRC_TITLE_MAX_LEN * 2];
    uint32_t                    m_nMediaLength;

    bool                        m_bUseSeekTimeAsPlayingTime;

    VecStrings                  m_vTitleFilters;

protected:
    friend class CMPSkinMainWnd;
    PLAYER_STATE                m_playerState;

protected:
    // For DHPlayer
    CMPAutoPtr<CMPluginManager> m_pPluginMgr;
    string                      m_strCurrentPlaylist;
    bool                        m_bPlModified;
    int                         m_ratingFilterMin;

    IMPlayer                    *m_spPlayer;
    CMPlayerEventHandler        *m_pEventHandler;

};

extern CPlayer g_Player;
