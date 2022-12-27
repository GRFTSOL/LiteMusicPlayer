#include "../MPlayerEngine/MPlayer.h"
#include "MPlayerAppBase.h"
#include "Player.h"
#include "PlayListFile.h"
#include "OnlineSearch.h"
#include "MPluginManager.h"
#include "../LyricsLib/HelperFun.h"
#include "../LyricsLib/MLLib.h"


#ifdef _WIN32
#include "MPMsg.h"
#include "win32/PlayerEventDispatcher.h"
#endif

#ifdef _MAC_OS
#include "mac/PlayerEventDispatcher.h"
#endif

CPlayer            g_Player;
CPlayerEventDispatcher        g_playerEventDispatcher;


void formatFullTitle(cstr_t szArtist, cstr_t szTitle, cstr_t szUri, string &strFullTitle)
{
    strFullTitle = formatMediaTitle(szArtist, szTitle);
    if (strFullTitle.empty())
        strFullTitle = urlGetTitle(szUri);
}

void copyPlaylist(IPlaylist *src, IPlaylist *dst)
{
    int            nCount = src->getCount();
    for (int i = 0; i < nCount; i++)
    {
        CMPAutoPtr<IMedia>        media;
        if (src->getItem(i, &media) == ERR_OK)
            dst->insertItem(-1, media);
    }
}

///////////////////////////////////////////////////////////////////////////
//
class CMPlayerEventHandler : public IMPEvent
{
    OBJ_REFERENCE_DECL
public:
    CMPlayerEventHandler() { OBJ_REFERENCE_INIT }
    virtual ~CMPlayerEventHandler() { }

    virtual void onStateChanged(PLAYER_STATE state)
    {
        CEventPlayerStatusChanged    *pEvent = new CEventPlayerStatusChanged();
        pEvent->eventType = ET_PLAYER_STATUS_CHANGED;
        pEvent->status = state;
        CMPlayerAppBase::getEventsDispatcher()->dispatchUnsyncEvent(pEvent);

        g_playerEventDispatcher.stopLyrDrawUpdate();

        if (state == PS_PLAYING)
            g_playerEventDispatcher.startLyrDrawUpdate();
    }

    virtual void onSeek()
    {
        IEvent    *pEvent = new IEvent();
        pEvent->eventType = ET_PLAYER_SEEK;
        CMPlayerAppBase::getEventsDispatcher()->dispatchUnsyncEvent(pEvent);
    }

    virtual void onCurrentMediaChanged()
    {
        g_Player.onMediaChangedOfZikiPlayer();

        IEvent    *pEvent = new IEvent();
        pEvent->eventType = ET_PLAYER_CUR_MEDIA_CHANGED;
        CMPlayerAppBase::getEventsDispatcher()->dispatchUnsyncEvent(pEvent);
    }

    virtual void onCurrentPlaylistChanged(IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, int nIndex, int nIndexOld)
    {
        CEventPlaylistChanged    *pEvent = new CEventPlaylistChanged();
        pEvent->eventType = ET_PLAYER_CUR_PLAYLIST_CHANGED;
        pEvent->action = action;
        pEvent->nIndex = nIndex;
        pEvent->nIndexOld = nIndexOld;
        CMPlayerAppBase::getEventsDispatcher()->dispatchUnsyncEvent(pEvent);
    }

    virtual void onSettingChanged(MP_SETTING_TYPE settingType, int value)
    {
        CEventPlayerSettingChanged    *pEvent = new CEventPlayerSettingChanged();
        pEvent->eventType = ET_PLAYER_SETTING_CHANGED;
        pEvent->settingType = settingType;
        pEvent->value = value;
        CMPlayerAppBase::getEventsDispatcher()->dispatchUnsyncEvent(pEvent);
    }

    virtual void onEQSettingChanged(const EQualizer *eq)
    {
        CEventPlayerEQChanged    *pEvent = new CEventPlayerEQChanged();
        pEvent->eventType = ET_PLAYER_EQ_SETTING_CHANGED;
        pEvent->eqlalizer = *eq;
        CMPlayerAppBase::getEventsDispatcher()->dispatchUnsyncEvent(pEvent);
    }

