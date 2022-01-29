// MPlayer.cpp: implementation of the CMPlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayer.h"
#include "MILocalFile.h"
#include "../LyricsLib/HelperFun.h"
// #include "MDWave.h"

#ifdef _WIN32
#include "win32/MOSoundCard.h"
#include "MDWmpCore.h"
#endif

#ifdef _LINUX
#include "linux/MOSoundCard.h"
#endif

#ifdef _MAC_OS
#include "MDAVPlayer.h"
#else
// #include "MOAnalyse.h"
#include "MDRow.h"
#include "MDWave.h"
#include "MDLibmad.h"

#ifdef _WIN32
#include "VISDemo.h"
#endif

// #include "DspDemo.h"
#include "MORaw.h"
#include "../MPlayer-Plugins/Dsp_SuperEQ/dsp_supereq.h"
#endif // #ifdef _MAC_OS

#define SIZE_INC (1024 * 4) 


// MLRESULT GetPlayerInstance(IMPlayer **ppPlayer);
// {
//     return CMPlayer::getInstance(ppPlayer);
// }

//////////////////////////////////////////////////////////////////////////
// CFBuffer

CFBuffer::CFBuffer(CMemAllocator *pMemAllocator, uint32_t nCapacity)
{
    OBJ_REFERENCE_INIT
    m_buf = nullptr;
    m_pMemAllocator = pMemAllocator;
    m_nCapacity = nCapacity;
    m_nSize = 0;

    m_buf = (char *)malloc(nCapacity);

    m_nCapacity = nCapacity;
    m_nSize = 0;

    m_pMemAllocator->addRef();
}

CFBuffer::~CFBuffer()
{
    if (m_buf)
        free(m_buf);

    m_pMemAllocator->release();
}

void CFBuffer::addRef()
{
    m_nReference++;
}

void CFBuffer::release()
{
    if (--m_nReference <= 0)
    {
        m_pMemAllocator->onRelease(this);
    }
}

char *CFBuffer::data()
{
    return m_buf;
}

uint32_t CFBuffer::size()
{
    return m_nSize;
}

uint32_t CFBuffer::capacity()
{
    return m_nCapacity;
}

void CFBuffer::resize(uint32_t nSize)
{
    assert(nSize <= m_nCapacity);
    if (nSize > m_nCapacity)
        nSize = m_nCapacity;
    m_nSize = nSize;
}

MLRESULT CFBuffer::reserve(uint32_t nCapacity)
{
    // assert(m_nRef == 1);
    if (nCapacity > m_nCapacity)
    {
        m_buf = (char *)realloc(m_buf, nCapacity);
        assert(m_buf);
        if (!m_buf)
            return ERR_NO_MEM;

        m_nCapacity = nCapacity;
    }

    return ERR_OK;
}

CMemAllocator::CMemAllocator()
{
    OBJ_REFERENCE_INIT;
}

CMemAllocator::~CMemAllocator()
{
    assert(m_listFree.size() == 0);
}

void CMemAllocator::quit()
{
    LIST_BUF::iterator    it;

    for (it = m_listFree.begin(); it != m_listFree.end(); ++it)
    {
        CFBuffer        *pBuffer = (CFBuffer*)(IFBuffer*)(*it);

        delete pBuffer;
        // pBuffer->release();
    }
    m_listFree.clear();
}

IFBuffer *CMemAllocator::allocFBuffer(uint32_t nCapacity)
{
    LIST_BUF::iterator    it;
    RMutexAutolock autolock(m_mutex);

    for (it = m_listFree.begin(); it != m_listFree.end(); ++it)
    {
        IFBuffer        *pBuffer = *it;
        if (pBuffer->capacity() >= nCapacity)
        {
            pBuffer->addRef();
            m_listFree.erase(it);
            return pBuffer;
        }
    }

    CFBuffer    *pBuffer;
    pBuffer = new CFBuffer(this, nCapacity);
    pBuffer->addRef();

    static int    nCount = 0;
    nCount++;
    DBG_LOG1("mallocated mem count: %d", nCount);

    return pBuffer;
}

IString *CMemAllocator::allocStr()
{
    return new CXStr();
}

void CMemAllocator::onRelease(IFBuffer *pBuf)
{
    pBuf->resize(0);
    RMutexAutolock    autolock(m_mutex);
    m_listFree.push_front(pBuf);
}

//////////////////////////////////////////////////////////////////////////
CMOAgent::CMOAgent()
{
    OBJ_REFERENCE_INIT;
}

CMOAgent::~CMOAgent()
{
}

cstr_t CMOAgent::getDescription()
{
    if (m_pMediaOutput)
        return m_pMediaOutput->getDescription();
    else
        return "No output device is available.";
}

MLRESULT CMOAgent::init(IMPlayer *pPlayer)
{
    return ERR_OK;
}

MLRESULT CMOAgent::quit()
{
    return ERR_OK;
}

MLRESULT CMOAgent::open(int nSampleRate, int nNumChannels, int nBitsPerSamp)
{
    if (m_pMediaOutput)
        return m_pMediaOutput->open(nSampleRate, nNumChannels, nBitsPerSamp);
    else
        return ERR_NO_DEVICE;
}

MLRESULT CMOAgent::waitForWrite()
{
    if (m_pMediaOutput)
        return m_pMediaOutput->waitForWrite();
    else
        return ERR_NO_DEVICE;
}

MLRESULT CMOAgent::write(IFBuffer *pBuf)
{
    if (m_pMediaOutput)
        return m_pMediaOutput->write(pBuf);
    else
        return ERR_NO_DEVICE;
}

MLRESULT CMOAgent::flush()
{
    if (m_pMediaOutput)
        return m_pMediaOutput->flush();
    else
        return ERR_NO_DEVICE;
}

MLRESULT CMOAgent::pause(bool bPause)
{
    if (m_pMediaOutput)
        return m_pMediaOutput->pause(bPause);
    else
        return ERR_NO_DEVICE;
}

