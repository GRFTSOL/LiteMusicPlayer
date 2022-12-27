#include "MDLibmad.h"
#include "audio.h"
#include "../MediaTags/MediaTags.h"


#define        BPS          16
#define DECODE_BUFF_SIZE        (1024 * 32)
#define INPUT_BUFF_SIZE            (1024 * 16)

#define    MAD_FLAG_MPEG_V2 MAD_FLAG_LSF_EXT

struct GetHeadInfoParam {
    IMediaInput                 *m_pInput;
    CMDLibmad                   *m_pMDLibMad;
    string                      m_bufInput;
};


/*
 * NAME:    decode->input_read()
 * DESCRIPTION:    (re)fill decoder input buffer by reading a file descriptor
*/
static enum mad_flow GetHeadInfo_decode_input_read(void *data, struct mad_stream *stream) {
    GetHeadInfoParam *pmdMad = (GetHeadInfoParam *)data;
    string &m_bufInput = pmdMad->m_bufInput;
    IMediaInput *m_pInput = pmdMad->m_pInput;
    int len;

    if (pmdMad->m_pInput->isEOF()) {
        return MAD_FLOW_STOP;
    }

    if (stream->next_frame) {
        assert((int)((cstr_t)stream->next_frame - m_bufInput.data()) > 0);
        m_bufInput.erase(0, int((cstr_t)stream->next_frame - m_bufInput.data()));
    }

    len = m_pInput->read(m_bufInput.data() + m_bufInput.size(), m_bufInput.capacity() - m_bufInput.size());
    if (len == 0) {
        if (m_pInput->isError()) {
            ERR_LOG0("m_pInput read Error!");
            return MAD_FLOW_BREAK;
        }

        assert(m_bufInput.size() >= MAD_BUFFER_GUARD);

        while (len < MAD_BUFFER_GUARD) {
            m_bufInput.append("\0", 1);
            len++;
        }
    } else {
        m_bufInput.resize(m_bufInput.size() + len);
    }

    mad_stream_buffer(stream, (const unsigned char *)m_bufInput.data(), m_bufInput.size());

    return MAD_FLOW_CONTINUE;
}


/*
 * NAME:    decode->input_read()
 * DESCRIPTION:    (re)fill decoder input buffer by reading a file descriptor
 */
static enum mad_flow decode_input_read(void *data, struct mad_stream *stream) {
    CMDLibmad *pmdMad = (CMDLibmad *)data;
    string &m_bufInput = pmdMad->m_bufInput;
    IMediaInput *m_pInput = pmdMad->m_pInput;
    int len;

    if (pmdMad->m_bSeekFlag) {
        // do seek.
        uint32_t lSeekTo;

        if (pmdMad->m_pnToc) {
            if (pmdMad->m_bXingTable) {
                lSeekTo = pmdMad->seekXingTable(pmdMad->m_nSeekPos);
            } else {
                lSeekTo = pmdMad->seekVbriTable(pmdMad->m_nSeekPos);
            }
        } else {
            double msPerFrame = (double)pmdMad->m_audioInfo.nSamplesPerFrame / pmdMad->m_audioInfo.header.samplerate * 1000;
            lSeekTo = (int)((double)pmdMad->m_nSeekPos / msPerFrame * pmdMad->m_audioInfo.nFrameSize) + pmdMad->m_audioInfo.nFirstFramePos;
        }
        assert(lSeekTo <= pmdMad->m_audioInfo.nMediaFileSize);
        m_pInput->seek(lSeekTo, SEEK_SET);
        m_bufInput.clear();
        pmdMad->m_pOutput->flush();
        pmdMad->m_bSeekFlag = false;
        stream->next_frame = nullptr;
    }

    if (m_pInput->isEOF()) {
        return MAD_FLOW_STOP;
    }

    if (stream->next_frame) {
        assert((int)((cstr_t)stream->next_frame - m_bufInput.data()) > 0);
        m_bufInput.erase(0, int((cstr_t)stream->next_frame - m_bufInput.data()));
    }

    len = m_pInput->read(m_bufInput.data() + m_bufInput.size(), m_bufInput.capacity() - m_bufInput.size());
    if (len == 0) {
        if (m_pInput->isError()) {
            ERR_LOG0("m_pInput read Error!");
            return MAD_FLOW_BREAK;
        }

        assert(m_bufInput.size() >= MAD_BUFFER_GUARD);

        while (len < MAD_BUFFER_GUARD) {
            m_bufInput.append("\0", 1);
            len++;
        }
    } else {
        m_bufInput.resize(m_bufInput.size() + len);
    }