    virtual void onPlayHaltError(MLRESULT nError)
    {
    }

};


CPlayer::CPlayer()
{
    emptyStr(m_szFullTitle);
    emptyStr(m_szArtist);
    emptyStr(m_szTitle);
    emptyStr(m_szSrcMedia);
    m_pEventHandler = nullptr;
    m_ratingFilterMin = 3;

    m_bUseSeekTimeAsPlayingTime = false;
    m_nMediaLength = 0;
    m_playerState = PS_STOPED;
}

CPlayer::~CPlayer()
{
}

void CPlayer::onInit(bool bMiniLyrics)
{
    // open title filters
    string strFilter;
    if (readFileByBom(getTitleFilterFile().c_str(), strFilter))
    {
        strSplit(strFilter.c_str(), '\n', m_vTitleFilters);
        for (size_t i = 0; i < m_vTitleFilters.size();)
        {
            if (m_vTitleFilters[i].empty() || startsWith(m_vTitleFilters[i].c_str(), ";"))
                m_vTitleFilters.erase(m_vTitleFilters.begin() + i);
            else
                i++;
        }
        trimStr(m_vTitleFilters, '\r');
        trimStr(m_vTitleFilters, ' ');
    }

    if (CMPlayer::getInstance(&m_spPlayer) != ERR_OK)
        return;

    assert(m_pPluginMgr == nullptr);

    m_pPluginMgr = new CMPluginManager;
    m_spPlayer->setPluginManager(m_pPluginMgr);

    m_pEventHandler = new CMPlayerEventHandler;
    m_pEventHandler->addRef();
    m_spPlayer->registerEvent(m_pEventHandler);

    cstr_t            szValue;
    MP_LOOP_MODE    loopMode;

    // get loop mode
    szValue = g_profile.getString(SZ_SECT_PLAYER, "repeat", "off");
    if (strcasecmp(szValue, "all") == 0)
        loopMode = MP_LOOP_ALL;
    else if (strcasecmp(szValue, "track") == 0)
        loopMode = MP_LOOP_TRACK;
    else
        loopMode = MP_LOOP_OFF;
    m_spPlayer->setLoop(loopMode);

    // shuffle
    m_spPlayer->setShuffle(
        isTRUE(g_profile.getString(SZ_SECT_PLAYER, "shuffle", SZ_FALSE)));

    // mute
    m_spPlayer->setMute(
        isTRUE(g_profile.getString(SZ_SECT_PLAYER, "mute", SZ_FALSE)));

    // volume
    m_spPlayer->setVolume(g_profile.getInt(SZ_SECT_PLAYER, "volume", 30));


    // Load current playlist
    string playlist = g_profile.getString("LatestPlaylist", "");
    if (!playlist.empty()) {
        loadPlaylist(playlist.c_str());
    }

    // current playing music
     m_spPlayer->setCurrentMediaInPlaylist(
        g_profile.getInt(SZ_SECT_PLAYER, "NowPlayingIdx", 0));

    g_playerEventDispatcher.init();

    if (g_profile.getBool("StartPlayAtStartup", false))
        play();
}

void CPlayer::onQuit()
{
    if (m_spPlayer) {
        g_playerEventDispatcher.quit();

        saveCurrentPlaylist();

        // save latest playing song file
        g_profile.writeInt(SZ_SECT_PLAYER, "NowPlayingIdx", m_spPlayer->getCurrentMediaInPlaylist());

        if (m_pEventHandler)
        {
            m_spPlayer->unregisterEvent(m_pEventHandler);
            m_pEventHandler->release();
        }

        assert(m_spPlayer);
        m_spPlayer->stop();
        CMPlayer::quitInstance(&m_spPlayer);
    }
}

// control player
void CPlayer::play()
{
    if (m_spPlayer)
        m_spPlayer->play();
}

