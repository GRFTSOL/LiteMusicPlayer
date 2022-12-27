#include "IMPlayer.h"
#include "MOSoundCard.h"


// Optimized for pocketpc.
bool g_bGaplessOutput = true;
#define        FADE_DURATION        500 / 1000    // ms
#define        DOUBLECONVT  int
// #define        DOUBLECONVT        double

void dBGMMError(MMRESULT nRet) {
    char szError[256];
    if (MMSYSERR_NOERROR == waveOutGetErrorText(nRet, szError, CountOf(szError))) {
        DBG_LOG0(szError);
    } else {
        DBG_LOG0("unknown");
    }
}

void CALLBACK
CMOSoundCard::MCICallBack(HWAVEOUT hwo, uint32_t msg, uint32_t dwInstance,
    uint32_t dwParam1, uint32_t dwParam2) {
    switch (msg) {
    case WOM_DONE:
        {
            LPWAVEHDR lpWaveHdr = (LPWAVEHDR) dwParam1;
            CMOSoundCard *pSoundCard= (CMOSoundCard *)dwInstance;
            IFBuffer *pBuf = (IFBuffer*)lpWaveHdr->dwUser;
            pBuf->release();
            pSoundCard->m_dwTotolBytesOffset += lpWaveHdr->dwBufferLength;

            if (pSoundCard->m_fadeMode == FADE_OUT && pSoundCard->m_pFadeOutEndHdr == lpWaveHdr) {
                waveOutPause(hwo);
                pSoundCard->m_pFadeOutEndHdr = nullptr;
            }

            pSoundCard->m_mutex.acquire();
            pSoundCard->m_listFree.push_back(lpWaveHdr);
            pSoundCard->m_nBuffered -= lpWaveHdr->dwBufferLength;
            if (pSoundCard->m_nBuffered < pSoundCard->m_nBufferedMax) {
                pSoundCard->m_eventCanWrite.set();
            }
            // DBG_LOG1("Mci callback: %d", pSoundCard->m_nBuffered);
            // if (pSoundCard->m_nBuffered <= 1024 * 10)
            {
                //    ERR_LOG1("Buffered: %d", pSoundCard->m_nBuffered);
            }
            assert(pSoundCard->m_listPrepared.front() == lpWaveHdr);
            pSoundCard->m_listPrepared.pop_front();
            pSoundCard->m_mutex.release();
            break;
        }
    }
}



CMOSoundCard::CMOSoundCard() : m_eventCanWrite(false, true) {
    OBJ_REFERENCE_INIT;

    m_dwTotolBytesOffset = 0;

    m_hwo = nullptr;

    m_fadeMode = FADE_NONE;
    m_pFadeOutEndHdr = nullptr;
    m_bWrittingPausedData = false;
}

CMOSoundCard::~CMOSoundCard() {
    if (m_hwo) {
        doStop();
    }
}

cstr_t CMOSoundCard::getDescription() {
    return "MPlayer soundcard output 1.0";
}

MLRESULT CMOSoundCard::open(int nSampleRate, int nNumChannels, int nBitsPerSamp) {
    DBG_LOG1("open: %d", m_nBuffered);

    m_fadeMode = FADE_NONE;

    if (m_hwo && g_bGaplessOutput) {
        if (m_nChannels != nNumChannels || nSampleRate != m_nSamplerate
            || m_nBitsPerSamp != nBitsPerSamp) {
            doStop();
        } else {
            m_eventCanWrite.set();
            return ERR_OK;
        }
    }
    assert(m_hwo == nullptr);

    m_nChannels = nNumChannels;
    m_nSamplerate = nSampleRate;
    m_nBitsPerSamp = nBitsPerSamp;

    return doOpen();
}

WAVEHDR* CMOSoundCard::NewWaveHdr() {
    WAVEHDR *waveHdr;
    waveHdr = new WAVEHDR;
    memset(waveHdr, 0, sizeof(WAVEHDR));

    return waveHdr;
}

MLRESULT CMOSoundCard::waitForWrite() {
    if (!m_eventCanWrite.acquire()) {
        return ERR_FALSE;
    }

    m_eventCanWrite.set();

    return ERR_OK;
}

