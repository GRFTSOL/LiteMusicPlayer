#include "../MPlayerEngine/IMPlayer.h"
#include "MDWmpCore.h"
#include "MPTime.h"
#include "MPlayer.h"

class CWmpEventsSink : 
    public IWMPEvents
{
public:
    CWmpEventsSink(CMDWmpCore *pMDWmpCore)
    {
        m_mdWmpCore = pMDWmpCore;
        m_nReference = 0;
    }

    virtual HRESULT STDMETHODCALLTYPE queryInterface( 
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */ 
        __RPC__deref_out  void **ppvObject)
    {
        if (memcmp(&riid, &__uuidof(IWMPEvents), sizeof(UUID)) == 0)
            *ppvObject = (IWMPEvents *)this;
        else if (memcmp(&riid, &__uuidof(IUnknown), sizeof(UUID)) == 0)
            *ppvObject = (IUnknown *)this;
        else
        {
            *ppvObject = nullptr;
            return S_FALSE;
        }
        return S_OK;
    }

    virtual ULONG STDMETHODCALLTYPE addRef(void)
    {
        return interlockedIncrement(&m_nReference);
    }

    virtual ULONG STDMETHODCALLTYPE release(void)
    {
        long n = interlockedDecrement(&m_nReference);
        if (n == 0)
            delete this;
        return n;
    }

    long            m_nReference;

public:
    void STDMETHODCALLTYPE OpenStateChange( long NewState ) { }
    void STDMETHODCALLTYPE PlayStateChange( long NewState );
    void STDMETHODCALLTYPE AudioLanguageChange( long LangID ) { }
    void STDMETHODCALLTYPE StatusChange() { }
    void STDMETHODCALLTYPE ScriptCommand( BSTR scType, BSTR Param ) { }
    void STDMETHODCALLTYPE NewStream() { }
    void STDMETHODCALLTYPE disconnect( long Result ) { }
    void STDMETHODCALLTYPE Buffering( VARIANT_BOOL start ) { }
    void STDMETHODCALLTYPE Error() { }
    void STDMETHODCALLTYPE Warning( long WarningType, long Param, BSTR Description ) { }
    void STDMETHODCALLTYPE EndOfStream( long Result );
    void STDMETHODCALLTYPE PositionChange( double oldPosition, double newPosition) { }
    void STDMETHODCALLTYPE MarkerHit( long MarkerNum ) { }
    void STDMETHODCALLTYPE DurationUnitChange( long NewDurationUnit ) { }
    void STDMETHODCALLTYPE CdromMediaChange( long CdromNum ) { }
    void STDMETHODCALLTYPE PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change ) { }
    void STDMETHODCALLTYPE CurrentPlaylistChange( WMPPlaylistChangeEventType change ) { }
    void STDMETHODCALLTYPE CurrentPlaylistItemAvailable( BSTR bstrItemName ) { }
    void STDMETHODCALLTYPE MediaChange( IDispatch * Item ) { }
    void STDMETHODCALLTYPE CurrentMediaItemAvailable( BSTR bstrItemName ) { }
    void STDMETHODCALLTYPE CurrentItemChange( IDispatch *pdispMedia) { }
    void STDMETHODCALLTYPE MediaCollectionChange() { }
    void STDMETHODCALLTYPE MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal ) { }
    void STDMETHODCALLTYPE MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal ) { }
    void STDMETHODCALLTYPE MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal) { }
    void STDMETHODCALLTYPE PlaylistCollectionChange() { }
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName) { }
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName) { }
    void STDMETHODCALLTYPE PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted) { }
    void STDMETHODCALLTYPE ModeChange( BSTR ModeName, VARIANT_BOOL NewValue) { }
    void STDMETHODCALLTYPE MediaError( IDispatch * pMediaObject) { }
    void STDMETHODCALLTYPE OpenPlaylistSwitch( IDispatch *pItem ) { }
    void STDMETHODCALLTYPE DomainChange( BSTR strDomain) { }
    void STDMETHODCALLTYPE SwitchedToPlayerApplication() { }
    void STDMETHODCALLTYPE SwitchedToControl() { }
    void STDMETHODCALLTYPE PlayerDockedStateChange() { }
    void STDMETHODCALLTYPE PlayerReconnect() { }
    void STDMETHODCALLTYPE Click( short nButton, short nShiftState, long fX, long fY ) { }
    void STDMETHODCALLTYPE DoubleClick( short nButton, short nShiftState, long fX, long fY ) { }
    void STDMETHODCALLTYPE KeyDown( short nKeyCode, short nShiftState ) { }
    void STDMETHODCALLTYPE KeyPress( short nKeyAscii ) { }
    void STDMETHODCALLTYPE KeyUp( short nKeyCode, short nShiftState ) { }
    void STDMETHODCALLTYPE MouseDown( short nButton, short nShiftState, long fX, long fY ) { }
    void STDMETHODCALLTYPE MouseMove( short nButton, short nShiftState, long fX, long fY ) { }
    void STDMETHODCALLTYPE MouseUp( short nButton, short nShiftState, long fX, long fY ) { }