void CPlayer::pause()
{
    if (m_spPlayer)
        m_spPlayer->pause();
}

void CPlayer::playPause()
{
    if (m_spPlayer)
    {
        if (m_spPlayer->getState() == PS_PLAYING)
            m_spPlayer->pause();
        else
            m_spPlayer->play();
    }
}

void CPlayer::stop()
{
    if (m_spPlayer)
        m_spPlayer->stop();
}

void CPlayer::prev()
{
    if (m_spPlayer)
        m_spPlayer->prev();
}

void CPlayer::next()
{
    if (m_spPlayer)
        m_spPlayer->next();
}

void CPlayer::seekTo(uint32_t nMsPos)
{
    if (m_spPlayer)
        m_spPlayer->seek(nMsPos);
}

// get player state and song info
PLAYER_STATE CPlayer::getPlayerState()
{
    if (m_spPlayer)
        return m_spPlayer->getState();
    return PS_STOPED;
}

uint32_t CPlayer::getMediaLength()
{
    return m_nMediaLength;
}

uint32_t CPlayer::getPlayPos()
{
    if (m_spPlayer)
    {
        uint32_t        nPos;

        assert(m_spPlayer);
        if (!m_spPlayer)
            return 0;

        nPos = m_spPlayer->getPos();

        return nPos;
    }

    return 0;
}

bool CPlayer::hasVideo()
{
    if (m_spPlayer)
        return false;

    return false;
}

void CPlayer::setLyrDrawUpdateFast(bool bFast)
{
    g_playerEventDispatcher.setLyrDrawUpdateFast(bFast);
}

void CPlayer::setToNextLoopMode()
{
    if (!m_spPlayer)
        return;

    MP_LOOP_MODE loopMode = m_spPlayer->getLoop();

    if (loopMode == MP_LOOP_OFF)
        loopMode = MP_LOOP_ALL;
    else if (loopMode == MP_LOOP_ALL)
        loopMode = MP_LOOP_TRACK;
    else
        loopMode = MP_LOOP_OFF;

    setLoop(loopMode);
}

void CPlayer::setLoop(MP_LOOP_MODE loopMode)
{
    if (!m_spPlayer)
        return;

    m_spPlayer->setLoop(loopMode);

    cstr_t        szValue;
    if (loopMode == MP_LOOP_ALL)
        szValue = "all";
    else if (loopMode == MP_LOOP_TRACK)
        szValue = "track";
    else
        szValue = "off";

    g_profile.writeString(SZ_SECT_PLAYER, "repeat", szValue);
}

void CPlayer::setShuffle(bool bValue)
{
    if (!m_spPlayer)
        return;

    m_spPlayer->setShuffle(bValue);

    g_profile.writeString(SZ_SECT_PLAYER, "shuffle", bValue ? SZ_TRUE : SZ_FALSE);
}

MP_LOOP_MODE CPlayer::getLoop()
{
    if (!m_spPlayer)
        return MP_LOOP_OFF;

    return m_spPlayer->getLoop();
}

bool CPlayer::isShuffle()
{
    if (m_spPlayer)
        return m_spPlayer->getShuffle();
    else
        return false;
}

int CPlayer::getVolume()
{
    if (m_spPlayer)
        return m_spPlayer->getVolume();
    else
        return g_playerEventDispatcher.getMasterVolume();
}

void CPlayer::setVolume(int nVol)
{
    if (m_spPlayer)
    {
        g_profile.writeString(SZ_SECT_PLAYER, "mute", SZ_FALSE);
        g_profile.writeInt(SZ_SECT_PLAYER, "volume", (int)nVol);
        m_spPlayer->setVolume(nVol);
    }
    else
        g_playerEventDispatcher.setMasterVolume(nVol);
}

bool CPlayer::isMute()
{
    if (m_spPlayer)
        return m_spPlayer->getMute();
    else
        return false;
}

