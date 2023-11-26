#pragma once

#include "../MPlayerEngine/IPlayerCore.hpp"
#include "../MPlayerEngine/MPTools.h"
#include "../Skin/EventsDispatcherBase.h"
#include "CurMediaAlbumArt.h"
#include "MediaLibrary.h"


#define LRC_TITLE_MAX_LEN   128

enum LoopMode {
    MP_LOOP_OFF,
    MP_LOOP_ALL,
    MP_LOOP_TRACK,
};

class IMPEvent {
public:
    virtual ~IMPEvent() { }

    enum SettingType {
        MPS_SHUFFLE,                     // bool
        MPS_LOOP,                        // bool
        MPS_MUTE,                        // bool
        MPS_VOLUME,                      // int
        MPS_BALANCE,                     // int
    };

    enum PlaylistChangeAction {
        PCA_FULL_UPDATE,
        PCA_INSERT,
        PCA_MOVE,
        PCA_REMOVE,
        PCA_CLEAR,
    };

    virtual void onStateChanged(PlayerState state) = 0;
    virtual void onSeek() = 0;
    virtual void onCurrentMediaChanged() = 0;
    // nIndex the new/cur item which was modified. nIndexOld, is used for move.
    virtual void onCurrentPlaylistChanged(PlaylistChangeAction action, int nIndex, int nIndexOld) = 0;

    virtual void onSettingChanged(SettingType settingType, int value) = 0;
    virtual void onEQSettingChanged(const EQualizer *eq) = 0;

};

class CPlayer : public IPlayerCoreCallback {
public:
    CPlayer();
    ~CPlayer();

public:
    // IPlayerCoreCallback
    virtual void onEndOfPlaying() override;
    virtual void onErrorOccured(const char *errorCode) override;

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

    PlayerState getPlayerState();

    uint32_t getMediaLength();
    uint32_t getPlayPos();

    void setLoop(LoopMode loopMode);
    LoopMode getLoop();
    void setToNextLoopMode();

    void setShuffle(bool bValue);
    bool isShuffle();

    int getVolume();
    void setVolume(int nVol);

    bool isMute();
    void setMute(bool bValue);

    void setBalance(int balance);
    int getBalance();

    void setEQ(const EQualizer *eq);
    void getEQ(EQualizer *eq);

    CMediaLibrary *getMediaLibrary() { return m_mediaLib; }

    void clearNowPlaying();
    void newNowPlaying();

    void addDirToNowPlaying(cstr_t szDir, bool bIncSubDir = true);
    void addToNowPlaying(cstr_t szMedia);

    bool isMediaInPlaylist(cstr_t szMedia);

    void addFileToMediaLib(cstr_t szMedia);
    void addDirToMediaLib(cstr_t szDir, bool bIncSubDir);

    bool isCurrentMediaFileExist();

    bool isMediaOpened() { return m_currentMedia != nullptr; }

    int getCurrentMediaIndex() { return m_idxCurrentMedia; }

    bool isCurrentMediaTempPlaying();

    void playMedia(MediaPtr &media);
    void playMedia(int nPlaylistIndex);

    void getFileOpenDlgExtention(string &strExtentions) const;
    void getSupportedExtentions(vector<string> &vExtentions) const;
    bool isExtSupported(cstr_t szExt);
    bool isExtAudioFile(cstr_t szExt);
    static bool isExtPlaylistFile(cstr_t szExt);

    void setNowPlayingModified(bool bModified) { m_isNowPlayingModified = bModified; }

    static string formatMediaTitle(Media *media);

    void filterLowRatingMedia(Playlist *playlist);

    cstr_t getArtist() { return m_szArtist; }
    cstr_t getTitle() { return m_szTitle; }
    cstr_t getAlbum() { return m_szAlbum; }
    cstr_t getFullTitle() { return m_szFullTitle; }
    cstr_t getSrcMedia() { return m_szSrcMedia; }
    void getSrcMedia(char *szMediaUrl, int nBufSize)
        { strcpy_safe(szMediaUrl, nBufSize, m_szSrcMedia); }
    string getMediaKey();
    int getMediaID();

