#include "MORaw.h"


CMORaw::CMORaw() : m_eventCanWrite(false, true) {
    OBJ_REFERENCE_INIT;

    m_dwTotolBytesOffset = 0;
    m_fp = nullptr;
    m_bPaused = false;
    m_nChannels = 0;
    m_nSamplerate = 0;
    m_nBps = 0;
}

CMORaw::~CMORaw() {
}

cstr_t CMORaw::getDescription() {
    return "MPlayer Raw file output 1.0";
}

ResultCode CMORaw::open(int nSampleRate, int nNumChannels, int nBitsPerSamp) {
    m_bPaused = false;
    m_nChannels = nNumChannels;
    m_nSamplerate = nSampleRate;
    m_nBps = nBitsPerSamp;

    m_nBytesPerSample = nNumChannels * (nBitsPerSamp / 8);

    char szFile[MAX_PATH];
    getAppResourceDir(szFile);
    strcat_safe(szFile, CountOf(szFile), "mout.raw");
    m_fp = fopen(szFile, "wb");
    if (!m_fp) {
        return ERR_OPEN_FILE;
    }

    return ERR_OK;
}

ResultCode CMORaw::waitForWrite() {
    sleep(10);

    m_eventCanWrite.acquire();
    m_eventCanWrite.set();

    return ERR_OK;
}

ResultCode CMORaw::write(IFBuffer *pBuf) {
    int nRet = ERR_OK;

    m_dwTotolBytesOffset += pBuf->size();

    static int iii = 0;
    ERR_LOG2("write: %d, %d", iii, pBuf->size());
    iii++;

    if (m_fp && pBuf->size()) {
        if (fwrite(pBuf->data(), 1, pBuf->size(), m_fp) > 0) {
            nRet = ERR_OK;
        } else {
            nRet = ERR_WRITE_FILE;
        }
    }

    pBuf->release();

    return nRet;
}

ResultCode CMORaw::flush() {
    if (m_bPaused) {
        m_bPaused = false;
        m_eventCanWrite.set();
    }
    return ERR_OK;
}

ResultCode CMORaw::pause(bool bPause) {
    m_bPaused = bPause;
    if (bPause) {
        m_eventCanWrite.reset();
    } else {
        m_eventCanWrite.set();
    }

    return ERR_OK;
}

bool CMORaw::isPlaying() {
    return false;
}

ResultCode CMORaw::stop() {
    if (m_fp) {
        fclose(m_fp);
        m_fp = nullptr;
    }

    if (m_bPaused) {
        m_bPaused = false;
        m_eventCanWrite.set();
    }

    return ERR_OK;
}

bool CMORaw::isOpened() {
    return m_nBps != 0;
}

// volume
ResultCode CMORaw::setVolume(int volume, int nBanlance) {
    return ERR_OK;
}

uint32_t CMORaw::getPos() {
    uint32_t nTime = (uint32_t)((double)m_dwTotolBytesOffset * 1000 / (m_nBytesPerSample * m_nSamplerate));

    return nTime;
}