    mad_stream_buffer(stream, (const unsigned char *)m_bufInput.data(), m_bufInput.size());

    return MAD_FLOW_CONTINUE;
}

/*
 * NAME:    decode->output()
 * DESCRIPTION: configure audio module and output decoded samples
 */
/*
static enum mad_flow decode_output(void *data, struct mad_header const *header,
                struct mad_pcm *pcm)
{
    CMDLibmad        *pmdMad = (CMDLibmad *)data;
    IMediaOutput    *pOutput = pmdMad->m_pOutput;
    IMemAllocator    *pMemAllocator = pmdMad->m_pMemAllocator;
    MLRESULT        nRet;

    mad_fixed_t const *ch1, *ch2;

    ch1 = pcm->samples[0];
    ch2 = pcm->samples[1];

    if (pcm->channels == 1)
        ch2 = 0;

    if (pOutput->getChannels() != pcm->channels || 
        pOutput->getSamplerate() != pcm->samplerate)
    {
        DBG_LOG1("output: decoded sample frequency %u Hz",pcm->samplerate);

        pOutput->isOpened();
        {
            pmdMad->m_nSeekPos = pmdMad->getPos();
            pOutput->stop();
        }
        pOutput->open(pcm->samplerate, pcm->channels, BPS);
    }

    nRet = pOutput->waitForWrite();
    if (nRet != ERR_OK)
        return MAD_FLOW_CONTINUE;

    if (pmdMad->m_bKillThread)
        return MAD_FLOW_STOP;

    IFBuffer    *pBuf = nullptr;
    pBuf = pMemAllocator->allocFBuffer(INPUT_BUFF_SIZE);
    assert(pBuf);
    uint32_t len = audio_pcm_s16le((unsigned char *)pBuf->data(), pcm->length,
        ch1, ch2, AUDIO_MODE_DITHER, &(pmdMad->m_audioStats));
    pBuf->resize(len);

    if (pmdMad->m_pPlayer->IsDspActive())
        pmdMad->m_pPlayer->DspProcess(pBuf, BPS, pcm->channels, pcm->samplerate);

    pmdMad->m_pPlayer->addVisData(pBuf, BPS, pcm->channels, pcm->samplerate);

    pOutput->write(pBuf);

    return MAD_FLOW_CONTINUE;
}
*/

static enum mad_flow error_default(void *data, struct mad_stream *stream,
    struct mad_frame *frame) {
    int *bad_last_frame = (int*)data;

    switch (stream->error) {
    case MAD_ERROR_BADCRC:
        if (*bad_last_frame) {
            mad_frame_mute(frame);
        } else {
            *bad_last_frame = 1;
        }

        return MAD_FLOW_IGNORE;
    default:
        return MAD_FLOW_CONTINUE;
    }
}



CMDLibmad::CMDLibmad() {
    OBJ_REFERENCE_INIT;

    m_pInput = nullptr;
    m_pPlayer = nullptr;
    m_pOutput = nullptr;
    m_bPaused = false;
    m_bKillThread = false;
    m_nSeekPos = 0;
    m_bSeekFlag = false;
    m_state = PS_STOPED;
    m_bufInput.reserve(INPUT_BUFF_SIZE);

    m_pnToc = nullptr;
    m_nTableSize = 0;
    m_bXingTable = false;
    m_nFileSizeOrg = 0;
}

CMDLibmad::~CMDLibmad() {
    m_threadDecode.join();

    if (m_pPlayer) {
        m_pPlayer->release();
        m_pPlayer = nullptr;
    }

    if (m_pnToc) {
        delete[] m_pnToc;
    }
}

cstr_t CMDLibmad::getDescription() {
    return "Mp3 file decoder";
}

cstr_t CMDLibmad::getFileExtentions() {
    return ".mp3|mp3 files";
}

