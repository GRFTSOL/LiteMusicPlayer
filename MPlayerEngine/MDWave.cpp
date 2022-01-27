// MDWave.cpp: implementation of the CMDWave class.
//
//////////////////////////////////////////////////////////////////////

#include "MDWave.h"

#define    WAVE_FORMAT_UNKNOWN        (0x0000)
#define    WAVE_FORMAT_PCM            (0x0001)
#define    WAVE_FORMAT_ADPCM        (0x0002)
#define    WAVE_FORMAT_ALAW        (0x0006)
#define    WAVE_FORMAT_MULAW        (0x0007)
#define    WAVE_FORMAT_OKI_ADPCM        (0x0010)
#define    WAVE_FORMAT_DIGISTD        (0x0015)
#define    WAVE_FORMAT_DIGIFIX        (0x0016)
#define    IBM_FORMAT_MULAW             (0x0101)
#define    IBM_FORMAT_ALAW            (0x0102)
#define    IBM_FORMAT_ADPCM             (0x0103)


CMDWave::CMDWave()
{
    OBJ_REFERENCE_INIT;

    m_pInput = nullptr;
    m_pPlayer = nullptr;
    m_pOutput = nullptr;
    m_bPaused = false;
    m_bKillThread = false;
    m_nSeekPos = 0;
    m_bSeekFlag = false;
    m_state = PS_STOPED;
    m_bufInput.reserve(1024 * 16);
}

CMDWave::~CMDWave()
{
    if (m_pPlayer)
    {
        m_pPlayer->release();
        m_pPlayer = nullptr;
    }
}

LPCXSTR CMDWave::getDescription()
{
    return "Wave File decoder";
}

LPCXSTR CMDWave::getFileExtentions()
{
    return ".wav|wave files";
}

MLRESULT CMDWave::getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
    MLRESULT        nRet;

    nRet = getHeadInfo(pInput);
    if (nRet != ERR_OK)
        return nRet;

    pMedia->setAttribute(MA_FILESIZE, m_audioInfo.nMediaFileSize);
     pMedia->setAttribute(MA_DURATION, m_audioInfo.nMediaLength);
     pMedia->setAttribute(MA_SAMPLE_RATE, m_audioInfo.samples_per_sec);
     pMedia->setAttribute(MA_CHANNELS, m_audioInfo.channels);
     pMedia->setAttribute(MA_BPS, m_audioInfo.bits_per_sample);
    pMedia->setAttribute(MA_BITRATE, m_audioInfo.avg_bytes_per_sec);

    return nRet;
}

bool CMDWave::isSeekable()
{
    return true;
}

bool CMDWave::isUseOutputPlug()
{
    return true;
}

MLRESULT CMDWave::play(IMPlayer *pPlayer, IMediaInput *pInput)
{
    assert(pPlayer && pInput);
    if (!pPlayer || !pInput)
        return ERR_INVALID_HANDLE;

    stop();

    MLRESULT        nRet;

    m_state = PS_STOPED;
    m_bPaused = false;
    m_nSeekPos = 0;
    m_bSeekFlag = false;

    m_pPlayer = pPlayer;
    m_pPlayer->addRef();

    m_pInput = pInput;
    m_pInput->addRef();

    nRet = m_pPlayer->queryInterface(MPIT_OUTPUT, (void **)&m_pOutput);
    if (nRet != ERR_OK)
        goto R_FAILED;

    nRet = m_pPlayer->queryInterface(MPIT_MEMALLOCATOR, (void **)&m_pMemAllocator);
    if (nRet != ERR_OK)
        goto R_FAILED;

    nRet = getHeadInfo(m_pInput);
    if (nRet != ERR_OK)
        goto R_FAILED;

    nRet = m_pOutput->open(m_audioInfo.samples_per_sec, m_audioInfo.channels, m_audioInfo.bits_per_sample);
    if (nRet != ERR_OK)
        goto R_FAILED;

    m_state = PS_PLAYING;
    m_bKillThread = false;

    m_threadDecode.create(decodeThread, this);
    m_threadDecode.setPriority(THREAD_PRIORITY_HIGHEST);

    return ERR_OK;

R_FAILED:
    if (m_pPlayer)
    {
        m_pPlayer->release();
        m_pPlayer = nullptr;
    }
    if (m_pInput)
    {
        m_pInput->release();
        m_pInput = nullptr;
    }
    if (m_pOutput)
    {
        m_pOutput->release();
        m_pOutput = nullptr;
    }
    if (m_pMemAllocator)
    {
        m_pMemAllocator->release();
        m_pMemAllocator = nullptr;
    }
    return nRet;
}