bool CMOAgent::isPlaying()
{
    if (m_pMediaOutput)
        return m_pMediaOutput->isPlaying();
    else
        return false;
}

MLRESULT CMOAgent::stop()
{
    if (m_pMediaOutput)
        return m_pMediaOutput->stop();
    else
        return ERR_NO_DEVICE;
}

bool CMOAgent::isOpened()
{
    if (m_pMediaOutput)
        return m_pMediaOutput->isOpened();
    else
        return false;
}

uint32_t CMOAgent::getPos()
{
    if (m_pMediaOutput)
        return m_pMediaOutput->getPos();
    else
        return 0;
}

// volume
MLRESULT CMOAgent::setVolume(int nVolume, int nBanlance)
{
    if (m_pMediaOutput)
        return m_pMediaOutput->setVolume(nVolume, nBanlance);
    else
        return ERR_NO_DEVICE;
}

//////////////////////////////////////////////////////////////////////////

MLRESULT CMPluginManagerAgent::detectPlugins()
{
    if (m_pluginMgr)
        m_pluginMgr->detectPlugins();

    //
    // register internal plugins
    //
    IMediaDecode    *pDecoder;
    
#ifdef _MAC_OS
    pDecoder = new CMDAVPlayer;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();
#else
#ifdef _MPLAYER
    IMPlayer *player = nullptr;
    CMPlayer::getInstance(&player);

#ifdef _WIN32
    IVis *pVis = new CVISDemo();
    pVis->init(player);
    player->registerVis(pVis);
#endif

    pDecoder = new CMDLibmad;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();

    pDecoder = new CMDWave;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();

    pDecoder = new CMDRow;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();
#else
    pDecoder = new CMDWmpCore;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();
#endif

#endif // #ifndef _MAC_OS

    return ERR_OK;
}


MLRESULT CMPluginManagerAgent::newInput(cstr_t szMediaUrl, IMediaInput **ppInput)
{
    if (m_pluginMgr && m_pluginMgr->newInput(szMediaUrl, ppInput) == ERR_OK)
        return ERR_OK;

    MLRESULT                nRet;

    *ppInput = new CMILocalFile;

    nRet = (*ppInput)->open(szMediaUrl);
    if (nRet != ERR_OK)
    {
        delete *ppInput;
        *ppInput = nullptr;
        return nRet;
    }

    (*ppInput)->addRef();

    return ERR_OK;
}


MLRESULT CMPluginManagerAgent::newDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder)
{
    assert(pInput);

    *ppDecoder = nullptr;

#ifndef _MAC_OS
    cstr_t szMediaUrl = pInput->getSource();

#ifdef _MPLAYER
    if (fileIsExtSame(szMediaUrl, ".mp3"))
        *ppDecoder = new CMDLibmad;
    else if (fileIsExtSame(szMediaUrl, ".wav"))
        *ppDecoder = new CMDWave;
    else if (fileIsExtSame(szMediaUrl, ".raw"))
        *ppDecoder = new CMDRow;
    else if (m_pluginMgr)
        return m_pluginMgr->newDecoder(pInput, ppDecoder);
#else
    if (fileIsExtSame(szMediaUrl, ".mp3"))
        *ppDecoder = new CMDLibmad;
#endif

#endif // #ifndef _MAC_OS

    if (*ppDecoder)
    {
        (*ppDecoder)->addRef();
        return ERR_OK;
    }
    else
        return ERR_NOT_SUPPORT_FILE_FORMAT;
}


MLRESULT CMPluginManagerAgent::newOutput(IMediaOutput **ppOutput)
{
    if (m_pluginMgr && m_pluginMgr->newOutput(ppOutput) == ERR_OK)
        return ERR_OK;

#ifndef _MAC_OS
    *ppOutput = new CMOSoundCard;
#endif // #ifndef _MAC_OS

    return ERR_OK;
}


MLRESULT CMPluginManagerAgent::getActiveDSP(IDSP **ppDSP)
{
    MLRESULT mlRes = ERR_NOT_SUPPORT;
    if (m_pluginMgr)
        mlRes = m_pluginMgr->getActiveDSP(ppDSP);
    
    if (mlRes == ERR_NOT_SUPPORT)
    {
#ifdef _MPLAYER
        *ppDSP = CDspSuperEQ::getInstance();
        return ERR_OK;
#endif

        //*ppDSP = new CDspDemo;
        //(*ppDSP)->addRef();

        return ERR_FALSE;
    }

    return mlRes;
}


MLRESULT CMPluginManagerAgent::getActiveVis(IVector *pvVis)
{
    if (m_pluginMgr)
        return m_pluginMgr->getActiveVis(pvVis);
    else
        return ERR_FALSE;
}

//////////////////////////////////////////////////////////////////////

CMPlayer        *CMPlayer::m_spPlayer = nullptr;

CMPlayer::CMPlayer()
{
    OBJ_REFERENCE_INIT;

    m_state = PS_STOPED;

#ifdef _MAC_OS
    m_pMDAgent = new CMDAVPlayer();
#else // #ifdef _MAC_OS

#ifdef _MPLAYER
    m_pMDAgent = new CMDAgent();
#else
    m_pMDAgent = new CMDWmpCore;
#endif
#endif // #ifdef _MAC_OS
    m_pMDAgent->init(this);

    m_pMOAgent = new CMOAgent;
    m_pMOAgent->init(this);

    m_pMemAllocator = new CMemAllocator;

    m_pMediaLib = new CMediaLibrary;
    m_pMediaLib->init(this);

    m_bShuffle = false;
    m_bMute = false;
    m_loopMode = MP_LOOP_OFF;
    m_nVolume = -1;
    m_nBalance = 0;

    memset(&m_Equalizer, 0, sizeof(m_Equalizer));

    m_currentPlaylist = new CPlaylist(this);
    m_currentPlaylist->addRef();

    m_nCurrentMedia = 0;
    m_pCurrentMedia = nullptr;
    m_bCurMediaPlayed = false;

    m_bAutoAddToMediaLib = true;

    m_nBufferedVisMs = 0;
    m_nMODelay = 1000;

    m_bQuitVis = false;
    m_threadVis = nullptr;

    srand(getTickCount());
}