MLRESULT CMOSoundCard::write(IFBuffer *pBuf) {
    assert(m_hwo);

    WAVEHDR *pWaveHdr;
    MMRESULT result;

    m_eventCanWrite.acquire();

    while (m_bWrittingPausedData) {
        sleep(20);
    }

    m_mutex.acquire();

    // get a free WaveHdr
    if (m_listFree.size()) {
        pWaveHdr = m_listFree.front();
        m_listFree.pop_front();
    } else {
        pWaveHdr = NewWaveHdr();
    }

    if (m_fadeMode == FADE_OUT && m_nFadeDoneSamples == 0) {
        m_listPausedHdr.push_back(pWaveHdr);
    } else {
        m_listPrepared.push_back(pWaveHdr);
    }
    m_nBuffered += pBuf->size();
    if (m_nBuffered < m_nBufferedMax) {
        m_eventCanWrite.set();
    }

    m_mutex.release();

    if (pWaveHdr->dwFlags & WHDR_PREPARED) {
        waveOutUnprepareHeader(m_hwo, pWaveHdr, sizeof(WAVEHDR));
        pWaveHdr->dwFlags = 0;
    }
    pWaveHdr->lpData = pBuf->data();
    pWaveHdr->dwBufferLength = pBuf->size();
    pWaveHdr->dwUser = (uint32_t)pBuf;

    if (m_fadeMode == FADE_IN) {
        // fade in sound data.
        fadeInSoundData((char *)pWaveHdr->lpData, pWaveHdr->dwBufferLength);
        if (m_nFadeDoneSamples == m_nFadeMaxSamples) {
            m_fadeMode = FADE_NONE;
        }
    } else if (m_fadeMode == FADE_OUT) {
        if (m_nFadeDoneSamples != 0) {
            fadeOutSoundData((char *)pWaveHdr->lpData, pWaveHdr->dwBufferLength);
            if (m_nFadeDoneSamples == 0) {
                m_pFadeOutEndHdr = pWaveHdr;
            }
        } else {
            // alread added data to m_listPausedHdr
            return ERR_OK;
        }
    }

    result = waveOutPrepareHeader(m_hwo, pWaveHdr, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        dBGMMError(result);
        return ERR_SOUND_DEVICE_WRITE;
    }

    result = waveOutWrite(m_hwo, pWaveHdr, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        dBGMMError(result);
        return ERR_SOUND_DEVICE_WRITE;
    }

    return ERR_OK;
}

MLRESULT CMOSoundCard::flush() {
    LIST_WAVHDR::iterator it, itEnd;

    m_mutex.acquire();
    itEnd = m_listPausedHdr.end();
    for(it = m_listPausedHdr.begin(); it != itEnd; ++it) {
        WAVEHDR *pWaveHdr = *it;
        IFBuffer *pBuf = (IFBuffer*)pWaveHdr->dwUser;
        pBuf->release();
        m_nBuffered -= pWaveHdr->dwBufferLength;
        delete pWaveHdr;
    }
    m_listPausedHdr.clear();
    m_mutex.release();

    waveOutReset(m_hwo);

    m_eventCanWrite.set();

    return ERR_OK;
}

MLRESULT CMOSoundCard::pause(bool bPause) {
    assert(m_hwo);

    if (bPause) {
        if (m_fadeMode != FADE_OUT) {
            // fade out sound data.
            m_fadeMode = FADE_OUT;
            m_nFadeMaxSamples = m_nSamplerate * FADE_DURATION; // fade time 0.5 sec
            m_nFadeDoneSamples = m_nFadeMaxSamples;
            m_pFadeOutEndHdr = nullptr;

            m_mutex.acquire();
            LIST_WAVHDR::iterator it, itEnd;
            itEnd = m_listPrepared.end();
            for(it = m_listPrepared.begin(); it != itEnd; ++it) {
                WAVEHDR *pWaveHdr = *it;
                assert(pWaveHdr->dwFlags & WHDR_PREPARED);

                fadeOutSoundData((char *)pWaveHdr->lpData, pWaveHdr->dwBufferLength);
                if (m_nFadeDoneSamples == m_nFadeMaxSamples) {
                    m_pFadeOutEndHdr = pWaveHdr;
                    break;
                }
            }
            m_mutex.release();
        }
    } else {
        if (m_fadeMode != FADE_IN) {
            LIST_WAVHDR::iterator it, itEnd;
            LIST_WAVHDR listTemp;

            // fade in sound data.
            m_fadeMode = FADE_IN;
            m_nFadeDoneSamples = 0;
            m_nFadeMaxSamples = m_nSamplerate * FADE_DURATION; // fade time 0.5 sec

            m_bWrittingPausedData = true;

            m_mutex.acquire();
            itEnd = m_listPausedHdr.end();
            for (it = m_listPausedHdr.begin(); it != itEnd; ++it) {
                WAVEHDR *pWaveHdr = *it;
                m_listPrepared.push_back(pWaveHdr);
            }
            listTemp = m_listPausedHdr;
            m_listPausedHdr.clear();
            m_mutex.release();

            itEnd = listTemp.end();
            for (it = listTemp.begin(); it != itEnd; ++it) {
                WAVEHDR *pWaveHdr = *it;
                MMRESULT result;

                if (m_nFadeDoneSamples != m_nFadeMaxSamples) {
                    fadeInSoundData(pWaveHdr->lpData, pWaveHdr->dwBufferLength);
                    if (m_nFadeDoneSamples == m_nFadeMaxSamples) {
                        m_fadeMode = FADE_NONE;
                    }
                }

                result = waveOutPrepareHeader(m_hwo, pWaveHdr, sizeof(WAVEHDR));
                if (result != MMSYSERR_NOERROR) {
                    dBGMMError(result);
                }

                result = waveOutWrite(m_hwo, pWaveHdr, sizeof(WAVEHDR));
                if (result != MMSYSERR_NOERROR) {
                    dBGMMError(result);
                }
            }
            m_bWrittingPausedData = false;

            m_eventCanWrite.set();

            waveOutRestart(m_hwo);
        }
    }

    return ERR_OK;
}

