#include "IMPlayer.h"
#include "MDRow.h"


// raw configuration
#define NCH                 2
#define SAMPLERATE          44100
#define BPS                 16
//#define NCH 1
//#define SAMPLERATE 22050
//#define BPS 8



CMDRow::CMDRow() {
    OBJ_REFERENCE_INIT;

    m_pInput = nullptr;
    m_pPlayer = nullptr;
    m_pOutput = nullptr;
    m_bPaused = false;
    m_bKillThread = false;
    m_nSeekPos = 0;
    m_bSeekFlag = false;
    m_state = PS_STOPPED;
}

CMDRow::~CMDRow() {
    if (m_pPlayer) {
        m_pPlayer->release();
        m_pPlayer = nullptr;
    }
}

cstr_t CMDRow::getDescription() {
    return "Raw File decoder";
}

cstr_t CMDRow::getFileExtentions() {
    return ".raw|raw files";
}

ResultCode CMDRow::getMediaInfo(IMediaInput *pInput, IMediaInfo *pMedia) {
    uint32_t nFileSize;
    pInput->getSize(nFileSize);

    m_nLengthMs = (uint32_t)((int64_t)nFileSize * 1000 / (SAMPLERATE * NCH * (BPS / 8)));

    pMedia->setAttribute(MA_FILESIZE, nFileSize);
    pMedia->setAttribute(MA_DURATION, m_nLengthMs);
    pMedia->setAttribute(MA_SAMPLE_RATE, SAMPLERATE);
    pMedia->setAttribute(MA_CHANNELS, NCH);
    pMedia->setAttribute(MA_BITS_PER_SAMPLE, BPS);

    return ERR_OK;
}

bool CMDRow::isSeekable() {
    return true;
}

bool CMDRow::isUseOutputPlug() {
    return true;
}

ResultCode CMDRow::play(IMPlayer *pPlayer, IMediaInput *pInput) {
    assert(pPlayer && pInput);
    if (!pPlayer || !pInput) {
        return ERR_INVALID_HANDLE;
    }

    ResultCode nRet;
    uint32_t nFileSize;

    stop();

    m_state = PS_STOPPED;
    m_bPaused = false;
    m_nSeekPos = 0;
    m_bSeekFlag = false;

    m_pPlayer = pPlayer;
    m_pPlayer->addRef();

    m_pInput = pInput;
    m_pInput->addRef();

    nRet = m_pPlayer->queryInterface(MPIT_OUTPUT, (void **)&m_pOutput);
    if (nRet != ERR_OK) {
        goto R_FAILED;
    }

    nRet = m_pPlayer->queryInterface(MPIT_MEMALLOCATOR, (void **)&m_pMemAllocator);
    if (nRet != ERR_OK) {
        goto R_FAILED;
    }

    m_state = PS_PLAYING;
    m_bKillThread = false;

    pInput->getSize(nFileSize);

    m_nLengthMs = (uint32_t)((int64_t)nFileSize * 1000 / (SAMPLERATE * NCH * (BPS / 8)));

    nRet = m_pOutput->open(SAMPLERATE, NCH, BPS);
    if (nRet != ERR_OK) {
        goto R_FAILED;
    }

    m_threadDecode.create(decodeThread, this);
    m_threadDecode.setPriority(THREAD_PRIORITY_HIGHEST);

    return ERR_OK;

R_FAILED:
    if (m_pPlayer) {
        m_pPlayer->release();
        m_pPlayer = nullptr;
    }
    if (m_pInput) {
        m_pInput->release();
        m_pInput = nullptr;
    }
    if (m_pOutput) {
        m_pOutput->release();
        m_pOutput = nullptr;
    }
    if (m_pMemAllocator) {
        m_pMemAllocator->release();
        m_pMemAllocator = nullptr;
    }
    return nRet;
}

ResultCode CMDRow::pause() {
    m_state = PS_PAUSED;
    return m_pOutput->pause(true);
}

ResultCode CMDRow::unpause() {
    m_state = PS_PLAYING;
    return m_pOutput->pause(false);
}

ResultCode CMDRow::stop() {
    m_bKillThread = true;

    if (m_state == PS_PAUSED) {
        m_pOutput->flush();
    }

    m_threadDecode.join();

    return ERR_OK;
}

uint32_t CMDRow::getLength() {
    return m_nLengthMs;
}

ResultCode CMDRow::seek(uint32_t dwPos) {
    m_bSeekFlag = true;
    m_nSeekPos = dwPos;

    m_pOutput->flush();

    return ERR_OK;
}

uint32_t CMDRow::getPos() {
    if (m_pOutput) {
        return m_nSeekPos + m_pOutput->getPos();
    } else {
        return m_nSeekPos;
    }
}

ResultCode CMDRow::setVolume(int volume, int nBanlance) {
    return m_pOutput->setVolume(volume, nBanlance);
}

void CMDRow::decodeThread(void *lpParam) {
    CMDRow *pMDRow;

    pMDRow = (CMDRow *)lpParam;

    pMDRow->addRef();
    pMDRow->decodeThreadProc();
    pMDRow->release();
}

void CMDRow::decodeThreadProc() {
    IFBuffer *pBuf;
    uint32_t bufSize, nReaded;

    // bufSize = SAMPLERATE * NCH * (BPS / 8);
    bufSize = 512 *4;

    while (!m_bKillThread) {
        pBuf = m_pMemAllocator->allocFBuffer(bufSize + 4);
        nReaded = m_pInput->read(pBuf->data(), bufSize);
        if (nReaded == 0) {
            pBuf->release();
            pBuf = nullptr;
            break;
        }
        pBuf->resize(nReaded);
        m_pOutput->waitForWrite();
        if (m_bKillThread) {
            pBuf->release();
            break;
        }
        if (m_bSeekFlag) {
            pBuf->release();
        } else {
            m_pPlayer->outputWrite(pBuf, SAMPLERATE, NCH, BPS);
        }

        if (m_bSeekFlag) {
            uint32_t nSeekTo = (uint32_t)((double)m_nSeekPos * SAMPLERATE * NCH * (BPS / 8) / 1000);
            m_pInput->seek(nSeekTo, SEEK_SET);
            m_pOutput->flush();
            m_bSeekFlag = false;
        }
    }

    while (!m_bKillThread && m_pOutput->isPlaying()) {
        sleep(10);
    }

    m_state = PS_STOPPED;

    m_pOutput->stop();
    m_pOutput->release();
    m_pOutput = nullptr;

    m_pInput->release();
    m_pInput = nullptr;

    m_pMemAllocator->release();
    m_pMemAllocator = nullptr;

    m_pPlayer->notifyEod(this, ERR_OK);
    m_pPlayer->release();
    m_pPlayer = nullptr;
}
