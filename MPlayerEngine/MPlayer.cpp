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


//////////////////////////////////////////////////////////////////////////
// CFBuffer

CFBuffer::CFBuffer(uint32_t nCapacity) {
    OBJ_REFERENCE_INIT
    m_buf = nullptr;
    m_nCapacity = nCapacity;
    m_nSize = 0;

    m_buf = (char *)malloc(nCapacity);

    m_nCapacity = nCapacity;
    m_nSize = 0;
}

CFBuffer::~CFBuffer() {
    if (m_buf) {
        free(m_buf);
    }
}

void CFBuffer::addRef() {
    m_nReference++;
}

void CFBuffer::release() {
    if (--m_nReference <= 0) {
        delete this;
    }
}

char *CFBuffer::data() {
    return m_buf;
}

uint32_t CFBuffer::size() {
    return m_nSize;
}

uint32_t CFBuffer::capacity() {
    return m_nCapacity;
}

void CFBuffer::resize(uint32_t nSize) {
    assert(nSize <= m_nCapacity);
    if (nSize > m_nCapacity) {
        nSize = m_nCapacity;
    }
    m_nSize = nSize;
}

ResultCode CFBuffer::reserve(uint32_t nCapacity) {
    // assert(m_nRef == 1);
    if (nCapacity > m_nCapacity) {
        m_buf = (char *)realloc(m_buf, nCapacity);
        assert(m_buf);
        if (!m_buf) {
            return ERR_NO_MEM;
        }

        m_nCapacity = nCapacity;
    }

    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
CMOAgent::CMOAgent() {
    OBJ_REFERENCE_INIT;
}

CMOAgent::~CMOAgent() {
}

cstr_t CMOAgent::getDescription() {
    if (m_pMediaOutput) {
        return m_pMediaOutput->getDescription();
    } else {
        return "No output device is available.";
    }
}

ResultCode CMOAgent::init(IMPlayer *pPlayer) {
    return ERR_OK;
}

ResultCode CMOAgent::quit() {
    return ERR_OK;
}

ResultCode CMOAgent::open(int nSampleRate, int nNumChannels, int nBitsPerSamp) {
    if (m_pMediaOutput) {
        return m_pMediaOutput->open(nSampleRate, nNumChannels, nBitsPerSamp);
    } else {
        return ERR_NO_DEVICE;
    }
}

ResultCode CMOAgent::waitForWrite() {
    if (m_pMediaOutput) {
        return m_pMediaOutput->waitForWrite();
    } else {
        return ERR_NO_DEVICE;
    }
}

ResultCode CMOAgent::write(IFBuffer *pBuf) {
    if (m_pMediaOutput) {
        return m_pMediaOutput->write(pBuf);
    } else {
        return ERR_NO_DEVICE;
    }
}

ResultCode CMOAgent::flush() {
    if (m_pMediaOutput) {
        return m_pMediaOutput->flush();
    } else {
        return ERR_NO_DEVICE;
    }
}

ResultCode CMOAgent::pause(bool bPause) {
    if (m_pMediaOutput) {
        return m_pMediaOutput->pause(bPause);
    } else {
        return ERR_NO_DEVICE;
    }
}

bool CMOAgent::isPlaying() {
    if (m_pMediaOutput) {
        return m_pMediaOutput->isPlaying();
    } else {
        return false;
    }
}

ResultCode CMOAgent::stop() {
    if (m_pMediaOutput) {
        return m_pMediaOutput->stop();
    } else {
        return ERR_NO_DEVICE;
    }
}

bool CMOAgent::isOpened() {
    if (m_pMediaOutput) {
        return m_pMediaOutput->isOpened();
    } else {
        return false;
    }
}

uint32_t CMOAgent::getPos() {
    if (m_pMediaOutput) {
        return m_pMediaOutput->getPos();
    } else {
        return 0;
    }
}

// volume
ResultCode CMOAgent::setVolume(int volume, int nBanlance) {
    if (m_pMediaOutput) {
        return m_pMediaOutput->setVolume(volume, nBanlance);
    } else {
        return ERR_NO_DEVICE;
    }
}

//////////////////////////////////////////////////////////////////////

CMPlayer *CMPlayer::m_spPlayer = nullptr;

CMPlayer::CMPlayer() {
    OBJ_REFERENCE_INIT;

    m_state = PS_STOPPED;

    m_pluginMgr.detectPlugins();

    //
    // set DSP
    //
    {
        RMutexAutolock lock(m_mutexDSP);
        if (m_pDsp) {
            m_pDsp.release();
        }
        if (m_pluginMgr.getActiveDSP(&m_pDsp) == ERR_OK) {
            m_pDsp->init(this);
        }
    }

    //
    // set Vis
    //
    auto vVis = m_pluginMgr.getActiveVis();
    for (int i = 0; i < (int)vVis.size(); i++) {
        IVisualizer *pVis = vVis.at(i);

        pVis->init(this);
        registerVis(pVis);
    }

#ifdef _MAC_OS
    m_pMDAgent = new CoreAVPlayer();
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

    memset(&m_Equalizer, 0, sizeof(m_Equalizer));

    m_nBufferedVisMs = 0;
    m_nMODelay = 1000;

    m_bQuitVis = false;
    m_threadVis = nullptr;

    srand((uint32_t)getTickCount());
}

CMPlayer::~CMPlayer() {
    stopVis();

    {
        RMutexAutolock lock(m_mutexVisdataAccess);
        V_VIS::iterator it, itEnd;
        itEnd = m_vVis.end();
        for (it = m_vVis.begin(); it != m_vVis.end(); ++it) {
            IVisualizer *pVis = *it;
            pVis->quit();
            pVis->release();
        }
    }

    {
        RMutexAutolock lock(m_mutexDSP);
        if (m_pDsp) {
            m_pDsp->quit();
            m_pDsp.release();
        }
    }

    m_pMDAgent.release();

    m_pMOAgent.release();
}

//
// Player control
//

ResultCode CMPlayer::play() {
    if (m_state == PS_PLAYING) {
        return seek(0);
    } else if (m_state == PS_STOPPED) {
        if (m_currentMedia) {
            int nRet;
            m_isAutoPlayNext = true;
            m_isCurMediaPlayed = true;
            nRet = m_pMDAgent->doDecode(m_currentMedia);
            if (nRet == ERR_OK) {
                m_state = PS_PLAYING;
                notifyPlayStateChanged();

                if (m_pPlayer->m_volume != -1) {
                    if (m_pPlayer->m_isMute) {
                        m_pMediaDecode->setVolume(0, 0);
                    } else {
                        m_pMediaDecode->setVolume(m_pPlayer->m_volume, m_pPlayer->m_balance);
                    }
                }
            }

            return nRet;
        } else {
            notifyPlayStateChanged();
            return ERR_EMPTY_PLAYLIST;
        }
    } else {
        return unpause();
    }
}

ResultCode CMPlayer::pause() {
    ResultCode ret;
    if (m_state != PS_PLAYING) {
        return ERR_OK;
    }

    ret = m_pMDAgent->pause();
    if (ret == ERR_OK) {
        m_state = PS_PAUSED;
        notifyPlayStateChanged();
    }

    return ret;
}

ResultCode CMPlayer::unpause() {
    ResultCode ret;
    if (m_state != PS_PAUSED) {
        return ERR_OK;
    }

    ret = m_pMDAgent->unpause();
    if (ret == ERR_OK) {
        m_state = PS_PLAYING;
        notifyPlayStateChanged();
    }

    return ret;
}

ResultCode CMPlayer::stop() {
    if (m_state == PS_STOPPED) {
        return ERR_OK;
    }

    // use a temp MediaDecode with addRef for thread safe
    return m_pMDAgent->stop();
}

ResultCode CMPlayer::seek(uint32_t dwPos) {
    if (m_pMDAgent) {
        ResultCode nRet;
        if ((int)dwPos < -1) {
            dwPos = 0;
        }
        nRet = m_pMDAgent->seek(dwPos);
        return nRet;
    } else {
        return ERR_PLAYER_INVALID_STATE;
    }
}

uint32_t CMPlayer::getPos() {
    RMutexAutolock autolock(m_mutexDataAccess);

    if (m_pMDAgent) {
        return m_pMDAgent->getPos();
    } else {
        return 0;
    }
}

PlayerState CMPlayer::getState() {
    return m_state;
}

void CMPlayer::outputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) {
    {
        RMutexAutolock lock(m_mutexDSP);
        if (m_pDsp) {
            m_pDsp->process(pBuf, nBps, nChannels, nSampleRate);
        }
    }

    if (isVisActive()) {
        addVisData(pBuf, nBps, nChannels, nSampleRate);
    }

    m_pMOAgent->write(pBuf);
}