MLRESULT CMDWave::pause()
{
    m_state = PS_PAUSED;
    return m_pOutput->pause(true);
}

MLRESULT CMDWave::unpause()
{
    m_state = PS_PLAYING;
    return m_pOutput->pause(false);
}

MLRESULT CMDWave::stop()
{
    m_bKillThread = true;

    if (m_state == PS_PAUSED)
    {
        m_pOutput->flush();
    }

    m_threadDecode.join();

    return ERR_OK;
}

uint32_t CMDWave::getLength()
{
    return m_audioInfo.nMediaLength;
}

MLRESULT CMDWave::seek(uint32_t dwPos)
{
    m_bSeekFlag = true;
    m_nSeekPos = dwPos;

    m_pOutput->flush();

    return ERR_OK;
}

uint32_t CMDWave::getPos()
{
    if (m_pOutput)
        return m_nSeekPos + m_pOutput->getPos();
    else
        return m_nSeekPos;
}

MLRESULT CMDWave::setVolume(int nVolume, int nBanlance)
{
    return m_pOutput->setVolume(nVolume, nBanlance);
}

void CMDWave::decodeThread(void *lpParam)
{
    CMDWave        *pMDRow;

    pMDRow = (CMDWave *)lpParam;

    pMDRow->addRef();
    pMDRow->decodeThreadProc();
    pMDRow->release();
}


int read_le_long(IMediaInput *pInput, long *ret)
{
    unsigned char buf[4];

    if (pInput->read(buf, 4) != 4)
        return 0;

    *ret = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
    return 4;
}

int read_le_short(IMediaInput *pInput, short *ret)
{
    unsigned char buf[2];

    if (pInput->read(buf, 2) != 2)
        return 0;

    *ret = (buf[1] << 8) | buf[0];
    return true;
}

void CMDWave::decodeThreadProc()
{
    int bytes, blk_size, size_per_second;
    int actual_read;

    size_per_second = m_audioInfo.samples_per_sec * m_audioInfo.channels *
        (m_audioInfo.bits_per_sample / 8);
    blk_size = size_per_second / 8;

    IFBuffer    *pBuf;

    while (!m_bKillThread)
    {
        bytes = blk_size;
        if (int(m_audioInfo.length - m_audioInfo.position) < bytes)
            bytes = m_audioInfo.length - m_audioInfo.position;
        if (bytes > 0)
        {
            pBuf = m_pMemAllocator->allocFBuffer(blk_size);
            actual_read = m_pInput->read(pBuf->data(), bytes);
            if (actual_read == 0)
            {
                pBuf->release();
                break;
            }
            else
            {
                pBuf->resize(actual_read);
                m_pOutput->waitForWrite();
                if (m_bKillThread)
                {
                    pBuf->release();
                    break;
                }
                if (m_bSeekFlag)
                    pBuf->release();
                else
                {
                    m_pPlayer->outputWrite(pBuf, m_audioInfo.bits_per_sample, m_audioInfo.channels, m_audioInfo.samples_per_sec);
                }
                m_audioInfo.position += actual_read;
            }
        }
        else
        {
            break;
        }
        if (m_bSeekFlag)
        {
            m_audioInfo.position = m_nSeekPos * (size_per_second / 1000);
            m_pInput->seek(m_audioInfo.position + m_audioInfo.data_offset, SEEK_SET);
            m_pOutput->flush();
            m_bSeekFlag = false;
        }

    }

    while (!m_bKillThread && m_pOutput->isPlaying())
    {
        sleep(10);
    }

    m_state = PS_STOPED;

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

    ERR_LOG0("End of decode thread! quit.");
}

