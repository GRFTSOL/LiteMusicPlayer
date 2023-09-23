#include "MP3InfoReader.h"
#include "ID3/ID3v2.h"
#include "ID3/ID3Helper.h"
#include "../TinyJS/utils/BinaryFileStream.h"


// sampling rates in hertz: 1. index = MPEG version ID, 2. index = sampling rate index
const uint32_t SAMPLE_RATES[4][3] = {
    {11025, 12000, 8000,  },    // MPEG 2.5
    {0,     0,     0,     },    // reserved
    {22050, 24000, 16000, },    // MPEG 2
    {44100, 48000, 32000  }     // MPEG 1
};

// bitrates: 1. index = LSF, 2. index = layer, 3. index = bitrate index
const uint32_t BIT_RATES[2][3][15] = {
    {    // MPEG 1
        {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},    // Layer1
        {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},    // Layer2
        {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,}     // Layer3
    },
    {    // MPEG 2, 2.5
        {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},       // Layer1
        {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},            // Layer2
        {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}             // Layer3
    }
};

// Samples per Frame: 1. index = LSF, 2. index = layer
const uint32_t SAMPLES_PER_FRAMES[2][3] = {
    {    // MPEG 1
        384,        // Layer1
        1152,       // Layer2
        1152        // Layer3
    },
    {    // MPEG 2, 2.5
        384,        // Layer1
        1152,       // Layer2
        576         // Layer3
    }
};

// Samples per Frame / 8
const uint32_t COEFFICENTS[2][3] = {
    {    // MPEG 1
        12,         // Layer1    (must be multiplied with 4, because of slot size)
        144,        // Layer2
        144         // Layer3
    },
    {    // MPEG 2, 2.5
        12,         // Layer1    (must be multiplied with 4, because of slot size)
        144,        // Layer2
        72          // Layer3
    }
};

// slot size per layer
const uint32_t SLOT_SIZES[3] = {
    4,              // Layer1
    1,              // Layer2
    1               // Layer3
};

// allowed combination of bitrate (1.index) and mono (2.index)
const bool ALLOWED_MODES[15][2] = {
    // {stereo, intensity stereo, dual channel allowed,single channel allowed}
    { true, true },         // free mode
    { false, true },        // 32
    { false, true },        // 48
    { false, true },        // 56
    { true, true },         // 64
    { false, true },        // 80
    { true, true },         // 96
    { true, true },         // 112
    { true, true },         // 128
    { true, true },         // 160
    { true, true },         // 192
    { true, false },        // 224
    { true, false },        // 256
    { true, false },        // 320
    { true, false }         // 384
};

enum { PRE_READ_BUFF_SIZE   = 256 };