void CPlayer::setMute(bool bValue)
{
    if (!m_spPlayer)
        return;

    m_spPlayer->setMute(bValue);

    g_profile.writeString(SZ_SECT_PLAYER, "mute", bValue ? SZ_TRUE : SZ_FALSE);
}

void CPlayer::clearPlaylist()
{
    if (!m_spPlayer)
        return;

    setPlaylistModified(true);

    CMPAutoPtr<IPlaylist>    playlist;
    if (m_spPlayer->getCurrentPlaylist(&playlist) == ERR_OK)
        playlist->clear();
}

void CPlayer::newCurrentPlaylist()
{
    clearPlaylist();

    m_strCurrentPlaylist = getAppDataDir();
    m_strCurrentPlaylist += "untitled playlist.m3u";

//    if (!isFileExist(m_strCurrentPlaylist.c_str()))
//        return;
//
//    stringPrintf strFile;
//    for (int i = 0; i < 100; i++)
//    {
//        strFile.printf("%suntitled playlist %d.m3u", getAppDataDir().c_str(), i);
//
//        if (!isFileExist(strFile.c_str()))
//        {
//            m_strCurrentPlaylist = strFile.c_str();
//            break;
//        }
//    }
}

void CPlayer::addDirToPlaylist(cstr_t szDir, bool bIncSubDir)
{
    if (!m_spPlayer)
        return;

    FileFind          find;
    string            strDir;
    string            strFile;

    if (!find.openDir(szDir))
        return;

    strDir = szDir;
    dirStringAddSep(strDir);

    while (find.findNext())
    {
        if (find.isCurDir())
        {
            if (bIncSubDir && strcmp(".", find.getCurName()) != 0 &&
                strcmp("..", find.getCurName()) != 0)
            {
                strFile = strDir + find.getCurName();
                addDirToPlaylist(strFile.c_str(), bIncSubDir);
            }
        }
        else
        {
            strFile = strDir + find.getCurName();
            addToPlaylist(strFile.c_str());
        }
    }
}

void CPlayer::addToPlaylist(cstr_t szMedia)
{
    if (!m_spPlayer)
        return;

    IPlaylist        *playList;
    MLRESULT        nRet;
    cstr_t            szExt;

    szExt = fileGetExt(szMedia);
    if (isEmptyString(szExt))
        return;

    if (isExtPlaylistFile(szExt))
    {
        loadPlaylist(szMedia, false);
        return;
    }

    if (!isExtAudioFile(szExt))
        return;

    // do not add repeated item.
    if (isMediaInPlaylist(szMedia))
        return;

    nRet = m_spPlayer->getCurrentPlaylist(&playList);
    if (nRet == ERR_OK)
    {
        CMPAutoPtr<IMedia>    media;

        if (m_spPlayer->newMedia(&media, szMedia) == ERR_OK)
            playList->insertItem(-1, media);

         playList->release();
    }
}

bool CPlayer::isMediaInPlaylist(cstr_t szMedia)
{
    if (!m_spPlayer)
        return true;

    CMPAutoPtr<IPlaylist>    playList;
    MLRESULT        nRet;
    bool            bRet = false;

    nRet = m_spPlayer->getCurrentPlaylist(&playList);
    if (nRet == ERR_OK)
    {
        auto nCount = playList->getCount();
        for (int i = 0; i < nCount; i++)
        {
            CMPAutoPtr<IMedia>    media;
            nRet = playList->getItem(i, &media);
            if (nRet == ERR_OK)
            {
                CXStr    url;
                nRet = media->getSourceUrl(&url);
                if (nRet == ERR_OK)
                {
                    if (strcmp(szMedia, url.c_str()) == 0)
                    {
                        bRet = true;
                        break;
                    }
                }
            }
        }
    }

    return bRet;
}