protected:
    CMPAutoPtr<CMDWmpCore>        m_mdWmpCore;

};

void CWmpEventsSink::PlayStateChange( long NewState )
{
    switch (NewState)
    {
    case wmppsUndefined:
        break;
    case wmppsStopped:
        if (m_mdWmpCore->m_mplayer)
            m_mdWmpCore->m_mplayer->notifyEod(m_mdWmpCore, ERR_OK);
        break;
    case wmppsPaused:
        break;
    case wmppsPlaying:
        break;
    case wmppsScanForward:
        break;
    case wmppsScanReverse:
        break;
    case wmppsBuffering:
        break;
    case wmppsWaiting:
        break;
    case wmppsMediaEnded:
        if (m_mdWmpCore->m_mplayer)
            m_mdWmpCore->m_mplayer->notifyEod(m_mdWmpCore, ERR_OK);
        break;
    case wmppsTransitioning:
        break;
    case wmppsReady:
        break;
    case wmppsReconnecting:
        break;
    case wmppsLast:
        break;
    default:
        break;
    }
}

void CWmpEventsSink::EndOfStream( long Result )
{
    if (m_mdWmpCore->m_mplayer)
        m_mdWmpCore->m_mplayer->notifyEod(m_mdWmpCore, ERR_OK);
}

//////////////////////////////////////////////////////////////////////////

CMDWmpCore::CMDWmpCore(void)
{
    OBJ_REFERENCE_INIT;

    m_dwAdviseCookie = 0;
    init();
}

CMDWmpCore::~CMDWmpCore(void)
{
    if (m_pConnectionPoint)
    {
        if (0 != m_dwAdviseCookie)
        {
            m_pConnectionPoint->Unadvise(m_dwAdviseCookie);
            m_dwAdviseCookie = 0;
        }
        m_pConnectionPoint = nullptr;
    }
}

MLRESULT hResultToMLResult(HRESULT hr)
{
    if (FAILED(hr))
        return ERR_FALSE;
    return ERR_OK;
}

LPCXSTR CMDWmpCore::getDescription()
{
    return "Windows Media Player Core";
}

LPCXSTR CMDWmpCore::getFileExtentions()
{
    return ".mp3|MP3 files|.wma|WMA files|.mp2|MP2 files";
}