bool CMPlayer::getMute() {
    return m_isMute;
}

// 0 ~ 100
ResultCode CMPlayer::setVolume(int volume) {
    assert(volume >= 0 && volume <= 100);
    if (volume < 0) {
        volume = 0;
    } else if (volume > 100) {
        volume = 100;
    }
    m_volume = volume;
    if (m_pMDAgent) {
        return m_pMDAgent->setVolume(volume, m_balance);
    } else {
        return ERR_OK;
    }
}

int CMPlayer::getVolume() {
    return m_volume;
}

// -100 ~ 100
ResultCode CMPlayer::setBalance(int balance) {
    assert(balance >= -100 && balance <= 100);
    if (balance < -100) {
        balance = -100;
    } else if (balance > 100) {
        balance = 100;
    }
    m_balance = balance;
    notifySettingsChanged(IMPEvent::MPS_BALANCE, m_balance);
    if (m_pMDAgent && m_volume != -1) {
        return m_pMDAgent->setVolume(m_volume, balance);
    } else {
        return ERR_OK;
    }
}

int CMPlayer::getBalance() {
    return m_balance;
}

ResultCode CMPlayer::setEQ(const EQualizer *eq) {
    m_Equalizer = *eq;
    notifyEQSettingsChanged(&m_Equalizer);
    return ERR_OK;
}