bool CPlayer::loadPlaylist(cstr_t szFile, bool bClear)
{
    if (!m_spPlayer)
        return false;

    MLRESULT                nRet;
    CMPAutoPtr<IPlaylist>    playlist, curPlaylist;

    if (m_spPlayer->newPlaylist(&playlist) != ERR_OK)
        return false;

    if (bClear)
        m_strCurrentPlaylist = szFile;
    else
    {
        nRet = m_spPlayer->getCurrentPlaylist(&curPlaylist);
        if (nRet == ERR_OK)
        {
            copyPlaylist(curPlaylist, playlist);
        }
    }

    if (!::loadPlaylist(m_spPlayer, playlist, szFile))
        return false;

    nRet = m_spPlayer->setCurrentPlaylist(playlist);

    return nRet == ERR_OK;
}

int CPlayer::getCurrentMediaIndex()
{
    if (!m_spPlayer)
        return 0;

    return m_spPlayer->getCurrentMediaInPlaylist();
}

bool CPlayer::isCurrentMediaFileExist()
{
    if (isEmptyString(m_szSrcMedia) || !isFileExist(m_szSrcMedia))
        return false;
    return true;
}

bool CPlayer::isMediaOpened()
{
    if (m_spPlayer)
    {
        CMPAutoPtr<IMedia>    media;
        MLRESULT    nRet;

        nRet = m_spPlayer->getCurrentMedia(&media);

        return nRet == ERR_OK;
    }
    else
    {
        if (isEmptyString(m_szSrcMedia) && isEmptyString(m_szFullTitle))
            return false;
        return true;
    }
}

void CPlayer::playMedia(int nPlaylistIndex)
{
    if (!m_spPlayer)
        return;

    m_spPlayer->stop();
    m_spPlayer->setCurrentMediaInPlaylist(nPlaylistIndex);
    m_spPlayer->play();
}

void CPlayer::getFileOpenDlgExtention(string &strExtentions) const
{
    // static char    szSupportedExtion[] = "All supported file\0*.m3u;*.pls;*.wpl;*.mp3;*.mp2;*.mpa;*.mp4;*.m4a;*.wav;*.ogg;*.wma\0Playlist file\0*.pls;*.wpl;*.m3u\0Mp3 file\0*.mp3;*.mp2;*.mpa\0WMA file\0*.wma\0MP4 file\0*.mp4;*.m4a\0Ogg file\0*.ogg\0Wave file\0*.wav\0All Files\0*.*\0\0";
    static char    szSupportedExtion[] = "All supported file\0*.m3u;*.pls;*.wpl;*.mp3;*.mp2;*.mpa;*.m4a;*.wav;*.ogg;*.wma\0Playlist file\0*.pls;*.wpl;*.m3u\0Mp3 file\0*.mp3;*.mp2;*.mpa\0WMA file\0*.wma\0MP4 file\0*.m4a\0Ogg file\0*.ogg\0Wave file\0*.wav\0All Files\0*.*\0\0";
    strExtentions.append(szSupportedExtion, CountOf(szSupportedExtion));
}

void CPlayer::getSupportedExtentions(vector<string> &vExtentions) const
{
    if (!m_spPlayer)
        return;

    const ListDecodersInfo &listDecoder = m_pPluginMgr->getDecodersInfo();

    auto itEnd = listDecoder.end();
    for (auto it = listDecoder.begin(); it != itEnd; ++it) {
        const DecoderInfo        &item = *it;
        vExtentions.insert(vExtentions.end(), item.vExt.begin(), item.vExt.end());
    }

    vExtentions.push_back(".m3u");
    vExtentions.push_back(".pls");
    vExtentions.push_back(".wpl");
}

bool CPlayer::isExtSupported(cstr_t szExt)
{
    return isExtAudioFile(szExt) || isExtPlaylistFile(szExt);
}

bool CPlayer::isExtAudioFile(cstr_t szExt)
{
    if (!m_spPlayer)
        return false;

    string strExtLowerCase = toLower(szExt);

    const ListDecodersInfo        &listDecoder = m_pPluginMgr->getDecodersInfo();
    ListDecodersInfo::const_iterator    it;

    for (it = listDecoder.begin(); it != listDecoder.end(); ++it)
    {
        const DecoderInfo        &item = *it;

        if (find(item.vExt.begin(), item.vExt.end(), strExtLowerCase) != item.vExt.end())
            return true;
    }

    return false;
}