MLRESULT CMDLibmad::getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia) {
    MLRESULT nRet;

    nRet = getHeadInfo(pInput);
    if (nRet != ERR_OK) {
        return nRet;
    }

    cstr_t szSource = pInput->getSource();
    FILE *fp = fopen(szSource, "rb");
    if (fp) {
        bool bUpdate = false;
        string artist, title, album, comment;
        string track, year, genre, composer;

        CID3v1 id3v1;
        ID3V1 tag;
        if (id3v1.getTag(fp, &tag) == ERR_OK) {
            title = tag.szTitle;
            artist = tag.szArtist;
            album = tag.szAlbum;
            year = tag.szYear;
            comment = tag.szComment;

            if (tag.byTrack != 255) {
                track = stringFromInt(tag.byTrack);
            }

            cstr_t szGenre = CID3v1::getGenreDescription(tag.byGenre);
            if (szGenre) {
                genre = szGenre;
            }

            bUpdate = true;
        }
        fclose(fp);

        CID3v2IF id3v2(ED_SYSDEF);

        nRet = id3v2.open(szSource, false, false);
        if (nRet == ERR_OK) {
            nRet = id3v2.getTags(artist,  title,  album,  comment,
                track,  year,  genre);
            if (nRet == ERR_OK) {
                bUpdate = true;
            }
        }

        if (bUpdate) {
            pMedia->setAttribute(MA_TITLE, title.c_str());
            pMedia->setAttribute(MA_ARTIST, artist.c_str());
            pMedia->setAttribute(MA_ALBUM, album.c_str());
            pMedia->setAttribute(MA_YEAR, year.c_str());
            pMedia->setAttribute(MA_COMMENT, comment.c_str());
            pMedia->setAttribute(MA_TRACK_NUMB, track.c_str());
            pMedia->setAttribute(MA_GENRE, genre.c_str());
        }
    }

    pMedia->setAttribute(MA_FORMAT, "MP3");
    pMedia->setAttribute(MA_FILESIZE, m_audioInfo.nMediaFileSize);
    pMedia->setAttribute(MA_DURATION, m_audioInfo.nMediaLength);
    pMedia->setAttribute(MA_SAMPLE_RATE, m_audioInfo.header.samplerate);
    pMedia->setAttribute(MA_CHANNELS, m_audioInfo.header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2);
    pMedia->setAttribute(MA_BITRATE, m_audioInfo.nBitRate);
    pMedia->setAttribute(MA_ISVBR, m_audioInfo.bVbr);

    pMedia->setAttribute(MA_EXTRA_INFO,
        stringPrintf("Header offset: %d\n"
        "MPEG version: %d\n"
        "MPEG layer: %d\n"
        "frames: %d\n"
        "CRC: %s\n"
        "copyrighted: %s\n",
        m_audioInfo.nFirstFramePos,
        (m_audioInfo.header.flags & MAD_FLAG_MPEG_V2) ? 2 : 1,
        m_audioInfo.header.layer,
        m_audioInfo.nFrameCount,
        m_audioInfo.header.crc_check ? "YES" : "NO",
        m_audioInfo.header.private_bits ? "YES" : "NO").c_str());

    return nRet;
}

bool CMDLibmad::isSeekable() {
    return true;
}

bool CMDLibmad::isUseOutputPlug() {
    return true;
}

