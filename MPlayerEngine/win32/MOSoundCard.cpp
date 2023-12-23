#include <assert.h>
#include "MOSoundCard.h"


// Optimized for pocketpc.
#define        FADE_DURATION        500 / 1000    // ms
#define        DOUBLECONVT  int
// #define        DOUBLECONVT        double

void LogMMError(MMRESULT nRet) {
    utf16_t error[256];
    if (MMSYSERR_NOERROR == waveOutGetErrorTextW(nRet, error, CountOf(error))) {
        DBG_LOG0(ucs2ToUtf8(error, -1).c_str());
    } else {
        DBG_LOG0("unknown");
    }
}

WAVEHDR *newWaveHdr(IFBuffer *fb) {
    WAVEHDR *waveHdr = new WAVEHDR;
    memset(waveHdr, 0, sizeof(WAVEHDR));

    auto size = fb->size();
    auto p = new uint8_t[size];
    memcpy(p, fb->data(), size);
    waveHdr->lpData = (LPSTR)p;
    waveHdr->dwBufferLength = size;

    return waveHdr;
}

MOSoundCard::MOSoundCard() {
}

MOSoundCard::~MOSoundCard() {
    if (m_hwo) {
        doStop();
    }
}

ResultCode MOSoundCard::waitForWrite(int timeOutMs) {
    if (m_nBuffered < m_nBufferedMax || m_nBufferedMax == 0) {
        return ERR_OK;
    } else {
        std::unique_lock<std::mutex> lock(_mutexWaitWrite);
        _cvWaitWrite.wait_for(lock, std::chrono::microseconds(timeOutMs));
        if (m_nBuffered < m_nBufferedMax) {
            return ERR_OK;
        } else {
            return ERR_BUFFER_FULL;
        }
    }
}

ResultCode MOSoundCard::write(IFBuffer *fb) {
    auto ret = doOpen(fb->sampleRate(), fb->channels(), fb->bps());
    if (ret != ERR_OK) {
        return ret;
    }

    assert(m_hwo);

    m_nBuffered += fb->size();

    // get a free WaveHdr
    auto waveHdr = newWaveHdr(fb);

    MMRESULT result = waveOutPrepareHeader(m_hwo, waveHdr, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        LogMMError(result);
        return ERR_SOUND_DEVICE_WRITE;
    }

    result = waveOutWrite(m_hwo, waveHdr, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        LogMMError(result);
        return ERR_SOUND_DEVICE_WRITE;
    }

    return ERR_OK;
}

ResultCode MOSoundCard::flush() {
    waveOutReset(m_hwo);
    return ERR_OK;
}

ResultCode MOSoundCard::play() {
    waveOutRestart(m_hwo);
    return ERR_OK;
}

ResultCode MOSoundCard::pause() {
    waveOutPause(m_hwo);
    return ERR_OK;
}

bool MOSoundCard::isPlaying() {
    return m_nBuffered > 0;
}

ResultCode MOSoundCard::stop() {
    waveOutReset(m_hwo);
    return ERR_OK;
    // return doStop();
}

uint32_t MOSoundCard::getPos() {
    if (!m_hwo) {
        return 0;
    }

    MMTIME mtime;
    mtime.wType = TIME_BYTES;
    if (waveOutGetPosition(m_hwo, &mtime, sizeof(mtime)) != MMSYSERR_NOERROR) {
        return 0;
    }

    return (uint32_t)((double)mtime.u.cb * 1000 / (m_iBytesPerSample * m_sampleRate));
}

ResultCode MOSoundCard::setVolume(int volume, int banlance) {
    assert(banlance >= -100 && banlance <= 100);
    m_volume = volume;


    int left = (int)((double)volume * (100 - banlance) * 0xFFFF / 10000);
    int right = (int)((double)volume * (100 + banlance) * 0xFFFF / 10000);

    if (m_hwo == nullptr) {
        return ERR_FALSE;
    }

    MMRESULT ret = waveOutSetVolume(m_hwo, MAKELONG(left, right));
    if (ret == MMSYSERR_NOERROR) {
        return ERR_OK;
    } else if (ret == MMSYSERR_NOTSUPPORTED) {
        return ERR_NOT_SUPPORT;
    } else if (ret == MMSYSERR_NOMEM) {
        return ERR_NO_MEM;
    } else {
        return ERR_FALSE;
    }
}

int MOSoundCard::getVolume() {
    return m_volume;
}


ResultCode MOSoundCard::doOpen(int sampleRate, int numChannels, int bitsPerSamp) {
    if (m_hwo) {
        if (m_channels != numChannels || sampleRate != m_sampleRate || m_bitsPerSamp != bitsPerSamp) {
            doStop();
        } else {
            return ERR_OK;
        }
    }

    m_channels = numChannels;
    m_sampleRate = sampleRate;
    m_bitsPerSamp = bitsPerSamp;

    MMRESULT mmresult = 0;

    m_iBytesPerSample = m_channels * (m_bitsPerSamp / 8);

    // cache 1 second of sound data
    m_nBufferedMax = m_iBytesPerSample * m_sampleRate;
    m_nBuffered = 0;

    m_wfex.wBitsPerSample = m_bitsPerSamp;
    m_wfex.wFormatTag = WAVE_FORMAT_PCM;
    m_wfex.nChannels = (uint16_t) m_channels;
    m_wfex.nSamplesPerSec = m_sampleRate;
    m_wfex.nBlockAlign = (m_bitsPerSamp / 8) * m_channels;
    m_wfex.nAvgBytesPerSec = m_sampleRate * m_wfex.nBlockAlign;
    m_wfex.cbSize = 0;

    mmresult = waveOutOpen(&m_hwo,
        WAVE_MAPPER,
        &m_wfex,
        (DWORD_PTR)mciCallBack,
        (DWORD_PTR)this,
        WAVE_ALLOWSYNC | CALLBACK_FUNCTION);

    if (mmresult != MMSYSERR_NOERROR) {
        LogMMError(mmresult);
        return ERR_FALSE;
    }

    /*    assert(!m_wavehdr_array);
    m_nBegFreeHeader = 0;
    m_nEndFreeHeader = m_nNumHeaders - 1;*/

    return ERR_OK;
}

ResultCode MOSoundCard::doStop() {
    if (m_hwo) {
        waveOutReset(m_hwo);
    }

    if (m_hwo) {
        waveOutClose(m_hwo);
        m_hwo = nullptr;
    }

    return ERR_OK;
}

void CALLBACK MOSoundCard::mciCallBack(HWAVEOUT hwo, uint32_t msg, DWORD_PTR user, DWORD_PTR param1, DWORD_PTR param2) {
    if (msg == WOM_DONE) {
        MOSoundCard *thiz= (MOSoundCard *)user;
        LPWAVEHDR waveHdr = (LPWAVEHDR)param1;

        thiz->m_nBuffered -= waveHdr->dwBufferLength;
        if (thiz->m_nBuffered < thiz->m_nBufferedMax) {
            thiz->_cvWaitWrite.notify_one();
        }

        delete[] (uint8_t *)waveHdr->lpData;
        delete waveHdr;
    }
}