MLRESULT CMDWave::getHeadInfo(IMediaInput *pInput)
{
    m_bufInput.clear();
    pInput->seek(0, SEEK_SET);

    char            szMagic[4];
    char            magic[4];
    long            len;

    memset(&m_audioInfo, 0, sizeof(m_audioInfo));

    pInput->getSize(m_audioInfo.nMediaFileSize);

    if (pInput->read(szMagic, 4) != 4)
        return ERR_BAD_FILE_FORMAT;
    if (strncmp(szMagic, "RIFF", 4))
        return ERR_BAD_FILE_FORMAT;
    if (!read_le_long(pInput, &len))
        return ERR_BAD_FILE_FORMAT;
    if (pInput->read(szMagic, 4) != 4)
        return ERR_BAD_FILE_FORMAT;
    if (strncmp(szMagic, "WAVE", 4) != 0)
        return ERR_BAD_FILE_FORMAT;

    for (;;)
    {
        if (pInput->read(magic, 4) != 4)
            return ERR_BAD_FILE_FORMAT;
        if (!read_le_long(pInput, &len))
            return ERR_BAD_FILE_FORMAT;
        if (!strncmp("fmt ", magic, 4))
            break;
        pInput->seek(len, SEEK_CUR);
    }
    if (len < 16)
        return ERR_BAD_FILE_FORMAT;
    if (!read_le_short(pInput, &m_audioInfo.format_tag))
        return ERR_BAD_FILE_FORMAT;

    switch (m_audioInfo.format_tag)
    {
        case WAVE_FORMAT_UNKNOWN:
        case WAVE_FORMAT_ALAW:
        case WAVE_FORMAT_MULAW:
        case WAVE_FORMAT_ADPCM:
        case WAVE_FORMAT_OKI_ADPCM:
        case WAVE_FORMAT_DIGISTD:
        case WAVE_FORMAT_DIGIFIX:
        case IBM_FORMAT_MULAW:
        case IBM_FORMAT_ALAW:
        case IBM_FORMAT_ADPCM:
            return ERR_BAD_FILE_FORMAT;
    }
    read_le_short(pInput, &m_audioInfo.channels);
    read_le_long(pInput, &m_audioInfo.samples_per_sec);
    read_le_long(pInput, &m_audioInfo.avg_bytes_per_sec);
    read_le_short(pInput, &m_audioInfo.block_align);
    read_le_short(pInput, &m_audioInfo.bits_per_sample);
    if (m_audioInfo.bits_per_sample != 8 && m_audioInfo.bits_per_sample != 16)
        return ERR_BAD_FILE_FORMAT;
    len -= 16;
    if (len)
        pInput->seek(len, SEEK_CUR);

    for (;;)
    {
        if (pInput->read(magic, 4) != 4)
            return ERR_BAD_FILE_FORMAT;

        if (!read_le_long(pInput, &len))
            return ERR_BAD_FILE_FORMAT;
        if (!strncmp("data", magic, 4))
            break;
        pInput->seek(len, SEEK_CUR);
    }
    m_audioInfo.data_offset = pInput->getPos();
    m_audioInfo.length = len;
    m_audioInfo.position = 0;
    m_audioInfo.nMediaLength = (int)((double)len / (m_audioInfo.channels * m_audioInfo.samples_per_sec * (m_audioInfo.bits_per_sample / 8)) * 1000);

    return ERR_OK;
}