bool CPlayer::isExtPlaylistFile(cstr_t szExt)
{
    if (strcasecmp(".m3u", szExt) == 0)
        return true;
    if (strcasecmp(".wpl", szExt) == 0)
        return true;
    if (strcasecmp(".pls", szExt) == 0)
        return true;

    return false;
}


void CPlayer::addDirToMediaLib(cstr_t szDir, bool bIncSubDir)
{
    if (!m_spPlayer)
        return;

    FileFind        find;
    string            strDir;
    string            strFile;

    if (!find.openDir(szDir))
        return;

    strDir = szDir;
    dirStringAddSep(strDir);

    while (find.findNext())
    {
        if (find.isCurDir())
        {
            if (bIncSubDir && strcmp(".", find.getCurName()) != 0 &&
                strcmp("..", find.getCurName()) != 0)
            {
                strFile = strDir + find.getCurName();
                addDirToMediaLib(strFile.c_str(), bIncSubDir);
            }
        }
        else
        {
            strFile = strDir + find.getCurName();
            addFileToMediaLib(strFile.c_str());
        }
    }
}

void CPlayer::addFileToMediaLib(cstr_t szMedia)
{
    if (!m_spPlayer)
        return;

    MLRESULT        nRet;
    cstr_t            szExt;

    szExt = fileGetExt(szMedia);
    if (isEmptyString(szExt))
        return;
// 
//     if (isExtPlaylistFile(szExt))
//     {
//         loadPlaylist(szMedia, false);
//         return;
//     }

    if (!isExtAudioFile(szExt))
        return;

    // do not add repeated item.
    CMPAutoPtr<IMediaLibrary>    pMediaLib;
    CMPAutoPtr<IMedia>            pMedia;
    CMPAutoPtr<IPlaylist>        playlist;

    if (m_spPlayer->getMediaLibrary(&pMediaLib) != ERR_OK)
        return;

    if (pMediaLib->getMediaByUrl(szMedia, &pMedia) == ERR_OK)
        return;

    nRet = pMediaLib->add(szMedia, &pMedia);
    if (nRet == ERR_OK)
    {
        nRet = m_spPlayer->getCurrentPlaylist(&playlist);
        if (nRet == ERR_OK)
            playlist->insertItem(-1, pMedia);
    }
}

string CPlayer::formatMediaTitle(IMedia *pMedia)
{
    CXStr artist, title;
    pMedia->getArtist(&artist);
    pMedia->getTitle(&title);

    string strFullTitle = ::formatMediaTitle(artist.c_str(), title.c_str());
    if (strFullTitle.empty()) {
        CXStr file;
        pMedia->getSourceUrl(&file);
        strFullTitle = fileGetTitle(file.c_str());
    }

    return strFullTitle;
}

void CPlayer::filterLowRatingMedia(IPlaylist *playlist)
{
    for (int i = playlist->getCount() - 1; i >= 0; i--)
    {
        CMPAutoPtr<IMedia>        media;

        if (playlist->getItem(i, &media) == ERR_OK)
        {
            int rating;
            media->getAttribute(MA_RATING, &rating);
            if (rating < m_ratingFilterMin)
            {
                playlist->removeItem(i);
            }
        }
    }
}

string CPlayer::getMediaKey()
{
    return g_LyricSearch.getAssociateFileKeyword(m_szSrcMedia, m_szFullTitle);
}