MLRESULT CMDWmpCore::getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
    if (!isOK())
        return ERR_DECODER_INIT_FAILED;

    CMPAutoPtr<IWMPMedia>    media;
    HRESULT hr = m_collection->add((BSTR)pInput->getSource(), &media);
    if (FAILED(hr))
        return hResultToMLResult(hr);

    double duration;
    hr = media->get_duration(&duration);
    if (SUCCEEDED(hr))
        pMedia->setAttribute(MA_DURATION, (int)(duration * 1000));

    bstr_s    artist, album, title;

    BSTR    pbstrItemName = SysAllocString(L"Artist");
    hr = media->getItemInfo(pbstrItemName, &artist);
    SysFreeString(pbstrItemName);
    if (SUCCEEDED(hr))
        pMedia->setAttribute(MA_ARTIST, artist.c_str());

    pbstrItemName = SysAllocString(L"Album");
    hr = media->getItemInfo(pbstrItemName, &album);
    SysFreeString(pbstrItemName);
    if (SUCCEEDED(hr))
        pMedia->setAttribute(MA_ALBUM, album.c_str());

    pbstrItemName = SysAllocString(L"Title");
    hr = media->getItemInfo(pbstrItemName, &title);
    SysFreeString(pbstrItemName);
    if (SUCCEEDED(hr))
        pMedia->setAttribute(MA_TITLE, title.c_str());

    return ERR_OK;
}

//
// decode media file related methods
//

bool CMDWmpCore::isSeekable()
{
    return true;
}

bool CMDWmpCore::isUseOutputPlug()
{
    return false;
}

MLRESULT CMDWmpCore::play(IMPlayer *pPlayer, IMediaInput *pInput)
{
    if (!isOK())
        return ERR_DECODER_INIT_FAILED;

    m_mplayer = pPlayer;

    CMPAutoPtr<IWMPMedia>    media;
    HRESULT hr = m_collection->add((BSTR)pInput->getSource(), &media);
    if (FAILED(hr))
        return hResultToMLResult(hr);

    m_controls->stop();
    m_playlist->clear();
    m_playlist->appendItem(media);
    m_controls->put_currentItem(media);

    m_controls->playItem(media);

    return ERR_OK;
}

MLRESULT CMDWmpCore::pause()
{
    if (!isOK())
        return ERR_DECODER_INIT_FAILED;

    HRESULT hr = m_controls->pause();

    return hResultToMLResult(hr);
}

MLRESULT CMDWmpCore::unpause()
{
    if (!isOK())
        return ERR_DECODER_INIT_FAILED;

    WMPPlayState    state;
    HRESULT hr = m_player->get_playState(&state);
    if (FAILED(hr))
        return hResultToMLResult(hr);

    if (state == wmppsPaused)
        m_controls->play();

    return ERR_OK;
}

MLRESULT CMDWmpCore::stop()
{
    if (!isOK())
        return ERR_DECODER_INIT_FAILED;

    HRESULT hr = m_controls->stop();
    return hResultToMLResult(hr);
}

uint32_t CMDWmpCore::getLength()
{
    if (!isOK())
        return 0;

    CMPAutoPtr<IWMPMedia>    media;
    HRESULT hr = m_controls->get_currentItem(&media);
    if (FAILED(hr))
        return 0;

    double duration = 0;
    hr = media->get_duration(&duration);
    if (FAILED(hr))
        return 0;

    return uint32_t(duration * 1000);
}

MLRESULT CMDWmpCore::seek(uint32_t nPos)
{
    if (!isOK())
        return ERR_DECODER_INIT_FAILED;

    double pos = (double)nPos / 1000;
    HRESULT hr = m_controls->put_currentPosition(pos);
    return hResultToMLResult(hr);
}

uint32_t CMDWmpCore::getPos()
{
    if (!isOK())
        return 0;

    double pos = 0;
    HRESULT hr = m_controls->get_currentPosition(&pos);
    if (FAILED(hr))
        return 0;

    return (uint32_t)(pos * 1000);
}

MLRESULT CMDWmpCore::setVolume(int nVolume, int nBanlance)
{
    if (!isOK())
        return ERR_DECODER_INIT_FAILED;

    CMPAutoPtr<IWMPSettings>    settings;

    HRESULT hr = m_player->get_settings(&settings);
    if (FAILED(hr))
        return hResultToMLResult(hr);

    hr = settings->put_volume(nVolume);

    return ERR_OK;
}