CMPlayer::~CMPlayer()
{
    stopVis();

    {
        RMutexAutolock lock(m_mutexVisdataAccess);
        V_VIS::iterator    it, itEnd;
        itEnd = m_vVis.end();
        for (it = m_vVis.begin(); it != m_vVis.end(); ++it)
        {
            IVis    *pVis = *it;
            pVis->quit();
            pVis->release();
        }
    }

    {
        RMutexAutolock lock(m_mutexDSP);
        if (m_pDsp)
        {
            m_pDsp->quit();
            m_pDsp.release();
        }
    }

    m_pMDAgent.release();

    m_pMOAgent.release();

    if (m_pCurrentMedia)
        m_pCurrentMedia->release();

    if (m_currentPlaylist)
        m_currentPlaylist->release();

    if (m_pMediaLib)
    {
        m_pMediaLib->close();
        m_pMediaLib.release();
    }

    if (m_pMemAllocator)
    {
        m_pMemAllocator->quit();
        m_pMemAllocator.release();
    }
}

MLRESULT CMPlayer::setPluginManager(IMPluginManager *pPluginMgr)
{
    if (m_pluginMgrAgent.m_pluginMgr)
        m_pluginMgrAgent.m_pluginMgr.release();

    // stop and unregister all Vis
    stopVis();

    {
        RMutexAutolock lock(m_mutexDSP);
        V_VIS::iterator    it, itEnd;
        itEnd = m_vVis.end();
        for (it = m_vVis.begin(); it != m_vVis.end(); ++it)
        {
            IVis    *pVis = *it;
            pVis->quit();
            pVis->release();
        }
    }

    m_pluginMgrAgent.m_pluginMgr = pPluginMgr;
    m_pluginMgrAgent.detectPlugins();

    //
    // set DSP
    //
    {
        RMutexAutolock lock(m_mutexDSP);
        if (m_pDsp)
            m_pDsp.release();
        if (m_pluginMgrAgent.getActiveDSP(&m_pDsp) == ERR_OK)
            m_pDsp->init(this);
    }

    //
    // set Vis
    //

    // register new vis
    CVector        vVis;
    m_pluginMgrAgent.getActiveVis(&vVis);
    for (int i = 0; i < (int)vVis.size(); i++)
    {
        IVis    *pVis = (IVis*)vVis.at(i);

        pVis->init(this);
        registerVis(pVis);
    }

    return ERR_OK;
}

MLRESULT CMPlayer::queryInterface(MPInterfaceType interfaceType, void **lpInterface)
{
    switch (interfaceType)
    {
    case MPIT_OUTPUT:
        *lpInterface = (IMediaOutput*)m_pMOAgent;
        m_pMOAgent->addRef();
        break;
    case MPIT_DECODE:
        *lpInterface = (IMediaDecode*)m_pMDAgent;
        m_pMDAgent->addRef();
        break;
    case MPIT_DSP:
        {
            RMutexAutolock autolock(m_mutexDSP);
            if (m_pDsp)
            {
                *lpInterface = m_pDsp;
                m_pDsp->addRef();
            }
            else
                return ERR_NOT_FOUND;
        }
        break;
    case MPIT_MEMALLOCATOR:
        *lpInterface = (IMemAllocator*)m_pMemAllocator;
        m_pMemAllocator->addRef();
        break;
    case MPIT_MEDIA_LIB:
        *lpInterface = (IMediaLibrary*)m_pMediaLib;
        m_pMediaLib->addRef();
        break;
    default:
        return ERR_NOT_FOUND;
        break;
    }

    return ERR_OK;
}

//
// Player control
//

MLRESULT CMPlayer::play()
{
    if (m_state == PS_PLAYING)
    {
        return seek(0);
    }
    else if (m_state == PS_STOPED)
    {
        if (m_pCurrentMedia)
        {
            int        nRet;
            m_bAutoPlayNext = true;
            m_bCurMediaPlayed = true;
            nRet = m_pMDAgent->doDecode(m_pCurrentMedia);

            if (nRet == ERR_OK)
            {
                m_state = PS_PLAYING;
                notifyPlayStateChanged();
            }

            return nRet;
        }
        else
        {
            notifyPlayStateChanged();
            return ERR_EMPTY_PLAYLIST;
        }
    }
    else
    {
        return unpause();
    }
}

MLRESULT CMPlayer::pause()
{
    MLRESULT        ret;
    if (m_state != PS_PLAYING)
        return ERR_OK;

    ret = m_pMDAgent->pause();
    if (ret == ERR_OK)
    {
        m_state = PS_PAUSED;
        notifyPlayStateChanged();
    }

    return ret;
}

MLRESULT CMPlayer::unpause()
{
    MLRESULT        ret;
    if (m_state != PS_PAUSED)
        return ERR_OK;

    ret = m_pMDAgent->unpause();
    if (ret == ERR_OK)
    {
        m_state = PS_PLAYING;
        notifyPlayStateChanged();
    }

    return ret;
}

MLRESULT CMPlayer::stop()
{
    m_bAutoPlayNext = false;

    return doStop();
}

MLRESULT CMPlayer::prev()
{
    long nOld = m_nCurrentMedia;

    {
        RMutexAutolock autolock(m_mutexDataAccess);

        if (m_bShuffle)
        {
            if (m_nCurrentShuffleMedia <= 1)
            {
                // in shuffle mode, only can back to 0
                return ERR_END_OF_PLAYLIST;
            }

            assert(m_vShuffleMedia.size() == m_currentPlaylist->getCount());
            m_nCurrentShuffleMedia--;
            m_nCurrentMedia = m_vShuffleMedia[m_nCurrentShuffleMedia - 1];
            // m_nCurrentMedia = rand() % m_currentPlaylist->getCount();
        }
        else
        {
            if (m_nCurrentMedia > 0)
                m_nCurrentMedia--;
            else if (m_loopMode == MP_LOOP_ALL)
                m_nCurrentMedia = m_currentPlaylist->getCount() - 1;
        }
    }

    if (nOld != m_nCurrentMedia)
    {
        // Notify changed
        currentMediaChanged();
    }

    return ERR_OK;
}