ResultCode CMPlayer::getEQ(EQualizer *eq) {
    *eq = m_Equalizer;
    return ERR_OK;
}

void CMPlayer::notifyEod(IMediaDecoder *pDecoder, ResultCode nError) {
    bool bSendNotify = m_state != PS_STOPPED;

    m_state = PS_STOPPED;

    m_pMDAgent->notifyEod(pDecoder, nError);
}


ResultCode CMPlayer::makeOutputReadyForDecode() {
    ResultCode nRet;

    if (!m_pMOAgent->m_pMediaOutput) {
        nRet = m_pluginMgr.newOutput(&(m_pMOAgent->m_pMediaOutput));
        if (nRet != ERR_OK) {
            return nRet;
        }

        m_pMOAgent->m_pMediaOutput->init(this);
    }

    return ERR_OK;
}

// Vis:
ResultCode CMPlayer::registerVis(IVisualizer *pVis) {
    RMutexAutolock autolock(m_mutexVisdataAccess);
    V_VIS::iterator it, itEnd;

    itEnd = m_vVis.end();
    for (it = m_vVis.begin(); it != m_vVis.end(); ++it) {
        if (*it == pVis) {
            return ERR_EXIST;
        }
    }

    pVis->addRef();
    m_vVis.push_back(pVis);

    if (m_vVis.size() == 1) {
        autolock.unlock();
        startVis();
    }

    return ERR_OK;
}

ResultCode CMPlayer::unregisterVis(IVisualizer *pVis) {
    RMutexAutolock autolock(m_mutexVisdataAccess);
    V_VIS::iterator it, itEnd;

    itEnd = m_vVis.end();
    for (it = m_vVis.begin(); it != m_vVis.end(); ++it) {
        if (*it == pVis) {
            pVis->quit();
            pVis->release();
            m_vVis.erase(it);
            if (m_vVis.empty()) {
                autolock.unlock();
                stopVis();
            }
            return ERR_OK;
        }
    }

    return ERR_NOT_FOUND;
}

// Dsp:
ResultCode CMPlayer::registerDsp(IDSP *pVis) {
    return ERR_OK;
}

ResultCode CMPlayer::unregisterDsp(IDSP *pVis) {
    return ERR_OK;
}

ResultCode CMPlayer::getInstance(IMPlayer **ppPlayer) {
    if (!m_spPlayer) {
        m_spPlayer = new CMPlayer;
    }
    m_spPlayer->addRef();

    *ppPlayer = m_spPlayer;

    return ERR_OK;
}

ResultCode CMPlayer::quitInstance(IMPlayer **ppPlayer) {
    *ppPlayer = nullptr;

    if (m_spPlayer) {
        delete m_spPlayer;
        m_spPlayer = nullptr;
    }

    return ERR_OK;
}

ResultCode CMPlayer::startVis() {
    assert(m_threadVis == nullptr);

    m_bQuitVis = false;
    m_threadVis = new std::thread(&CMPlayer::threadVis, this);
    return ERR_OK;
}

ResultCode CMPlayer::stopVis() {
    if (m_threadVis == nullptr) {
        return ERR_OK;
    }

    m_bQuitVis = true;
    m_threadVis->join();
    delete m_threadVis;
    m_threadVis = nullptr;

    RMutexAutolock autolock(m_mutexVisdataAccess);

    while (!m_queVisDataPre.empty()) {
        VisDataPre &temp = m_queVisDataPre.front();
        temp.pBuf->release();
        m_queVisDataPre.pop_front();
    }

    m_nBufferedVisMs = 0;

    return ERR_OK;
}