bool CMDWmpCore::init()
{
    HRESULT hr = CoCreateInstance(__uuidof(WindowsMediaPlayer), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWMPPlayer), (void **)&m_player);
    if (FAILED(hr))
    {
        ERR_LOG1("Failed to create IWMPPlayer instance: %s", Error2Str(hr).c_str());
        return false;
    }

    CMPAutoPtr<IConnectionPointContainer> connectionPointContainer;
    hr = m_player->queryInterface(__uuidof(IConnectionPointContainer), (void **)&connectionPointContainer);
    if (FAILED(hr))
    {
        ERR_LOG1("Failed to query IConnectionPointContainer: %s", Error2Str(hr).c_str());
        return false;
    }

    hr = connectionPointContainer->FindConnectionPoint(__uuidof(IWMPEvents), &m_pConnectionPoint);
    if (FAILED(hr))
    {
        ERR_LOG1("Failed to FindConnectionPoint IWMPEvents: %s", Error2Str(hr).c_str());
        return false;
    }

    CWmpEventsSink *pSink = new CWmpEventsSink(this);
    pSink->addRef();
    hr = m_pConnectionPoint->Advise(pSink, &m_dwAdviseCookie);
    if ((FAILED(hr)) || (0 == m_dwAdviseCookie))
        m_pConnectionPoint = nullptr;

    hr = m_player->get_controls(&m_controls);
    if (FAILED(hr))
    {
        ERR_LOG1("Failed to get_controls : %s", Error2Str(hr).c_str());
        return false;
    }

    hr = m_player->get_currentPlaylist(&m_playlist);
    if (FAILED(hr))
    {
        ERR_LOG1("Failed to get_currentPlaylist : %s", Error2Str(hr).c_str());
        return false;
    }

    hr = m_player->get_mediaCollection(&m_collection);
    if (FAILED(hr))
    {
        ERR_LOG1("Failed to get_mediaCollection : %s", Error2Str(hr).c_str());
        return false;
    }

    return true;
}

MLRESULT CMDWmpCore::doDecode(IMedia *pMedia)
{
    if (!isOK())
        return ERR_DECODER_INIT_FAILED;

    MLRESULT                    nRet;
    CXStr                        strMedia;
    CMPAutoPtr<IMediaInput>        pInput;
    CMPTime        timePlayed;

    nRet = pMedia->getSourceUrl(&strMedia);
    if (nRet != ERR_OK)
        return nRet;

    nRet = m_pPlayer->m_pluginMgrAgent.newInput(strMedia.c_str(), &pInput);
    if (nRet != ERR_OK)
        return nRet;

    getMediaInfo(m_pPlayer, pInput, pMedia);

    nRet = play(m_pPlayer, pInput);
    if (nRet != ERR_OK)
    {
        if (nRet == ERR_MI_NOT_FOUND && pMedia->getID() != MEDIA_ID_INVALID)
        {
            // the source can't be opened, set it as deleted.
            m_pPlayer->m_pMediaLib->setDeleted(&pMedia);
        }
        goto R_FAILED;
    }

    if (m_pPlayer->m_nVolume != -1)
    {
        if (m_pPlayer->m_bMute)
            setVolume(0, 0);
        else
            setVolume(m_pPlayer->m_nVolume, m_pPlayer->m_nBalance);
    }

    // set the play time
    timePlayed.getCurrentTime();
    pMedia->setAttribute(MA_TIME_PLAYED, timePlayed.m_time);

    if (pMedia->getID() == MEDIA_ID_INVALID && m_pPlayer->isAutoAddToMediaLib())
        m_pPlayer->m_pMediaLib->add(pMedia);

    // If media info in media library isn't up to date,
    // update to media library.
    if (pMedia->getID() != MEDIA_ID_INVALID && !pMedia->isInfoUpdatedToMediaLib())
    {
        m_pPlayer->m_pMediaLib->updateMediaInfo(pMedia);
    }

    return ERR_OK;

R_FAILED:
    return nRet;
}

bool CMDWmpCore::isOK()
{
    return m_player != nullptr && m_playlist != nullptr && m_controls != nullptr && m_collection != nullptr;
}