MLRESULT CMPlayer::next()
{
    if (m_state != PS_STOPED)
    {
        // No rating consideration will be done at all if the song is skipped within these seconds. 
        if (getPos() < 20 * 1000)
            m_pMediaLib->markPlaySkipped(m_pCurrentMedia);
    }

    return doNext(true);
}

MLRESULT CMPlayer::seek(uint32_t dwPos)
{
    if (m_pMDAgent)
    {
        MLRESULT    nRet;
        if ((int)dwPos < -1)
            dwPos = 0;
        nRet = m_pMDAgent->seek(dwPos);
        if (nRet == ERR_OK)
            notifySeek();

        return nRet;
    }
    else
        return ERR_PLAYER_INVALID_STATE;
}

MLRESULT CMPlayer::newMedia(IMedia **ppMedia, cstr_t szUrl)
{
    assert(ppMedia);

    int            nRet;

    nRet = m_pMediaLib->getMediaByUrl(szUrl, ppMedia);
    if (nRet != ERR_OK)
    {
        *ppMedia = new CMedia;
        if (*ppMedia == nullptr)
            return ERR_NO_MEM;

        (*ppMedia)->addRef();
        (*ppMedia)->setSourceUrl(szUrl);
    }

    return ERR_OK;
}

MLRESULT CMPlayer::newPlaylist(IPlaylist **ppPlaylist)
{
    *ppPlaylist = new CPlaylist(this);
    if (!*ppPlaylist)
        return ERR_NO_MEM;

    (*ppPlaylist)->addRef();

    return ERR_OK;
}

MLRESULT CMPlayer::getMediaLibrary(IMediaLibrary **ppMediaLib)
{
    assert(m_pMediaLib);
    *ppMediaLib = m_pMediaLib;
    m_pMediaLib->addRef();

    return ERR_OK;
}

MLRESULT CMPlayer::getCurrentPlaylist(IPlaylist **ppPlaylist)
{
    RMutexAutolock autolock(m_mutexDataAccess);

    m_currentPlaylist->addRef();
    *ppPlaylist = m_currentPlaylist;

    return ERR_OK;
}

MLRESULT CMPlayer::getCurrentMedia(IMedia **ppMedia)
{
    if (m_pCurrentMedia)
    {
        m_pCurrentMedia->addRef();
        *ppMedia = m_pCurrentMedia;

        return ERR_OK;
    }
    else
        return ERR_NOT_FOUND;
    // return m_currentPlaylist->getItem(m_nCurrentMedia, ppMedia);
}

long CMPlayer::getCurrentMediaInPlaylist()
{
    return m_nCurrentMedia;
}

MLRESULT CMPlayer::setCurrentMediaInPlaylist(long nIndex)
{
    RMutexAutolock autolock(m_mutexDataAccess);

    if (nIndex >= 0 && nIndex < (int)m_currentPlaylist->getCount())
    {
        m_nCurrentMedia = nIndex;
        autolock.unlock();
        currentMediaChanged();
        return ERR_OK;
    }
    else
    {
        return ERR_NOT_FOUND;
    }
}

MLRESULT CMPlayer::setCurrentPlaylist(IPlaylist *pPlaylist)
{
    {
        RMutexAutolock autolock(m_mutexDataAccess);
        if (m_currentPlaylist)
            m_currentPlaylist->release();
        m_currentPlaylist = (CPlaylist *)pPlaylist;
        m_currentPlaylist->addRef();
        m_nCurrentMedia = 0;
    }

    generateShuffleMediaQueue();

    notifyCurrentPlaylistChanged(IMPEvent::PCA_FULL_UPDATE, 0, 0);

    currentMediaChanged();

    return ERR_OK;
}

MLRESULT CMPlayer::setCurrentMedia(IMedia *pMedia)
{
    MLRESULT        nRet;

    if (m_state != PS_STOPED)
    {
        nRet = doStop();
        if (nRet != ERR_OK && m_state != PS_STOPED)
            return nRet;
    }

    {
        RMutexAutolock autolock(m_mutexDataAccess);
        if (m_currentPlaylist)
            m_currentPlaylist->release();

        m_currentPlaylist = new CPlaylist(this);
        m_currentPlaylist->addRef();
        nRet = m_currentPlaylist->insertItem(-1, pMedia);
        if (nRet != ERR_OK)
            return nRet;
    }

    m_nCurrentMedia = 0;

    generateShuffleMediaQueue();
    notifyCurrentPlaylistChanged(IMPEvent::PCA_FULL_UPDATE, 0, 0);

    currentMediaChanged();

    return play();
}

MLRESULT CMPlayer::setCurrentMedia(cstr_t szSourceMedia)
{
    MLRESULT            nRet;
    CMPAutoPtr<IMedia>    pMedia;

    nRet = newMedia(&pMedia, szSourceMedia);
    if (nRet != ERR_OK)
        return nRet;

    nRet = setCurrentMedia(pMedia);
    if (m_state != PS_PLAYING)
        play();

    return nRet;
}

//
// Current playing Media state
//
uint32_t CMPlayer::getLength()
{
    RMutexAutolock autolock(m_mutexDataAccess);

    if (m_pMDAgent)
        return m_pMDAgent->getLength();
    else
    {
        IMedia        *pMedia;
        if (m_currentPlaylist->getItem(m_nCurrentMedia, &pMedia) == ERR_OK)
        {
            int            value = 0;
            if (pMedia->getAttribute(MA_DURATION, &value) == ERR_OK)
                return value;
            else
                return 0;
        }
        else
            return 0;
    }
}

uint32_t CMPlayer::getPos()
{
    RMutexAutolock autolock(m_mutexDataAccess);

    if (m_pMDAgent)
        return m_pMDAgent->getPos();
    else
        return 0;
}