MLRESULT CPlayer::getCurrentMedia(IMedia **ppMedia)
{
    if (m_spPlayer)
        return m_spPlayer->getCurrentMedia(ppMedia);
    else
        return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::getCurrentPlaylist(IPlaylist **ppPlaylist)
{
    if (m_spPlayer)
        return m_spPlayer->getCurrentPlaylist(ppPlaylist);
    else
        return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::newPlaylist(IPlaylist **ppPlaylist)
{
    if (m_spPlayer)
        return m_spPlayer->newPlaylist(ppPlaylist);
    else
        return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::setCurrentPlaylist(IPlaylist *pPlaylist)
{
    if (m_spPlayer)
        return m_spPlayer->setCurrentPlaylist(pPlaylist);
    else
        return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::setCurrentMediaInPlaylist(int nIndex)
{
    if (m_spPlayer)
        return m_spPlayer->setCurrentMediaInPlaylist(nIndex);
    else
        return ERR_NOT_IMPLEMENTED;
}

MLRESULT CPlayer::newMedia(IMedia **ppMedia, cstr_t szUrl)
{
    if (m_spPlayer)
        return m_spPlayer->newMedia(ppMedia, szUrl);
    else
        return ERR_NOT_IMPLEMENTED;
}

int CPlayer::unregisterVis(IVis *pVis)
{
    if (m_spPlayer)
        return m_spPlayer->unregisterVis(pVis);
    else
        return ERR_NOT_IMPLEMENTED;
}

int CPlayer::registerVis(IVis *pVis)
{
    if (m_spPlayer)
        return m_spPlayer->registerVis(pVis);
    else
        return ERR_NOT_IMPLEMENTED;
}

string formatTime(int nTimeSec);

void CPlayer::getCurMediaAttribute(MediaAttribute mediaAttr, string &strValue)
{
    if (mediaAttr == MA_ARTIST)
        strValue = m_szArtist;
    else if (mediaAttr == MA_ALBUM)
        strValue = m_szAlbum;
    else if (mediaAttr == MA_TITLE)
        strValue = m_szTitle;
    else if (mediaAttr == MA_DURATION)
    {
        if (m_nMediaLength > 0)
            strValue = formatTime(m_nMediaLength / 1000);
    }
    else if (m_spPlayer)
    {
        strValue.resize(0);

        CMPAutoPtr<IMedia>    media;
        int nRet = m_spPlayer->getCurrentMedia(&media);
        if (nRet == ERR_OK)
        {
            CXStr    str;
            nRet = media->getAttribute(mediaAttr, &str);
            if (nRet == ERR_OK)
                strValue = str.c_str();
        }
    }
    else
        strValue.resize(0);
}

void CPlayer::saveCurrentPlaylist()
{
    if (!m_bPlModified)
        return;

    CMPAutoPtr<IPlaylist>    playlist;

    if (getCurrentPlaylist(&playlist) == ERR_OK)
    {
        if (savePlaylistAsM3u(playlist, m_strCurrentPlaylist.c_str()))
        {
            setPlaylistModified(false);
            // SaveRecentPlList(m_strCurrentPlaylist.c_str());
            g_profile.writeString("LatestPlaylist", m_strCurrentPlaylist.c_str());
        }
    }
}

void CPlayer::saveCurrentPlaylistAs(cstr_t szFile)
{
    m_strCurrentPlaylist = szFile;
    saveCurrentPlaylist();
}

void CPlayer::uISeekbarOnSeek(int nPos)
{
    g_LyricData.SetPlayElapsedTime(nPos);

    CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);
}

string fixFileUri(cstr_t szUri)
{
    static cstr_t SZ_FILE_PREFIX        = "file://";
    static cstr_t SZ_LOCALHOST_MAC        = "localhost";
    size_t nLenPrefix = strlen(SZ_FILE_PREFIX);
    if (szUri && strncasecmp(szUri, SZ_FILE_PREFIX, nLenPrefix) == 0)
    {
        szUri += nLenPrefix;
        
        if (startsWith(szUri, SZ_LOCALHOST_MAC))
            // For mac: file://localhost/Volumes/
            szUri += strlen(SZ_LOCALHOST_MAC);
        else if (szUri[0] == '/' && szUri[2] == ':')
            // Songbird use file:///c:/xx...xxx.mp3 , remove the extra '/'
            szUri++;
        
        string uri = uriUnquote(szUri);
        strrep(uri, '/', PATH_SEP_CHAR);
        return uri;
    }
    else
        return szUri;
}

void CPlayer::onMediaChangedOfZikiPlayer()
{
    if (!m_spPlayer)
        return;

    MLRESULT                nRet;
    CMPAutoPtr<IMedia>        media;

    emptyStr(m_szSrcMedia);
    emptyStr(m_szFullTitle);
    emptyStr(m_szArtist);
    emptyStr(m_szTitle);
    emptyStr(m_szAlbum);
    m_nMediaLength = 0;

    nRet = m_spPlayer->getCurrentMedia(&media);
    if (nRet == ERR_OK)
    {
        CXStr        str;

        nRet = media->getSourceUrl(&str);
        if (nRet == ERR_OK)
            strcpy_safe(m_szSrcMedia, CountOf(m_szSrcMedia), str.c_str());

        m_nMediaLength = media->getDuration();

        nRet = media->getArtist(&str);
        if (nRet == ERR_OK)
            strcpy_safe(m_szArtist, CountOf(m_szArtist), str.c_str());

        nRet = media->getTitle(&str);
        if (nRet == ERR_OK)
            strcpy_safe(m_szTitle, CountOf(m_szTitle), str.c_str());

        nRet = media->getAlbum(&str);
        if (nRet == ERR_OK)
            strcpy_safe(m_szAlbum, CountOf(m_szAlbum), str.c_str());

        m_nMediaLength = media->getDuration();

        string            strFullTitle;
        formatFullTitle(m_szArtist, m_szTitle, m_szSrcMedia, strFullTitle);
        strcpy_safe(m_szFullTitle, CountOf(m_szFullTitle), strFullTitle.c_str());
    }
}

string CPlayer::getTitleFilterFile()
{
    return dirStringJoin(getAppDataDir().c_str(), "TitlFilters.txt");
}

bool CPlayer::setFieldValue(char szField[], int lenField, cstr_t szValue)
{
    if (szValue == nullptr)
        return false;

    string str;
    if (uriIsQuoted(szValue))
        str = uriUnquote(szValue);
    else
        str = szValue;

    if (szField != m_szSrcMedia)
    {
        for (size_t i = 0; i < m_vTitleFilters.size(); i++)
            strrep(str, m_vTitleFilters[i].c_str(), "");

        // remove regular file extension.
        static cstr_t vToTrim[] = { ".mp3", ".m4a", ".wma", ".flac", ".ogg", ".aac", ".mpg", ".mpeg", ".avi", ".mp4", ".mkv", ".rmvb", ".mov", };
        cstr_t szExt = fileGetExt(str.c_str());
        if (!isEmptyString(szExt))
        {
            for (int i = 0; i < CountOf(vToTrim); i++)
            {
                if (strcasecmp(szExt, vToTrim[i]) == 0)
                {
                    str.erase(str.size() - strlen(szExt), strlen(szExt));
                    break;
                }
            }
        }
    }

    if (strcmp(szField, str.c_str()) != 0)
    {
        strcpy_safe(szField, lenField, str.c_str());
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CPlayer)

class CTestCaseCPlayer : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCaseCPlayer);
//     CPPUNIT_TEST(TestUnquote);
    CPPUNIT_TEST_SUITE_END();

protected:
//     void TestUnquote()
//     {
//         cstr_t cases[] = { "%E5%BC%A0%E6%AF%85", "artist.movie.1080p.xxx", 
//             "", "%2%X1", };
// 
//         cstr_t results[] = { "张毅", "artist.movie.1080p.xxx", "", "%2%X1", };
// 
//         CPPUNIT_ASSERT(IsQuoted(cases[0]));
//         for (int i = 0; i < CountOf(cases); i++)
//         {
//             string r = Unquote(cases[i]);
//             CPPUNIT_ASSERT(strcmp(results[i], r.c_str()) == 0);
//         }
//     }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCPlayer);

#endif // _CPPUNIT_TEST