int findFirstFrame(FILE *fp, uint8_t byHeader[PRE_READ_BUFF_SIZE]) {
    try {
        BinaryFileInputStream is(fp);

        string buf = is.readString(sizeof(ID3v2Header));
        ID3v2Header *header = (ID3v2Header *)buf.c_str();

        if (strncmp(header->szID, ID3_TAGID, ID3_TAGIDSIZE) == 0) {
            // 略过 ID3v2 tag
            int len = syncBytesToUInt32(header->bySize);
            is.forward(len);
        }

        uint8_t prev = is.readUInt8();
        while (!is.isEof()) {
            uint8_t cur = is.readUInt8();
            if (prev == 0xFF && cur != 0xFF
                && (cur & 0xE0) == 0xE0) {
                is.forward(-2);
                is.readBuf(byHeader, PRE_READ_BUFF_SIZE);
                return (int)is.offset() - PRE_READ_BUFF_SIZE;
            }
            prev = cur;
        }
    } catch (std::exception &e) {
        DBG_LOG1("Failed to findFirstFrame: %s", e.what());
    }

    return -1;
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
    CBigEndianBuffReader(const uint8_t *buf, int nLen) : m_buf(buf) {
        m_bufEndPos = m_buf + nLen;
    }

    int extractI1() {
        if (m_buf + 1 > m_bufEndPos) {
            return -1;
        }

        m_buf++;

        return m_buf[0];
    }

    int extractIx(int nBytes) {
        if (m_buf + nBytes > m_bufEndPos) {
            return -1;
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

    bool isAvailable(int nBytes) {
        return m_buf + nBytes <= m_bufEndPos;
    }

    bool advance(int nBytes) {
        m_buf += nBytes;
        return m_buf <= m_bufEndPos;
    }

protected:
    const uint8_t               *m_buf, *m_bufEndPos;

};


int getVbrInfoFrameCount(uint8_t const *frameBuff, int nBuffLen, MP3Info &info) {
    int nOffset;

    if (info.version == MP3Info::MPEG2 || info.version == MP3Info::MPEG25) {
        nOffset = info.channelMode == MP3Info::SingleChannel ? 9 : 17;
    } else {
        nOffset = info.channelMode == MP3Info::SingleChannel ? 17 : 32;
    }
    nOffset += 4;

    auto buff = frameBuff + nOffset;

    int nFrameCount = -1;

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
        CBigEndianBuffReader bufReader(buff + 4, nBuffLen - nOffset - 4);

        if (!bufReader.isAvailable(4)) {
            return -1;
        }
        int nHeadFlags = bufReader.extractIx(4);

        if (nHeadFlags & FRAMES_FLAG) {
            if (!bufReader.isAvailable(4)) {
                return -1;
            }
            nFrameCount = bufReader.extractIx(4);
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
        uint32_t dwVersion, dwQuality;
        float fDelay;

        if (!bufReader.isAvailable(2 + 2 + 2 + 4 + 4)) {
            return -1;
        }

        dwVersion = bufReader.extractIx(2);
        fDelay = (float)bufReader.extractIx(2);
        dwQuality = bufReader.extractIx(2);
        /* int nFileSizeOrg = */bufReader.extractIx(4);
        nFrameCount = bufReader.extractIx(4);
    }

    return nFrameCount;
}

bool parseFrameHeader(uint8_t byHeader[PRE_READ_BUFF_SIZE], size_t headerPos, size_t fileSize, MP3Info &infoOut) {

    // get MPEG version [bit 11,12]
    infoOut.version = (MP3Info::MPAVersion)((byHeader[1] >> 3) & 0x03); // mask only the rightmost 2 bits
    if (infoOut.version == MP3Info::MPEGReserved) {
        return false;
    }

    // true means lower sampling frequencies (=MPEG2/MPEG2.5)
    bool isLSF = infoOut.version == MP3Info::MPEG1 ? false : true;

    // get layer (0 = layer1, 2 = layer2, ...)  [bit 13,14]
    infoOut.layer = (MP3Info::MPALayer)(3 - ((byHeader[1] >> 1) & 0x03));
    if (infoOut.layer == MP3Info::LayerReserved) {
        return false;
    }

    // protection bit (inverted) [bit 15]
    infoOut.hasCRC = !((byHeader[1]) & 0x01);

    // bitrate [bit 16..19]
    uint8_t bBitrateIndex = (uint8_t)((byHeader[2] >> 4) & 0x0F);
    if (bBitrateIndex == 0x0F) { // all bits set is reserved
        return false;
    }
    infoOut.bitRate = BIT_RATES[isLSF][infoOut.layer][bBitrateIndex]; // convert from kbit to bit
    if (infoOut.bitRate == 0) { // means free bitrate (is unsupported yet)
        return false;
    }

    // sampling rate [bit 20,21]
    uint8_t bIndex = (uint8_t)((byHeader[2] >> 2) & 0x03);
    if (bIndex == 0x03) { // all bits set is reserved
        return false;
    }
    infoOut.sampleRate = SAMPLE_RATES[infoOut.version][bIndex];

    // padding bit [bit 22]
    uint32_t paddingSize = 1 * ((byHeader[2] >> 1) & 0x01); // in Slots (always 1)

    infoOut.samplesPerFrame = SAMPLES_PER_FRAMES[isLSF][infoOut.layer];

    // private bit [bit 23]
    infoOut.isPrivate = (byHeader[2]) & 0x01;

    // channel mode [bit 24,25]
    infoOut.channelMode = (MP3Info::ChannelMode)((byHeader[3] >> 6) & 0x03);

    // mode extension [bit 26,27]
    infoOut.modeExt = (uint8_t)((byHeader[3] >> 4) & 0x03);

    // determine the bound for intensity stereo
    if (infoOut.channelMode == MP3Info::JointStereo) {
        // only valid for intensity stereo
        // uint16_t m_wBound = 4 + m_ModeExt * 4;
    }

    // copyright bit [bit 28]
    infoOut.isCopyRight = (byHeader[3] >> 3) & 0x01;

    // original bit [bit 29]
    infoOut.isOriginal = (byHeader[3] >> 2) & 0x01;

    // emphasis [bit 30,31]
    infoOut.emphasis = (MP3Info::Emphasis)((byHeader[3]) & 0x03);
    if (infoOut.emphasis == MP3Info::EmphReserved) {
        return false;
    }

    // extended check for layer II
    if (infoOut.layer == MP3Info::Layer2) {
        // MPEG 1
        if (infoOut.version == MP3Info::MPEG1) {
            if (!ALLOWED_MODES[bBitrateIndex][infoOut.channelMode == MP3Info::SingleChannel]) {
                return false;
            }

            // which allocation table is used
            switch (infoOut.bitRate / (infoOut.channelMode == MP3Info::SingleChannel ? 1 : 2)) {
            case 32:
            case 48:
                if (infoOut.sampleRate == 32000) {
                    infoOut.allocationTableIndex = 3; // table d
                } else {
                    infoOut.allocationTableIndex = 2; // table c
                }
                break;
            case 56:
            case 64:
            case 80:
                if (infoOut.sampleRate != 48000) {
                    infoOut.allocationTableIndex = 0; // table a
                    break;
                }
            case 96:
            case 112:
            case 128:
            case 160:
            case 192:
                if (infoOut.sampleRate != 48000) {
                    infoOut.allocationTableIndex = 1; // table b
                    break;
                } else {
                    infoOut.allocationTableIndex = 0; // table a
                }
                break;
            }
        } else { // MPEG 2/2.5
            infoOut.allocationTableIndex = 4;
        }
    }

    int nSamplesPerFrame = SAMPLES_PER_FRAMES[isLSF][infoOut.layer];
    int nFrameSize = (((COEFFICENTS[isLSF][infoOut.layer] * infoOut.bitRate * 1000 / infoOut.sampleRate) + paddingSize)) * SLOT_SIZES[infoOut.layer];

    int64_t nFrameCount = getVbrInfoFrameCount(byHeader, PRE_READ_BUFF_SIZE, infoOut);
    if (nFrameCount < 0) {
        if (nFrameSize > 0) {
            nFrameCount = (fileSize - headerPos) / nFrameSize;
        }
    }

    if (nFrameCount > 0) {
        infoOut.duration = int((double)nSamplesPerFrame * nFrameCount * 1000 / infoOut.sampleRate + 0.5);
    } else if (infoOut.bitRate > 0) {
        infoOut.duration = (uint32_t)((fileSize - headerPos) * 8 / (double)infoOut.bitRate + 0.5);
    } else {
        infoOut.duration = 0;
    }
    return true;
}

void findHeader(BinaryFileInputStream &is) {
    uint8_t prev = is.readUInt8();
    while (!is.isEof()) {
        uint8_t cur = is.readUInt8();
        if (prev == 0xFF && cur != 0xFF
            && (cur & 0xE0) == 0xE0) {
            is.forward(-2);
            return;
        }
        prev = cur;
    }

    throw std::exception();
}

bool readMP3Info(FILE *fp, MP3Info &infoOut) {
    uint8_t byHeader[PRE_READ_BUFF_SIZE];

    fseek(fp, 0, SEEK_SET);

    try {
        BinaryFileInputStream is(fp);

        auto fileSize = is.size();
        string buf = is.readString(sizeof(ID3v2Header));
        ID3v2Header *header = (ID3v2Header *)buf.c_str();

        if (strncmp(header->szID, ID3_TAGID, ID3_TAGIDSIZE) == 0) {
            // 略过 ID3v2 tag
            int len = syncBytesToUInt32(header->bySize);
            is.forward(len);
        } else {
            is.setOffset(0);
        }

        while (true) {
            if (is.offset() != 0) {
                // 如果 mp3 从文件头开始，则不需要 findHeader()
                findHeader(is);
            }

            auto headerPos = is.offset();
            is.readBuf(byHeader, PRE_READ_BUFF_SIZE);

            if (parseFrameHeader(byHeader, headerPos, fileSize, infoOut)) {
                return true;
            }
        }

    } catch (std::exception &e) {
        DBG_LOG1("Failed to findFirstFrame: %s", e.what());
        return false;
    }
}