PLAYER_STATE CMPlayer::getState()
{
    return m_state;
}

void CMPlayer::outputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate)
{
    {
        RMutexAutolock lock(m_mutexDSP);
        if (m_pDsp)
            m_pDsp->process(pBuf, nBps, nChannels, nSampleRate);
    }

    if (isVisActive())
        addVisData(pBuf, nBps, nChannels, nSampleRate);

    m_pMOAgent->write(pBuf);
}
//
// Player settings
//
void CMPlayer::setShuffle(bool bShuffle)
{
    m_bShuffle = bShuffle;
    notifySettingsChanged(IMPEvent::MPS_SHUFFLE, m_bShuffle);
}

void CMPlayer::setLoop(MP_LOOP_MODE loopMode)
{
    m_loopMode = loopMode;
    notifySettingsChanged(IMPEvent::MPS_LOOP, loopMode);
}

bool CMPlayer::getShuffle()
{
    return m_bShuffle;
}

MP_LOOP_MODE CMPlayer::getLoop()
{
    return m_loopMode;
}

void CMPlayer::setMute(bool bMute)
{
    m_bMute = bMute;

    if (m_pMDAgent)
    {
        if (bMute)
            m_pMDAgent->setVolume(0, 0);
        else if (m_nVolume != -1)
            m_pMDAgent->setVolume(m_nVolume, m_nBalance);
    }

    notifySettingsChanged(IMPEvent::MPS_MUTE, m_bMute);
}

bool CMPlayer::getMute()
{
    return m_bMute;
}

// 0 ~ 100
MLRESULT CMPlayer::setVolume(long nVolume)
{
    assert(nVolume >= 0 && nVolume <= 100);
    if (nVolume < 0)
        nVolume = 0;
    else if (nVolume > 100)
        nVolume = 100;
    m_nVolume = nVolume;
    notifySettingsChanged(IMPEvent::MPS_VOLUME, m_nVolume);
    if (m_pMDAgent)
        return m_pMDAgent->setVolume(nVolume, m_nBalance);
    else
        return ERR_OK;
}

long CMPlayer::getVolume()
{
    return m_nVolume;
}

// -100 ~ 100
MLRESULT CMPlayer::setBalance(long nBalance)
{
    assert(nBalance >= -100 && nBalance <= 100);
    if (nBalance < -100)
        nBalance = -100;
    else if (nBalance > 100)
        nBalance = 100;
    m_nBalance = nBalance;
    notifySettingsChanged(IMPEvent::MPS_BALANCE, m_nBalance);
    if (m_pMDAgent && m_nVolume != -1)
        return m_pMDAgent->setVolume(m_nVolume, nBalance);
    else
        return ERR_OK;
}

long CMPlayer::getBalance()
{
    return m_nBalance;
}

MLRESULT CMPlayer::setEQ(const EQualizer *eq)
{
    m_Equalizer = *eq;
    notifyEQSettingsChanged(&m_Equalizer);
    return ERR_OK;
}

MLRESULT CMPlayer::getEQ(EQualizer *eq)
{
    *eq = m_Equalizer;
    return ERR_OK;
}

//
// Event tracer
//
void CMPlayer::registerEvent(IMPEvent *pEventHandler)
{
    RMutexAutolock    autolock(m_mutexEventHandler);

    pEventHandler->addRef();
    m_listEventHandler.push_back(pEventHandler);
}

void CMPlayer::unregisterEvent(IMPEvent *pEventHandler)
{
    RMutexAutolock    autolock(m_mutexEventHandler);

    list<IMPEvent *>::iterator it, itEnd;
    itEnd = m_listEventHandler.end();
    for (it = m_listEventHandler.begin(); it != itEnd; ++it)
    {
        if (pEventHandler == *it)
        {
            m_listEventHandler.erase(it);
            pEventHandler->release();
            return;
        }
    }
}

void CMPlayer::notifyEod(IMediaDecode *pDecoder, MLRESULT nError)
{
    bool bSendNotify = m_state != PS_STOPED;

    m_state = PS_STOPED;

    m_pMDAgent->notifyEod(pDecoder, nError);

    if (m_bAutoPlayNext)
    {
        if (m_loopMode == MP_LOOP_TRACK)
        {
            play();
            return;
        }

        MLRESULT    nRet;
        nRet = doNext();
        if (nRet == ERR_OK)
        {
            nRet = play();
            if (nRet == ERR_MI_OPEN_SRC && nError == ERR_OK)
            {
                // if (ERR_OPEN_FILE && last_file_can be played)
                //        try the other medias.
                int            i, nTryMax;
                nTryMax = m_currentPlaylist->getCount();
                for (i = 1; m_bAutoPlayNext && i < nTryMax 
                    && nRet == ERR_MI_OPEN_SRC && doNext() == ERR_OK; i++)
                {
                    nRet = play();
                    Sleep(500);
                }
            }
        }
        else
        {
            if (m_bCurMediaPlayed)
                m_pMediaLib->markPlayFinished(m_pCurrentMedia);
        }
    }

    if (bSendNotify)
        notifyPlayStateChanged();
}


MLRESULT CMPlayer::makeOutputReadyForDecode()
{
    MLRESULT        nRet;

    if (!m_pMOAgent->m_pMediaOutput)
    {
        nRet = m_pluginMgrAgent.newOutput(&(m_pMOAgent->m_pMediaOutput));
        if (nRet != ERR_OK)
            return nRet;

        m_pMOAgent->m_pMediaOutput->init(this);
    }

    return ERR_OK;
}