bool CMOSoundCard::isPlaying() {
    if (m_listPrepared.empty()) {
        return false;
    } else {
        return true;
    }
}

MLRESULT CMOSoundCard::stop() {
    LIST_WAVHDR::iterator it, itEnd;

    m_fadeMode = FADE_NONE;

    m_mutex.acquire();
    itEnd = m_listPausedHdr.end();
    for(it = m_listPausedHdr.begin(); it != itEnd; ++it) {
        WAVEHDR *pWaveHdr = *it;
        IFBuffer *pBuf = (IFBuffer*)pWaveHdr->dwUser;
        pBuf->release();
        m_nBuffered -= pWaveHdr->dwBufferLength;
        delete pWaveHdr;
    }
    m_listPausedHdr.clear();
    m_mutex.release();

    if (g_bGaplessOutput) {
        if (m_hwo) {
            waveOutReset(m_hwo);
        }

        m_eventCanWrite.set();
        return ERR_OK;
    } else {
        return doStop();
    }
}

bool CMOSoundCard::isOpened() {
    return m_hwo != nullptr;
}

// volume
MLRESULT CMOSoundCard::setVolume(int volume, int nBanlance) {
    assert(nBanlance >= -100 && nBanlance <= 100);
    MMRESULT ret;
    int nLeft, nRight;

    nLeft = (int)((double)volume * (100 - nBanlance) * 0xFFFF / 10000);
    nRight = (int)((double)volume * (100 + nBanlance) * 0xFFFF / 10000);

    if (m_hwo) {
        ret = waveOutSetVolume(m_hwo, MAKELONG(nLeft, nRight));
    } else {
        return ERR_FALSE;
    }
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

#define MAXINT32            0x7FFFFFFF

uint32_t CMOSoundCard::getPos() {
    if (!m_hwo) {
        return 0;
    }

    MMTIME sTime;
    uint32_t nTime;

    sTime.wType = TIME_BYTES;
    if (waveOutGetPosition(m_hwo, &sTime, sizeof(sTime)) != MMSYSERR_NOERROR) {
        return 0;
    }

    nTime = (uint32_t)((double)sTime.u.cb * 1000 / (m_iBytesPerSample * m_nSamplerate));

    return nTime;
}


MLRESULT CMOSoundCard::doOpen() {
    MMRESULT mmresult = 0;

    m_iBytesPerSample = m_nChannels * (m_nBitsPerSamp / 8);

    // cache 1 second of sound data
    m_nBufferedMax = m_iBytesPerSample * m_nSamplerate;
    m_nBuffered = 0;

    m_bWrittingPausedData = false;

    m_wfex.wBitsPerSample = m_nBitsPerSamp;
    m_wfex.wFormatTag = WAVE_FORMAT_PCM;
    m_wfex.nChannels = (uint16_t) m_nChannels;
    m_wfex.nSamplesPerSec = m_nSamplerate;
    m_wfex.nBlockAlign = (m_nBitsPerSamp / 8) * m_nChannels;
    m_wfex.nAvgBytesPerSec = m_nSamplerate * m_wfex.nBlockAlign;
    m_wfex.cbSize = 0;

    mmresult = waveOutOpen(&m_hwo,
        WAVE_MAPPER,
        &m_wfex,
        (uint32_t) MCICallBack,
        (uint32_t)this,
        WAVE_ALLOWSYNC | CALLBACK_FUNCTION);

    if (mmresult != MMSYSERR_NOERROR) {
        dBGMMError(mmresult);
        return ERR_FALSE;
    }

    /*    assert(!m_wavehdr_array);
    m_nBegFreeHeader = 0;
    m_nEndFreeHeader = m_nNumHeaders - 1;*/

    return ERR_OK;
}


MLRESULT CMOSoundCard::doStop() {
    if (m_hwo) {
        waveOutReset(m_hwo);
    }

    LIST_WAVHDR::iterator it, itEnd;
    LIST_WAVHDR listTemp;

    m_mutex.acquire();
    listTemp = m_listFree;
    m_listFree.clear();

    itEnd = m_listPausedHdr.end();
    for(it = m_listPausedHdr.begin(); it != itEnd; ++it) {
        WAVEHDR *pWaveHdr = *it;
        IFBuffer *pBuf = (IFBuffer*)pWaveHdr->dwUser;
        m_nBuffered -= pWaveHdr->dwBufferLength;
        pBuf->release();
        delete pWaveHdr;
    }
    m_listPausedHdr.clear();
    m_mutex.release();

    itEnd = listTemp.end();
    for(it = listTemp.begin(); it != itEnd; ++it) {
        WAVEHDR *pWaveHdr = *it;
        if (pWaveHdr->dwFlags & WHDR_PREPARED) {
            waveOutUnprepareHeader(m_hwo, pWaveHdr, sizeof(WAVEHDR));
            pWaveHdr->dwFlags = 0;
        }
        delete pWaveHdr;
    }

    if (m_hwo) {
        waveOutClose(m_hwo);
        m_hwo = nullptr;
    }

    return ERR_OK;
}

void CMOSoundCard::fadeInSoundData(char *data, int nLen) {
    //
    // Fade in sound data.
    //
    assert(m_nFadeDoneSamples < m_nFadeMaxSamples);

    int n = nLen / (m_nBitsPerSamp / 8);
    int nSamples = n / m_nChannels;
    int i, nTo;

    if (m_nBitsPerSamp == 8) {
        char *buf = data;

        if (m_nFadeDoneSamples + nSamples > m_nFadeMaxSamples) {
            nTo = (m_nFadeMaxSamples - m_nFadeDoneSamples) * m_nChannels;
        } else {
            nTo = n;
        }
        if (m_nChannels == 1) {
            for (i = 0; i < nTo; i++) {
                buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                m_nFadeDoneSamples ++;
            }
        } else if (m_nChannels == 2) {
            for (i = 0; i < nTo; i++) {
                buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                i++;
                buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                m_nFadeDoneSamples ++;
            }
        }
    } else if (m_nBitsPerSamp == 16) {
        short *buf = (short *)data;

        if (m_nFadeDoneSamples + nSamples > m_nFadeMaxSamples) {
            nTo = (m_nFadeMaxSamples - m_nFadeDoneSamples) * m_nChannels;
        } else {
            nTo = n;
        }
        if (m_nChannels == 1) {
            for (i = 0; i < nTo; i++) {
                buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                m_nFadeDoneSamples ++;
            }
        } else if (m_nChannels == 2) {
            for (i = 0; i < nTo; i++) {
                buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                i++;
                buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                m_nFadeDoneSamples++;
            }
        }
    } else {
        m_nFadeDoneSamples = m_nFadeMaxSamples;
    }
}

void CMOSoundCard::fadeOutSoundData(char *data, int nLen) {
    //
    // Fade out sound data.
    //

    int n = nLen / (m_nBitsPerSamp / 8);
    int nSamples = n / m_nChannels;
    int i, nTo;

    if (m_nBitsPerSamp == 8) {
        char *buf = data;

        if (m_nFadeDoneSamples < nSamples) {
            nTo = m_nFadeDoneSamples * m_nChannels;
        } else {
            nTo = n;
        }
        if (m_nChannels == 1) {
            for (i = 0; i < nTo; i++) {
                buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                m_nFadeDoneSamples --;
            }
            for (; i < n; i++) {
                buf[i] = 0;
            }
        } else if (m_nChannels == 2) {
            for (i = 0; i < nTo; i++) {
                buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                i++;
                buf[i] = (char)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                m_nFadeDoneSamples --;
            }
            for (; i < n; i++) {
                buf[i] = 0;
            }
        }
    } else if (m_nBitsPerSamp == 16) {
        short *buf = (short *)data;

        if (m_nFadeDoneSamples < nSamples) {
            nTo = m_nFadeDoneSamples * m_nChannels;
        } else {
            nTo = n;
        }
        if (m_nChannels == 1) {
            for (i = 0; i < nTo; i++) {
                buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                m_nFadeDoneSamples --;
            }
            for (; i < n; i++) {
                buf[i] = 0;
            }
        } else if (m_nChannels == 2) {
            for (i = 0; i < nTo; i++) {
                buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                i++;
                buf[i] = (short)((DOUBLECONVT)buf[i] * m_nFadeDoneSamples / m_nFadeMaxSamples);
                m_nFadeDoneSamples --;
            }
            for (; i < n; i++) {
                buf[i] = 0;
            }
        }
    } else {
        m_nFadeDoneSamples = 0;
    }
}