void CMPlayer::calcVisParam(VisParam &param) {
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
    if (nSamples >= VIS_N_WAVE_SAMPLE) {
        nSamples = VIS_N_WAVE_SAMPLE;
    }
    param.nChannels = temp.nChannels;

    //
    // clear not setted data
    //
    if (temp.nChannels == 2) {
        for (int i = nSamples; i < VIS_N_WAVE_SAMPLE; i++) {
            param.waveformData[0][i] = 0;
            param.waveformData[1][i] = 0;
        }
    } else {
        for (int i = nSamples; i < VIS_N_WAVE_SAMPLE; i++) {
            param.waveformData[0][i] = 0;
        }
    }

    // analyse pcm data
    if (temp.nBps == 16) {
        // 16 bps wave data
        const unsigned short *buf = (const unsigned short *)temp.pBuf->data();

        int k = 0;
        if (temp.nChannels == 2) {
            // stereo
            for (int i = 0; i < nSamples; i++) {
                param.waveformData[0][i] = (unsigned char)(buf[k++] / 0xFF);
                param.waveformData[1][i] = (unsigned char)(buf[k++] / 0xFF);
            }
            calc_freq(param.spectrumData[0], param.waveformData[0]);
            calc_freq(param.spectrumData[1], param.waveformData[1]);
        } else {
            // mono
            for (int i = 0; i < nSamples; i++) {
                param.waveformData[0][i] = (unsigned char)(buf[k++] / 0xFF);
            }
            calc_freq(param.spectrumData[0], param.waveformData[0]);
        }
    } else if (temp.nBps == 8) {
        const unsigned short *buf = (const unsigned short *)temp.pBuf->data();

        int k = 0;
        if (temp.nChannels == 2) {
            // stereo
            for (int i = 0; i < nSamples; i++) {
                param.waveformData[0][i] = (unsigned char)(buf[k++]);
                param.waveformData[1][i] = (unsigned char)(buf[k++]);
            }
            calc_freq(param.spectrumData[0], param.waveformData[0]);
            calc_freq(param.spectrumData[1], param.waveformData[1]);
        } else {
            // mono
            for (int i = 0; i < nSamples; i++) {
                param.waveformData[0][i] = (unsigned char)(buf[k++]);
            }
            calc_freq(param.spectrumData[0], param.waveformData[0]);
        }
    }

    temp.pBuf->release();
}

void CMPlayer::threadVisProc(void *lpData) {
    CMPlayer *pPlayer = (CMPlayer *)lpData;

    pPlayer->threadVis();
}

void CMPlayer::threadVis() {
    static VisParam param;

    while (!m_bQuitVis) {
        Sleep(30);

        if (m_state == PS_PLAYING) {
            calcVisParam(param);
        } else if (m_state == PS_STOPPED) {
            memset(param.spectrumData, 0, sizeof(param.spectrumData));
            memset(param.waveformData, 0, sizeof(param.waveformData));
        }

        // call vis to render
        V_VIS::iterator it, itEnd;

        RMutexAutolock lock(m_mutexVisdataAccess);
        itEnd = m_vVis.end();
        for (it = m_vVis.begin(); it != m_vVis.end(); ++it) {
            IVisualizer *pVis = *it;
            pVis->render(&param);
        }
    }
}

bool CMPlayer::isVisActive() {
    return !m_vVis.empty() && !m_bQuitVis;
}

void CMPlayer::addVisData(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) {
    VisDataPre data;

    data.pBuf = pBuf;
    data.pBuf->addRef();
    data.nBps = nBps;
    data.nChannels = nChannels;
    data.nPlayingPos = 0;
    data.nSampleRate = nSampleRate;

    RMutexAutolock autolock(m_mutexVisdataAccess);

    if ((m_nBufferedVisMs > m_nMODelay && m_queVisDataPre.size() >= 9) || m_queVisDataPre.size() > 40) {
        VisDataPre &temp = m_queVisDataPre.front();
        m_nBufferedVisMs -= temp.pBuf->size() / (temp.nBps / 8) / temp.nChannels / temp.nSampleRate;
        temp.pBuf->release();
        m_queVisDataPre.pop_front();
    }

    m_nBufferedVisMs += pBuf->size() / (nBps / 8) / nChannels / nSampleRate;

    m_queVisDataPre.push_back(data);
}