MLRESULT CMPlayer::loadMediaTagInfo(IMedia *pMedia, bool bForceReload)
{
    MLRESULT                nRet;
    CXStr        url;

    nRet = pMedia->getSourceUrl(&url);
    if (nRet != ERR_OK)
        return ERR_OK;

    CMPAutoPtr<IMediaInput>        pInput;

    nRet = m_pluginMgrAgent.newInput(url.c_str(), &pInput);
    if (nRet != ERR_OK)
        return nRet;

    nRet = m_pMDAgent->getMediaInfo(this, pInput, pMedia);
    if (nRet != ERR_OK)
        return nRet;

    CXStr    artist, title;

    pMedia->getArtist(&artist);
    pMedia->getArtist(&title);

    // get artist, title from song file name.
    if (artist.size() == 0 && title.size() == 0)
    {
        string        strArtist, strTitle;
        analyseLyricsFileNameEx(strArtist, strTitle, url.c_str());

        pMedia->setAttribute(MA_ARTIST, strArtist.c_str());
        pMedia->setAttribute(MA_TITLE, strTitle.c_str());
    }

    return ERR_OK;
}

// Vis:
MLRESULT CMPlayer::registerVis(IVis *pVis)
{
    RMutexAutolock autolock(m_mutexVisdataAccess);
    V_VIS::iterator it, itEnd;

    itEnd = m_vVis.end();
    for (it = m_vVis.begin(); it != m_vVis.end(); ++it) {
        if (*it == pVis)
            return ERR_EXIST;
    }

    pVis->addRef();
    m_vVis.push_back(pVis);

    if (m_vVis.size() == 1) {
        autolock.unlock();
        startVis();
    }

    return ERR_OK;
}

MLRESULT CMPlayer::unregisterVis(IVis *pVis)
{
    RMutexAutolock autolock(m_mutexVisdataAccess);
    V_VIS::iterator    it, itEnd;

    itEnd = m_vVis.end();
    for (it = m_vVis.begin(); it != m_vVis.end(); ++it)
    {
        if (*it == pVis)
        {
            pVis->quit();
            pVis->release();
            m_vVis.erase(it);
            if (m_vVis.empty())
            {
                autolock.unlock();
                stopVis();
            }
            return ERR_OK;
        }
    }

    return ERR_NOT_FOUND;
}

// Dsp:
MLRESULT CMPlayer::registerDsp(IDSP *pVis)
{
    return ERR_OK;
}

MLRESULT CMPlayer::unregisterDsp(IDSP *pVis)
{
    return ERR_OK;
}

MLRESULT CMPlayer::doStop()
{
    if (m_state == PS_STOPED)
        return ERR_OK;

    // use a temp MediaDecode with addRef for thread safe
    return m_pMDAgent->stop();
}

MLRESULT CMPlayer::doNext(bool bLoop)
{
    RMutexAutolock autolock(m_mutexDataAccess);

    if (m_currentPlaylist->getCount() == 0)
        return ERR_EMPTY_PLAYLIST;

    long            nOld = m_nCurrentMedia;

    if (m_bShuffle)
    {
        if (m_nCurrentShuffleMedia >= (int)m_vShuffleMedia.size() - 1)
        {
            if (m_loopMode != MP_LOOP_ALL && !bLoop)
                return ERR_END_OF_PLAYLIST;

            // all media has been played, regenerate radom playlist.
            generateShuffleMediaQueue();
        }

        assert(m_vShuffleMedia.size() == m_currentPlaylist->getCount());
        m_nCurrentMedia = m_vShuffleMedia[m_nCurrentShuffleMedia];
        m_nCurrentShuffleMedia++;
        // m_nCurrentMedia = rand() % m_currentPlaylist->getCount();
    }
    else
    {
        if (m_nCurrentMedia < (int)m_currentPlaylist->getCount() - 1)
            m_nCurrentMedia++;
        else if (m_loopMode != MP_LOOP_ALL && !bLoop)
            return ERR_END_OF_PLAYLIST;
        else
            m_nCurrentMedia = 0;
    }

    autolock.unlock();

    if (nOld != m_nCurrentMedia)
    {
        // Notify changed
        currentMediaChanged();
    }
    return ERR_OK;
}

static bool isBasicMediaInfoSame(IMedia *pMedia1, IMedia *pMedia2)
{
    static MediaAttribute        vStrAttrCheck[] = { MA_ARTIST, MA_ALBUM, MA_TITLE, MA_GENRE, MA_COMMENT };
    static MediaAttribute        vIntAttrCheck[] = { MA_TRACK_NUMB, MA_YEAR, MA_BITRATE, MA_DURATION, MA_FILESIZE };

    int i;
    CXStr        str1, str2;

    for (i = 0; i < CountOf(vStrAttrCheck); i++)
    {
        str1.clear();
        str2.clear();
        pMedia1->getAttribute(vStrAttrCheck[i], &str1);
        pMedia2->getAttribute(vStrAttrCheck[i], &str2);
        if (strcmp(str1.c_str(), str2.c_str()) != 0)
            return false;
    }

    int        n1, n2;
    for (i = 0; i < CountOf(vIntAttrCheck); i++)
    {
        n1 = n2 = 0;
        pMedia1->getAttribute(vIntAttrCheck[i], &n1);
        pMedia2->getAttribute(vIntAttrCheck[i], &n2);
        if (n1 != n2)
            return false;
    }

    return true;
}

void CMPlayer::currentMediaChanged()
{
    list<IMPEvent *>::iterator    it, itEnd;
    MLRESULT                nRet;
    bool                    bPlay = false;

    if (m_state != PS_STOPED)
    {
        if (m_state == PS_PLAYING)
            bPlay = true;
        m_bAutoPlayNext = false;
        nRet = doStop();
        if (nRet != ERR_OK && m_state != PS_STOPED)
            return;
    }

    {
        RMutexAutolock autolock(m_mutexDataAccess);

        if (m_pCurrentMedia)
        {
            if (m_bAutoPlayNext && m_bCurMediaPlayed)
                m_pMediaLib->markPlayFinished(m_pCurrentMedia);

            m_pCurrentMedia->release();
            m_pCurrentMedia = nullptr;
            m_bCurMediaPlayed = false;
        }

        if (m_currentPlaylist->getCount() > 0)
        {
            nRet = m_currentPlaylist->getItem(m_nCurrentMedia, &m_pCurrentMedia);
            if (nRet == ERR_OK)
            {
                nRet = loadMediaTagInfo(m_pCurrentMedia, false);
                if (nRet == ERR_OK)
                {
                    // update media info to media library, if changed.
                    CMPAutoPtr<IMedia>    mediaOld;
                    CXStr        url;
                    m_pCurrentMedia->getSourceUrl(&url);
                    if (m_pMediaLib->getMediaByUrl(url.c_str(), &mediaOld) == ERR_OK
                        && !isBasicMediaInfoSame(mediaOld, m_pCurrentMedia))
                    {
                        m_pMediaLib->updateMediaInfo(m_pCurrentMedia);
                    }
                }
            }
        }
    }

    m_mutexEventHandler.lock();
    itEnd = m_listEventHandler.end();
    for (it = m_listEventHandler.begin(); it != itEnd; ++it)
    {
        IMPEvent    *pEventHandler = *it;
        pEventHandler->onCurrentMediaChanged();
    }
    m_mutexEventHandler.unlock();

    if (bPlay)
        play();
}