    void getCurMediaAttribute(MediaAttribute mediaAttr, string &strValue);

    void saveCurrentPlaylist();

    void setLyrDrawUpdateFast(bool bFast);

    //
    // Use UI Seekbar time as playing time
    //
    void onUISeekbarSeek(int nPos);
    void setUseSeekTimeAsPlayingTime(bool bUseSeekTimeAsPlayingTime) { m_isUseSeekTimeAsPlayingTime = bUseSeekTimeAsPlayingTime; }
    bool isUseSeekTimeAsPlayingTime() const { return m_isUseSeekTimeAsPlayingTime; }

    void onMediaChanged();

    string getTitleFilterFile();

    MediaPtr newMedia(cstr_t szUrl);
    PlaylistPtr newPlaylist();

    PlaylistPtr getCurrentPlaylist() { return m_currentPlaylist; }
    MediaPtr getCurrentMedia() { return m_currentMedia; }

    ResultCode setCurrentMediaInPlaylist(int index);

    void setCurrentPlaylist(const PlaylistPtr &playlist);

    void updateMediaInfo(Media *media);

    // Reload Media tag info: artist, title, bitarte, bps, etc.
    ResultCode loadMediaTagInfo(Media *media);

    void registerVisualizer(IEventHandler *eventHandler);
    void unregisterVisualizer(IEventHandler *eventHandler);

    //
    // 通知事件
    //
    void notifyPlayStateChanged();
    void notifyCurrentPlaylistChanged(IMPEvent::PlaylistChangeAction action, int nIndex, int nIndexOld);
    void notifySettingsChanged(IMPEvent::SettingType settingType, int value);
    void notifyEQSettingsChanged(const EQualizer *peq);
    void notifySeek();

    void notifyPlaylistChanged(Playlist *playlist, IMPEvent::PlaylistChangeAction action, int nIndex, int nIndexOld);

protected:
    void setCurrentMedia(MediaPtr &media);
    void currentMediaChanged();
    void generateShuffleMediaQueue();
    ResultCode doNext(bool bLoop);

protected:
    char                        m_szSrcMedia[MAX_PATH];
    char                        m_szArtist[LRC_TITLE_MAX_LEN], m_szTitle[LRC_TITLE_MAX_LEN];
    char                        m_szAlbum[LRC_TITLE_MAX_LEN], m_szFullTitle[LRC_TITLE_MAX_LEN * 2];
    uint32_t                    m_mediaLength;

    bool                        m_isUseSeekTimeAsPlayingTime;

    VecStrings                  m_vTitleFilters;

protected:
    friend class CMPSkinMainWnd;
    bool                        m_isAutoPlayNext;
    PlayerState                 m_state;

protected:
    int                         m_currentPlaylistId;
    bool                        m_isNowPlayingModified;
    int                         m_ratingFilterMin = 3;

    IPlayerCore                 *m_playerCore = nullptr;

    CMediaLibrary               *m_mediaLib;
    PlaylistPtr                 m_currentPlaylist;
    MediaPtr                    m_currentMedia;

    std::recursive_mutex        m_mutexDataAccess;
    vector<int>                 m_vShuffleMedia;
    int                         m_idxCurrentShuffleMedia;
    bool                        m_isShuffle = false;
    bool                        m_isMute = false;
    LoopMode                    m_loopMode = MP_LOOP_OFF;
    int                         m_volume = -1;
    int                         m_balance = 0;
    EQualizer                   m_equalizer;

    bool                        m_isCurMediaPlayed = false;
    int                         m_idxCurrentMedia = 0;

    bool                        m_isAutoAddToMediaLib = true;

    SetStrings                  m_audioFileExts;

};

string getMediaFormat(cstr_t url);

string formatDuration(int nTimeSec);

void getArtistTitleFromFileName(string &artist, string &title, cstr_t fileName);

/**
* Format media title in format of "artist - title"
**/
string formatMediaTitle(cstr_t artist, cstr_t title);

extern CPlayer g_player;