MLRESULT CMDLibmad::play(IMPlayer *pPlayer, IMediaInput *pInput) {
    MLRESULT nRet;

    m_state = PS_STOPED;
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

    nRet = m_pOutput->open(m_audioInfo.header.samplerate,
        m_audioInfo.header.mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2,
        BPS);
    if (nRet != ERR_OK) {
        goto R_FAILED;
    }

    m_state = PS_PLAYING;
    m_bKillThread = false;

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

MLRESULT CMDLibmad::pause() {
    if (m_state != PS_PLAYING) {
        return ERR_PLAYER_INVALID_STATE;
    }
    m_state = PS_PAUSED;
    assert(m_pOutput);
    return m_pOutput->pause(true);
}

MLRESULT CMDLibmad::unpause() {
    if (m_state != PS_PAUSED) {
        return ERR_PLAYER_INVALID_STATE;
    }
    m_state = PS_PLAYING;
    assert(m_pOutput);
    return m_pOutput->pause(false);
}

MLRESULT CMDLibmad::stop() {
    m_bKillThread = true;

    if (m_state == PS_PAUSED) {
        assert(m_pOutput);
        m_pOutput->flush();
    }

    m_threadDecode.join();

    return ERR_OK;
}

uint32_t CMDLibmad::getLength() {
    return m_audioInfo.nMediaLength;
}

MLRESULT CMDLibmad::seek(uint32_t dwPos) {
    m_bSeekFlag = true;
    m_nSeekPos = dwPos;

    assert(m_pOutput);
    if (m_pOutput && m_state == PS_PLAYING) {
        m_pOutput->flush();
    }

    return ERR_OK;
}

uint32_t CMDLibmad::getPos() {
    if (m_pOutput && !m_bSeekFlag) {
        return m_nSeekPos + m_pOutput->getPos();
    } else {
        return m_nSeekPos;
    }
}

MLRESULT CMDLibmad::setVolume(int volume, int nBanlance) {
    if (!m_pOutput) {
        return ERR_PLAYER_INVALID_STATE;
    }

    return m_pOutput->setVolume(volume, nBanlance);
}

void CMDLibmad::decodeThread(void *lpParam) {
    CMDLibmad *pMDRow;

    pMDRow = (CMDLibmad *)lpParam;

    pMDRow->addRef();
    pMDRow->decodeThreadProc();
    pMDRow->release();
}

uint32_t CMDLibmad::decodeThreadProc() {
    m_bufInput.clear();
    m_pInput->seek(0, SEEK_SET);

    mad_decoder_init(&m_decoder, this,
        decode_input_read,
        nullptr/*decode_header*/, /*decode_filter*/nullptr,
        nullptr,
        nullptr, 0);

    int result = mad_decoder_run(&m_decoder);

    mad_decoder_finish(&m_decoder);

    while (!m_bKillThread && m_pOutput->isPlaying()) {
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

    return 0;
}

template<class T>
T* TNew(T * pointer) {
    T* p = new T;
    return p;
}

//
// for xing header
//
#define FRAMES_FLAG         0x0001
#define BYTES_FLAG          0x0002
#define TOC_FLAG            0x0004
#define VBR_SCALE_FLAG      0x0008
#define FRAMES_AND_BYTES (FRAMES_FLAG | BYTES_FLAG)
#define MAX_XING_HEADER_LEN            (32 + 4 * 6 + 100)

class CBigEndianBuffReader {
public:
    class cOutOfRangeException {
    public:
        cOutOfRangeException() {}
    };

    CBigEndianBuffReader(const uint8_t *buf, int nLen) : m_buf(buf) {
        m_bufEndPos = m_buf + nLen;
    }

    int extractI1() {
        if (m_buf + 1 > m_bufEndPos) {
            throw cOutOfRangeException();
        }

        m_buf++;

        return m_buf[0];
    }

    int extractIx(int nBytes) {
        if (m_buf + nBytes > m_bufEndPos) {
            throw cOutOfRangeException();
        }

        int x = m_buf[0];
        // big endian extract

        for (int i = 1; i < nBytes; i++) {
            x <<= 8;
            x |= m_buf[i];
        }

        m_buf += nBytes;

        return x;
    }

    void advance(int nBytes) {
        m_buf += nBytes;
        if (m_buf > m_bufEndPos) {
            throw cOutOfRangeException();
        }
    }

protected:
    const uint8_t               *m_buf, *m_bufEndPos;

};


bool CMDLibmad::getVbrInfo(mad_header *header, unsigned char const *frameBuff, int nBuffLen) {
    int nOffset;
    unsigned char const *buff;

    if (header->flags & MAD_FLAG_MPEG_V2) {
        nOffset = header->mode == MAD_MODE_SINGLE_CHANNEL ? 9 : 17;
    } else {
        nOffset = header->mode == MAD_MODE_SINGLE_CHANNEL ? 17 : 32;
    }
    nOffset += 4;

    m_audioInfo.nFrameCount = -1;
    buff = frameBuff + nOffset;
    if (m_pnToc) {
        delete[] m_pnToc;
        m_pnToc = nullptr;
    }

    if ((buff[0] == 'X' && buff[1] == 'i' && buff[2] == 'n' && buff[3] == 'g')
        || (buff[0] == 'I' && buff[1] == 'n' && buff[2] == 'f' && buff[3] == 'o')) {
        /* XING VBR-Header

        size    description
        4        'Xing' or 'Info'
        4        flags (indicates which fields are used)
        4        frames (optional)
        4        bytes (optional)
        100    toc (optional)
        4        a VBR quality indicator: 0=best 100=worst (optional)

        */
        int nHeadFlags;
        uint32_t dwQuality;
        CBigEndianBuffReader bufReader(buff + 4, nBuffLen - nOffset - 4);

        m_bXingTable = true;

        try
        {
            nHeadFlags = bufReader.extractIx(4);

            if (nHeadFlags & FRAMES_FLAG) {
                m_audioInfo.nFrameCount = bufReader.extractIx(4);
            }

            if (nHeadFlags & BYTES_FLAG) {
                m_nFileSizeOrg = bufReader.extractIx(4);
            } else {
                m_pInput->getSize(m_nFileSizeOrg);
            }

            if (nHeadFlags & TOC_FLAG) {
                m_nTableSize = 100;
                m_pnToc = new int[m_nTableSize];
                if (m_pnToc) {
                    for(int i = 0; i < m_nTableSize; i++) {
                        m_pnToc[i] = bufReader.extractI1();
                    }
                }
            }

            if (nHeadFlags & VBR_SCALE_FLAG) {
                dwQuality = bufReader.extractIx(4);
            }
        }
        catch(CBigEndianBuffReader::cOutOfRangeException &) {
            return false;
        }
    } else if (buff[0] == 'V' && buff[1] == 'B' && buff[2] == 'R' && buff[3] == 'I') {
        // VBRI header
        /* FhG VBRI Header

        size    description
        4        'VBRI' (ID)
        2        version
        2        delay
        2        quality
        4        # bytes
        4        # frames
        2        table size (for TOC)
        2        table scale (for TOC)
        2        size of table entry (max. size = 4 uint8_t (must be stored in an integer))
        2        frames per table entry

        ??        dynamic table consisting out of frames with size 1-4
        whole length in table size! (for TOC)

        */

        CBigEndianBuffReader bufReader(buff + 4, nBuffLen - nOffset - 4);

        // extract all fields from header (all mandatory)
        uint32_t dwVersion, dwQuality, dwTableScale, dwBytesPerEntry;
        float fDelay;

        m_bXingTable = false;

        try
        {
            dwVersion = bufReader.extractIx(2);
            fDelay = (float)bufReader.extractIx(2);
            dwQuality = bufReader.extractIx(2);
            m_nFileSizeOrg = bufReader.extractIx(4);
            m_audioInfo.nFrameCount = bufReader.extractIx(4);
            m_nTableSize = bufReader.extractIx(2);
            dwTableScale = bufReader.extractIx(2);
            dwBytesPerEntry = bufReader.extractIx(2);
            m_nFramesPerEntry = bufReader.extractIx(2);

            // extract TOC  (for more accurate seeking)
            m_pnToc = new int[m_nTableSize];
            if (m_pnToc) {
                for (unsigned int i = 0 ; i < m_nTableSize; i++) {
                    m_pnToc[i] = bufReader.extractIx(dwBytesPerEntry);
                }
            }
        }
        catch(CBigEndianBuffReader::cOutOfRangeException &) {
            return false;
        }
    }

    if (m_audioInfo.nFrameCount <= 0) {
        return false;
    }

    return true;
}

uint32_t CMDLibmad::seekXingTable(uint32_t nSeekToMs) {
    float fPercent;

    fPercent = nSeekToMs * 100 / (float)m_audioInfo.nMediaLength - 1;

    // interpolate in TOC to get file seek point in bytes
    int a;
    float fa, fb, fx;

    a = (int)fPercent;
    if (a >= 100) {
        return m_audioInfo.nMediaFileSize;
    } else if (a <= 0) {
        return m_audioInfo.nFirstFramePos;
    }

    fa = (float)m_pnToc[a];

    if (a < 99) {
        fb = (float)m_pnToc[a+1];
    } else {
        fb = 256.0f;
    }

    fx = fa + (fb-fa)*(fPercent-a);

    return (int)((1.0f/256.0f)*fx* (m_nFileSizeOrg)) + m_audioInfo.nMediaFileSize - m_nFileSizeOrg;
}

uint32_t CMDLibmad::seekVbriTable(uint32_t nSeekToMs) {
    unsigned int i=0, fraction = 0;
    uint32_t dwSeekPoint = 0;

    float fLengthMS;
    float fLengthMSPerTOCEntry;
    float fAccumulatedTimeMS = 0.0f ;

    fLengthMS = (float)m_audioInfo.nMediaLength;
    fLengthMSPerTOCEntry = fLengthMS / (float)m_nTableSize;

    if (nSeekToMs > fLengthMS) {
        nSeekToMs = (int)fLengthMS;
    }

    while (fAccumulatedTimeMS <= nSeekToMs) {
        dwSeekPoint += m_pnToc[i++];
        fAccumulatedTimeMS += fLengthMSPerTOCEntry;
    }

    // Searched too far; correct result
    fraction = ( (int)((((fAccumulatedTimeMS - nSeekToMs) / fLengthMSPerTOCEntry)
        + (1.0f/(2.0f*(float)m_nFramesPerEntry))) * (float)m_nFramesPerEntry));

    dwSeekPoint -= (uint32_t)((float)m_pnToc[i-1] * (float)(fraction)
        / (float)m_nFramesPerEntry);

    return dwSeekPoint + m_audioInfo.nMediaFileSize - m_nFileSizeOrg;
}

int run_get_header(struct mad_decoder *decoder) {
    enum mad_flow (*error_func)(void *, struct mad_stream *, struct mad_frame *);
    void *error_data;
    int bad_last_frame = 0;
    struct mad_stream *stream;
    struct mad_frame *frame;
    struct mad_synth *synth;
    int result = 0;
    int nFrameCheced = 0;

    string buffXingHeader;
    GetHeadInfoParam *pmdMad = (GetHeadInfoParam *)decoder->cb_data;
    MPEG_AUDIO_INFO &audioInfo = pmdMad->m_pMDLibMad->m_audioInfo;
    struct mad_header    &header_prev = audioInfo.header;
    memset(&header_prev, 0, sizeof(header_prev));

    if (decoder->input_func == 0) {
        return 0;
    }

    if (decoder->error_func) {
        error_func = decoder->error_func;
        error_data = decoder->cb_data;
    } else {
        error_func = error_default;
        error_data = &bad_last_frame;
    }

    stream = &decoder->sync->stream;
    frame = &decoder->sync->frame;
    synth = &decoder->sync->synth;

    mad_stream_init(stream);
    mad_frame_init(frame);
    mad_synth_init(synth);

    mad_stream_options(stream, decoder->options);

#define MAX_RETYIES         32
    stream->error = MAD_ERROR_BUFLEN;
    for (int i = 0; i < MAX_RETYIES && stream->error == MAD_ERROR_BUFLEN; i++) {
        switch (decoder->input_func(decoder->cb_data, stream)) {
        case MAD_FLOW_STOP:
            goto done;
        case MAD_FLOW_BREAK:
            goto fail;
        case MAD_FLOW_IGNORE:
            continue;
        case MAD_FLOW_CONTINUE:
            break;
        }

        while (1) {
            if (mad_header_decode(&frame->header, stream) == -1) {
                if (!MAD_RECOVERABLE(stream->error)) {
                    break;
                }

                switch (error_func(error_data, stream, frame)) {
                case MAD_FLOW_STOP:
                    goto done;
                case MAD_FLOW_BREAK:
                    goto fail;
                case MAD_FLOW_IGNORE:
                case MAD_FLOW_CONTINUE:
                default:
                    continue;
                }
            }

            struct mad_header        *header = &frame->header;

            if (nFrameCheced == 0) {
                // copy xing and vbri header
                if (pmdMad->m_pMDLibMad->getVbrInfo(header, stream->this_frame, int(stream->bufend - stream->this_frame))) {
                    audioInfo.bVbr = true;
                } else {
                    audioInfo.bVbr = false;
                }

                assert((cstr_t)stream->this_frame - pmdMad->m_bufInput.c_str() >= 0);
                audioInfo.nFirstFramePos = pmdMad->m_pInput->getPos() +
                (int)((cstr_t)stream->this_frame - pmdMad->m_bufInput.c_str()) - pmdMad->m_bufInput.size();
            }

            nFrameCheced++;

            if (header_prev.layer != header->layer ||
                header_prev.mode != header->mode ||
                header_prev.samplerate != header->samplerate) {
                header_prev = *header;
            } else {
                int nPadding;
                if (header->flags & MAD_FLAG_PADDING) {
                    nPadding = 1;
                } else {
                    nPadding = 0;
                }
                if (header->layer == MAD_LAYER_I) {
                    audioInfo.nSamplesPerFrame = 384;
                    audioInfo.nFrameSize = (12 * header->bitrate / header->samplerate + nPadding) * 4;
                } else {
                    if (header->flags & MAD_FLAG_MPEG_V2) {
                        // LSF
                        audioInfo.nSamplesPerFrame = 576;
                        audioInfo.nFrameSize = 72 * header->bitrate / header->samplerate + nPadding;
                    } else {
                        audioInfo.nSamplesPerFrame = 1152;
                        audioInfo.nFrameSize = 144 * header->bitrate / header->samplerate + nPadding;
                    }
                }

                pmdMad->m_pInput->getSize(audioInfo.nMediaFileSize);

                if (audioInfo.bVbr && audioInfo.nFrameCount > 0) {
                    audioInfo.nMediaLength = (int)((double)audioInfo.nSamplesPerFrame * audioInfo.nFrameCount * 1000 / header->samplerate);
                    assert(audioInfo.nMediaLength > 0);
                    if (audioInfo.nMediaLength <= 0) {
                        audioInfo.nMediaLength = 1;
                    }
                    audioInfo.nFrameSize = (audioInfo.nMediaFileSize - audioInfo.nFirstFramePos) / audioInfo.nFrameCount;
                    audioInfo.nBitRate = (int)(((double)audioInfo.nMediaFileSize - audioInfo.nFirstFramePos) * 8 * 1000 / audioInfo.nMediaLength);
                    goto done;
                } else {
                    audioInfo.nFrameCount = (audioInfo.nMediaFileSize - audioInfo.nFirstFramePos) / audioInfo.nFrameSize;
                    audioInfo.nMediaLength = int((double)audioInfo.nSamplesPerFrame * audioInfo.nFrameCount * 1000 / header->samplerate);
                    audioInfo.nBitRate = audioInfo.header.bitrate;
                    goto done;
                }
            }
        }
    }

fail:
    result = -1;

done:
    mad_synth_finish(synth);
    mad_frame_finish(frame);
    mad_stream_finish(stream);

    return result;
}

/*
 * NAME:    decoder->run()
 * DESCRIPTION:    run the decoder thread either synchronously or asynchronously
 */
int CMDLibmad::mad_decoder_run(struct mad_decoder *decoder) {
    int result;

    // decoder->sync = ((sync*)malloc(sizeof(*decoder->sync));
    decoder->sync = new mad_decoder::mad_sync;
    if (decoder->sync == 0) {
        return -1;
    }

    result = run_sync(decoder);

    delete decoder->sync;
    decoder->sync = 0;

    return result;
}

int CMDLibmad::run_sync(struct mad_decoder *decoder) {
    enum mad_flow (*error_func)(void *, struct mad_stream *, struct mad_frame *);
    void *error_data;
    int bad_last_frame = 0;
    struct mad_stream *stream;
    struct mad_frame *frame;
    struct mad_synth *synth;

    IFBuffer *pBuf = nullptr;
    int nChannels = 0, nSampleRate = 0;

    int result = 0;

    if (decoder->input_func == 0) {
        return 0;
    }

    if (decoder->error_func) {
        error_func = decoder->error_func;
        error_data = decoder->cb_data;
    } else {
        error_func = error_default;
        error_data = &bad_last_frame;
    }

    stream = &decoder->sync->stream;
    frame = &decoder->sync->frame;
    synth = &decoder->sync->synth;

    mad_stream_init(stream);
    mad_frame_init(frame);
    mad_synth_init(synth);

    mad_stream_options(stream, decoder->options);

    do {
        switch (decoder->input_func(decoder->cb_data, stream)) {
        case MAD_FLOW_STOP:
            goto done;
        case MAD_FLOW_BREAK:
            goto fail;
        case MAD_FLOW_IGNORE:
            continue;
        case MAD_FLOW_CONTINUE:
            break;
        }

        while (1) {
            if (decoder->header_func) {
                if (mad_header_decode(&frame->header, stream) == -1) {
                    if (!MAD_RECOVERABLE(stream->error)) {
                        break;
                    }

                    switch (error_func(error_data, stream, frame)) {
                    case MAD_FLOW_STOP:
                        goto done;
                    case MAD_FLOW_BREAK:
                        goto fail;
                    case MAD_FLOW_IGNORE:
                    case MAD_FLOW_CONTINUE:
                    default:
                        continue;
                    }
                }

                switch (decoder->header_func(decoder->cb_data, &frame->header)) {
                case MAD_FLOW_STOP:
                    goto done;
                case MAD_FLOW_BREAK:
                    goto fail;
                case MAD_FLOW_IGNORE:
                    continue;
                case MAD_FLOW_CONTINUE:
                    break;
                }
            }

            if (mad_frame_decode(frame, stream) == -1) {
                if (!MAD_RECOVERABLE(stream->error)) {
                    break;
                }

                switch (error_func(error_data, stream, frame)) {
                case MAD_FLOW_STOP:
                    goto done;
                case MAD_FLOW_BREAK:
                    goto fail;
                case MAD_FLOW_IGNORE:
                    break;
                case MAD_FLOW_CONTINUE:
                default:
                    continue;
                }
            } else {
                bad_last_frame = 0;
            }

            if (decoder->filter_func) {
                switch (decoder->filter_func(decoder->cb_data, stream, frame)) {
                case MAD_FLOW_STOP:
                    goto done;
                case MAD_FLOW_BREAK:
                    goto fail;
                case MAD_FLOW_IGNORE:
                    continue;
                case MAD_FLOW_CONTINUE:
                    break;
                }
            }

            mad_synth_frame(synth, frame);

            struct mad_header const *header = &frame->header;
            struct mad_pcm *pcm = &synth->pcm;
            mad_fixed_t const *ch1, *ch2;

            ch1 = pcm->samples[0];
            ch2 = pcm->samples[1];

            if (pcm->channels == 1) {
                ch2 = 0;
            }

            if (nChannels != pcm->channels ||
                nSampleRate != pcm->samplerate) {
                DBG_LOG1("output: decoded sample frequency %u Hz",pcm->samplerate);

                if (m_pOutput->isOpened()) {
                    // if (pBuf && outputWrite(pBuf, nChannels, nSampleRate))
                    //      pBuf = nullptr;

                    // while (m_pOutput->isPlaying())
                    //      sleep(10);

                    // m_nSeekPos = getPos();
                    m_pOutput->stop();
                }
                m_pOutput->open(pcm->samplerate, pcm->channels, BPS);
                nChannels = pcm->channels;
                nSampleRate = pcm->samplerate;
            }

            pBuf = m_pMemAllocator->allocFBuffer(DECODE_BUFF_SIZE);
            uint32_t len = audio_pcm_s16le((unsigned char *)pBuf->data() + pBuf->size(), pcm->length,
                ch1, ch2, AUDIO_MODE_DITHER, &m_audioStats);
            pBuf->resize(len);
            m_pOutput->waitForWrite();

            if (m_bKillThread) {
                pBuf->release();
                goto done;
            }

            if (m_bSeekFlag) {
                pBuf->release();
                break;
            }

            m_pPlayer->outputWrite(pBuf, BPS, pcm->channels, pcm->samplerate);
        }
    }
    while (stream->error == MAD_ERROR_BUFLEN);

fail:
    result = -1;

done:
    if (!m_bKillThread) {
        while (m_pOutput->isPlaying()) {
            sleep(10);
        }
    }

    mad_synth_finish(synth);
    mad_frame_finish(frame);
    mad_stream_finish(stream);

    return result;
}

/*
bool CMDLibmad::outputWrite(IFBuffer *pBuf, int nChannels, int nSampleRate)
{
    m_pOutput->waitForWrite();

    if (m_bKillThread)
        return false;

    if (m_bSeekFlag)
    {
        pBuf->release();
        return true;
    }

    if (m_pPlayer->IsDspActive())
        m_pPlayer->DspProcess(pBuf, BPS, nChannels, nSampleRate);

    m_pPlayer->addVisData(pBuf, BPS, nChannels, nSampleRate);
    m_pOutput->write(pBuf);

    return true;
}*/

MLRESULT CMDLibmad::getHeadInfo(IMediaInput *pInput) {
    m_bufInput.clear();

    int result = -1;
    GetHeadInfoParam param;
    param.m_pInput = pInput;
    param.m_pMDLibMad = this;
    param.m_bufInput.reserve(INPUT_BUFF_SIZE);

    memset(&m_audioInfo, 0, sizeof(m_audioInfo));

    mad_decoder_init(&m_decoder, &param,
        GetHeadInfo_decode_input_read,
        nullptr, /*decode_filter*/nullptr,
        nullptr,
        nullptr, 0);

    m_decoder.sync = new mad_decoder::mad_sync;
    if (m_decoder.sync) {
        result = run_get_header(&m_decoder);

        delete m_decoder.sync;
        m_decoder.sync = 0;
    }

    mad_decoder_finish(&m_decoder);

    if (result == 0) {
        return ERR_OK;
    } else {
        return ERR_PLAYER_INVALID_FILE;
    }
}