MLRESULT CMPlayer::getInstance(IMPlayer **ppPlayer)
{
    if (!m_spPlayer)
    {
        m_spPlayer = new CMPlayer;
    }
    m_spPlayer->addRef();

    *ppPlayer = m_spPlayer;

    return ERR_OK;
}

MLRESULT CMPlayer::quitInstance(IMPlayer **ppPlayer)
{
    *ppPlayer = nullptr;

    if (m_spPlayer)
    {
        delete m_spPlayer;
        m_spPlayer = nullptr;
    }

    return ERR_OK;
}

void CMPlayer::notifyPlaylistChanged(CPlaylist *playlist, IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, long nIndex, long nIndexOld)
{
    if (m_currentPlaylist == playlist)
    {
        // first notify the message
        notifyCurrentPlaylistChanged(action, nIndex, nIndexOld);

        //
        // is current media changed?
        //
        CMPAutoPtr<IMedia>    pMedia;
        long                nNewCurrentIndex;
        RMutexAutolock autolock(m_mutexDataAccess);

        if (m_nCurrentMedia >= (int)m_currentPlaylist->getCount())
            m_nCurrentMedia = 0;

        generateShuffleMediaQueue();

        if (m_currentPlaylist->getCount() == 0)
        {
            autolock.unlock();
            if (m_pCurrentMedia)
                currentMediaChanged();
            return;
        }

        // is current media?
        if (m_currentPlaylist->getItem(m_nCurrentMedia, &pMedia) == ERR_OK)
        {
            if (m_pCurrentMedia == pMedia)
                return;
        }

        // get new index of current media.
        if (m_currentPlaylist->getItemIndex(m_pCurrentMedia, nNewCurrentIndex) == ERR_OK)
        {
            m_nCurrentMedia = nNewCurrentIndex;

            // unlock before notify
            autolock.unlock();

            list<IMPEvent *>::iterator    it, itEnd;

            RMutexAutolock lock(m_mutexEventHandler);
            itEnd = m_listEventHandler.end();
            for (it = m_listEventHandler.begin(); it != itEnd; ++it)
            {
                IMPEvent    *pEventHandler = *it;
                pEventHandler->onCurrentMediaChanged();
            }
            return;
        }

        // unlock before currentMediaChanged
        autolock.unlock();

        currentMediaChanged();
    }
}

void CMPlayer::notifyPlayStateChanged()
{
    list<IMPEvent *>::iterator    it, itEnd;

    RMutexAutolock lock(m_mutexEventHandler);

    itEnd = m_listEventHandler.end();
    for (it = m_listEventHandler.begin(); it != itEnd; ++it)
    {
        IMPEvent    *pEventHandler = *it;
        pEventHandler->onStateChanged(m_state);
    }
}

void CMPlayer::notifyCurrentPlaylistChanged(IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, long nIndex, long nIndexOld)
{
    list<IMPEvent *>::iterator    it, itEnd;

    // notify current playlist changed
    RMutexAutolock lock(m_mutexEventHandler);

    itEnd = m_listEventHandler.end();
    for (it = m_listEventHandler.begin(); it != itEnd; ++it)
    {
        IMPEvent    *pEventHandler = *it;
        pEventHandler->onCurrentPlaylistChanged(action, nIndex, nIndexOld);
    }
}

void CMPlayer::notifySettingsChanged(IMPEvent::MP_SETTING_TYPE settingType, long value)
{
    list<IMPEvent *>::iterator    it, itEnd;

    RMutexAutolock lock(m_mutexEventHandler);

    itEnd = m_listEventHandler.end();
    for (it = m_listEventHandler.begin(); it != itEnd; ++it)
    {
        IMPEvent    *pEventHandler = *it;
        pEventHandler->onSettingChanged(settingType, value);
    }
}

void CMPlayer::notifyEQSettingsChanged(const EQualizer *peq)
{
    list<IMPEvent *>::iterator    it, itEnd;

    RMutexAutolock lock(m_mutexEventHandler);

    itEnd = m_listEventHandler.end();
    for (it = m_listEventHandler.begin(); it != itEnd; ++it)
    {
        IMPEvent    *pEventHandler = *it;
        pEventHandler->onEQSettingChanged(peq);
    }
}

void CMPlayer::notifySeek()
{
    list<IMPEvent *>::iterator    it, itEnd;

    // notify current playlist changed
    RMutexAutolock lock(m_mutexEventHandler);
    itEnd = m_listEventHandler.end();
    for (it = m_listEventHandler.begin(); it != itEnd; ++it)
    {
        IMPEvent    *pEventHandler = *it;
        pEventHandler->onSeek();
    }
}

// For internal using, do NOT lock.
void CMPlayer::generateShuffleMediaQueue()
{
    long            nCount;

    nCount = m_currentPlaylist->getCount();
    m_nCurrentShuffleMedia = 0;

    vector<long>    vMediaSequence;
    long            i, nNext;

    for (i = 0; i < nCount; i++)
        vMediaSequence.push_back(i);

    m_vShuffleMedia.clear();
    for (i = 0; i < nCount; i++)
    {
        nNext = rand() % vMediaSequence.size();
        m_vShuffleMedia.push_back(vMediaSequence[nNext]);
        vMediaSequence.erase(vMediaSequence.begin() + nNext);
    }
}

MLRESULT CMPlayer::startVis()
{
    assert(m_threadVis == nullptr);

    m_bQuitVis = false;
    m_threadVis = new std::thread(&CMPlayer::threadVis, this);
    return ERR_OK;
}

MLRESULT CMPlayer::stopVis()
{
    if (m_threadVis == nullptr) {
        return ERR_OK;
    }

    m_bQuitVis = true;
    m_threadVis->join();
    delete m_threadVis;
    m_threadVis = nullptr;

    RMutexAutolock autolock(m_mutexVisdataAccess);

    while (!m_queVisDataPre.empty())
    {
        VisDataPre    &temp = m_queVisDataPre.front();
        temp.pBuf->release();
        m_queVisDataPre.pop_front();
    }

    m_nBufferedVisMs = 0;

    return ERR_OK;
}

void CMPlayer::calcVisParam(VisParam &param)
{
    VisDataPre temp;

    {
        RMutexAutolock lock(m_mutexVisdataAccess);
        // get pcm data
        if (m_queVisDataPre.size() == 0) {
            return;
        }
        temp = m_queVisDataPre.front();
        m_queVisDataPre.pop_front();
    }

    m_nBufferedVisMs -= temp.pBuf->size() / (temp.nBps / 8) / temp.nChannels / temp.nSampleRate;

    int nSamples = temp.pBuf->size() / (temp.nBps / 8) / temp.nChannels;
    if (nSamples >= VIS_N_WAVE_SAMPLE)
        nSamples = VIS_N_WAVE_SAMPLE;
    param.nChannels = temp.nChannels;

    //
    // clear not setted data 
    //
    if (temp.nChannels == 2)
    {
        for (int i = nSamples; i < VIS_N_WAVE_SAMPLE; i++)
        {
            param.waveformData[0][i] = 0;
            param.waveformData[1][i] = 0;
        }
    }
    else
    {
        for (int i = nSamples; i < VIS_N_WAVE_SAMPLE; i++)
        {
            param.waveformData[0][i] = 0;
        }
    }

    // analyse pcm data
    if (temp.nBps == 16)
    {
        // 16 bps wave data
        const unsigned short *buf = (const unsigned short *)temp.pBuf->data();

        int        k = 0;
        if (temp.nChannels == 2)
        {
            // stereo
            for (int i = 0; i < nSamples; i++)
            {
                param.waveformData[0][i] = (unsigned char)(buf[k++] / 0xFF);
                param.waveformData[1][i] = (unsigned char)(buf[k++] / 0xFF);
            }
            calc_freq(param.spectrumData[0], param.waveformData[0]);
            calc_freq(param.spectrumData[1], param.waveformData[1]);
        }
        else
        {
            // mono
            for (int i = 0; i < nSamples; i++)
            {
                param.waveformData[0][i] = (unsigned char)(buf[k++] / 0xFF);
            }
            calc_freq(param.spectrumData[0], param.waveformData[0]);
        }
    }
    else if (temp.nBps == 8)
    {
        const unsigned short *buf = (const unsigned short *)temp.pBuf->data();

        int        k = 0;
        if (temp.nChannels == 2)
        {
            // stereo
            for (int i = 0; i < nSamples; i++)
            {
                param.waveformData[0][i] = (unsigned char)(buf[k++]);
                param.waveformData[1][i] = (unsigned char)(buf[k++]);
            }
            calc_freq(param.spectrumData[0], param.waveformData[0]);
            calc_freq(param.spectrumData[1], param.waveformData[1]);
        }
        else
        {
            // mono
            for (int i = 0; i < nSamples; i++)
            {
                param.waveformData[0][i] = (unsigned char)(buf[k++]);
            }
            calc_freq(param.spectrumData[0], param.waveformData[0]);
        }
    }

    temp.pBuf->release();
}

void CMPlayer::threadVisProc(void *lpData)
{
    CMPlayer    *pPlayer = (CMPlayer *)lpData;

    pPlayer->threadVis();
}

void CMPlayer::threadVis()
{
    static VisParam        param;

    while (!m_bQuitVis)
    {
        Sleep(30);

        if (m_state == PS_PLAYING)
        {
            calcVisParam(param);
        }
        else if (m_state == PS_STOPED)
        {
            memset(param.spectrumData, 0, sizeof(param.spectrumData));
            memset(param.waveformData, 0, sizeof(param.waveformData));
        }

        // call vis to render
        V_VIS::iterator    it, itEnd;

        RMutexAutolock lock(m_mutexVisdataAccess);
        itEnd = m_vVis.end();
        for (it = m_vVis.begin(); it != m_vVis.end(); ++it)
        {
            IVis    *pVis = *it;
            pVis->render(&param);
        }
    }
}

bool CMPlayer::isVisActive()
{
    return !m_vVis.empty() && !m_bQuitVis;
}

void CMPlayer::addVisData(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate)
{
    VisDataPre        data;

    data.pBuf = pBuf;
    data.pBuf->addRef();
    data.nBps = nBps;
    data.nChannels = nChannels;
    data.nPlayingPos = 0;
    data.nSampleRate = nSampleRate;

    RMutexAutolock    autolock(m_mutexVisdataAccess);

    if ((m_nBufferedVisMs > m_nMODelay && m_queVisDataPre.size() >= 9) || m_queVisDataPre.size() > 40)
    {
        VisDataPre    &temp = m_queVisDataPre.front();
        m_nBufferedVisMs -= temp.pBuf->size() / (temp.nBps / 8) / temp.nChannels / temp.nSampleRate;
        temp.pBuf->release();
        m_queVisDataPre.pop_front();
    }

    m_nBufferedVisMs += pBuf->size() / (nBps / 8) / nChannels / nSampleRate;

    m_queVisDataPre.push_back(data);
}
