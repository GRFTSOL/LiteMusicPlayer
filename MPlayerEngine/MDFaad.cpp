#include "MDFaad.h"
#include "../MediaTags/MediaTags.h"
#include "../MediaTags/MP3InfoReader.h"
extern "C" {
#include "../third-parties/faad2/frontend/mp4read.h"
}


#define BPS                     16
#define INPUT_BUFF_SIZE         (1024 * 32)

MDFaad::MDFaad() {
    _aacdec = NeAACDecOpen();
}

MDFaad::~MDFaad() {
    NeAACDecClose(_aacdec);
}

cstr_t MDFaad::getDescription() {
    return "MP4/AAC file decoder";
}

cstr_t MDFaad::getFileExtentions() {
    return ".mp4|mp4 files|.aac|aac files|.m4a|m4a files";
}

ResultCode MDFaad::open(IMediaInput *input) {
    _input = input;
    _input->seek(0, SEEK_SET);

    uint8_t header[10];
    auto ret = _input->read(header, sizeof(header));
    if (ret != sizeof(header)) {
        return ERR_BAD_FILE_FORMAT;
    }

    _isMp4 = header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p';

    _input->seek(0, SEEK_SET);
    _buf.bind(input, INPUT_BUFF_SIZE);

    if (_isMp4) {
        return openMp4();
    } else {
        return openAac();
    }
}

ResultCode MDFaad::decode(IFBuffer *fb) {
    assert(fb);

    if (_isMp4) {
        return decodeMp4(fb);
    } else {
        return decodeAac(fb);
    }
}

bool MDFaad::isSeekable() {
    return true;
}

uint32_t MDFaad::getDuration() {
    return _duration;
}

ResultCode MDFaad::seek(uint32_t pos) {
    if (_isMp4) {
        uint32_t startSampleId = 0;
        if (pos > 0.1) {
            int64_t sample = (int64_t)pos * mp4config.samplerate / 1000 / _frameSize;
            uint32_t limit = INT_MAX;
            startSampleId = sample < limit ? (uint32_t)sample : limit;
        }

        mp4read_seek(startSampleId);
    } else {
        auto framesPerSec = _sampleRate / 1024.0f;
        auto index = uint32_t(pos * framesPerSec / 1000);
        if (index >= 0 && index < _frameOffsets.size()) {
            _input->seek(_frameOffsets[index], SEEK_SET);
            _buf.clear();
        }
    }

    return ERR_OK;
}

ResultCode MDFaad::openMp4() {
    auto mp4file = _input->getSource();

#ifdef DEBUG
    mp4config.verbose.header = 1;
    mp4config.verbose.tags = 1;
#endif

    if (mp4read_open((char *)mp4file) != 0) {
        ERR_LOG1("Failed to open file: %s", mp4file);
        return ERR_OPEN_FILE;
    }

    _aacdec = NeAACDecOpen();

    // Set configuration
    auto config = NeAACDecGetCurrentConfiguration(_aacdec);
    config->outputFormat = FAAD_FMT_16BIT;
    config->downMatrix = 0;
    // config->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(_aacdec, config);

    if (NeAACDecInit2(_aacdec, mp4config.asc.buf, mp4config.asc.size, &_sampleRate, &_channels) < 0) {
        ERR_LOG0("Error initializing decoder library.");
        NeAACDecClose(_aacdec);
        mp4read_close();
        return ERR_BAD_FILE_FORMAT;
    }

    _frameSize = 1024;
    mp4AudioSpecificConfig mp4ASC = {0};

    if (mp4config.asc.size) {
        if (NeAACDecAudioSpecificConfig(mp4config.asc.buf, mp4config.asc.size, &mp4ASC) >= 0) {
            if (mp4ASC.frameLengthFlag == 1) _frameSize = 960;
            if (mp4ASC.sbr_present_flag == 1 || mp4ASC.forceUpSampling) _frameSize *= 2;
        }
    }

    if (mp4ASC.samplingFrequency > 0) {
        _duration = mp4config.samples * 1000LL / mp4ASC.samplingFrequency;
    }

    mp4read_seek(0);

    return ERR_OK;
}

ResultCode MDFaad::decodeMp4(IFBuffer *fb) {
    if (mp4read_frame() != 0) {
        return ERR_EOF;
    }

    NeAACDecFrameInfo frameInfo = {0};
    auto sample_buffer = NeAACDecDecode(_aacdec, &frameInfo, mp4config.bitbuf.data, mp4config.bitbuf.size);
    if (!sample_buffer) {
        // unable to decode file, abort
        return ERR_BAD_FILE_FORMAT;
    }

    fb->set(BPS, _channels, (uint32_t)_sampleRate);
    auto size = uint32_t(frameInfo.samples * sizeof(int16_t));
    fb->resize(size);
    memcpy(fb->data(), sample_buffer, size);

    if (frameInfo.error > 0) {
        ERR_LOG1("Warning: %s\n", NeAACDecGetErrorMessage(frameInfo.error));
    }

    return ERR_OK;
}

bool MDFaad::_aacLookForHeader() {
    while (true) {
        if (_buf.size() >= 4) {
            auto p = _buf.data(), end = p + _buf.size() - 4;
            while (p <= end) {
                if ((p[0] == 0xff && (p[1] & 0xf6) == 0xf0) ||
                    (p[0] == 'A' && p[1] == 'D' && p[2] == 'I' && p[3] == 'F')) {
                    _buf.forward(uint32_t(p - _buf.data()));
                    _firstFrameOffset = _input->getPos() - (uint32_t)(_buf.data() + _buf.size() - p);
                    return true;
                } else {
                    p++;
                }
            }
            _buf.forward(_buf.size() - 3);
        } else if (!_buf.fill()) {
            return false;
        }
    }
}


void MDFaad::_aacParseAdts() {
    static int ADTS_SAMPLE_RATES[] = { 96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,7350,0,0,0};

    _sampleRate = 0;
    _duration = 0;
    _frameOffsets.clear();

    size_t frameSizeTotal = 0;

    int frames = 0;
    while (true) {
        if (_buf.size() > 7) {
            // check syncword
            auto p = _buf.data();
            if (!(p[0] == 0xFF && (p[1] & 0xF6) == 0xF0)) {
                break;
            }

            if (frames == 0) {
                _sampleRate = ADTS_SAMPLE_RATES[(p[2] & 0x3c) >> 2];
            }

            int frameSize = ((p[3] & 0x3) << 11) | (p[4] << 3) | (p[5] >> 5);
            if (frameSize == 0) {
                break;
            }

            _frameOffsets.push_back(_buf.filePosition(p));

            frameSizeTotal += frameSize;
            _buf.forward(frameSize);
            frames++;
        } else if (!_buf.fill()) {
            break;
        }
    }

    auto framesPerSec = _sampleRate / 1024.0f;
    // float bytesPerFrame = frames ? frameSizeTotal /(frames * 1000.0) : 0;

    // _bitRate = (int)(8. * bytesPerFrame * framesPerSec + 0.5);
    if (framesPerSec != 0) {
        _duration = frames * 1000.0 / framesPerSec;
    } else {
        _duration = 1;
    }
}

bool MDFaad::_aacNextFrame() {
    while (true) {
        if (_buf.size() > 7) {
            // check syncword
            auto p = _buf.data();
            if (!(p[0] == 0xFF && (p[1] & 0xF6) == 0xF0)) {
                return false;
            }

            int frameSize = ((p[3] & 0x3) << 11) | (p[4] << 3) | (p[5] >> 5);
            if (frameSize == 0) {
                return false;
            }

            if (frameSize > _buf.size()) {
                _buf.fill();
                if (frameSize > _buf.size()) {
                    return false;
                }
            }

            _buf.forward(frameSize);
            return true;
        } else {
            if (!_buf.fill()) {
                return false;
            }
        }
    }
}

bool MDFaad::_aacFindNextFrame() {
    while (true) {
        auto p = _buf.data(), end = p + _buf.size();
        while (end - p > 7) {
            if (!(p[0] == 0xFF && (p[1] & 0xF6) == 0xF0)) {
                p++;
                continue;
            }

            int frameSize = ((p[3] & 0x3) << 11) | (p[4] << 3) | (p[5] >> 5);
            if (frameSize == 0) {
                p++;
                continue;
            }

            auto offset = (uint32_t)(p - _buf.data());
            _buf.forward(offset);

            return true;
        }

        if (!_buf.fill()) {
            return false;
        }
    }
}

ResultCode MDFaad::openAac() {
    uint32_t fileSize;
    _input->getSize(fileSize);

    _id3v2TagSize = _aacGetId3v2Tag();
    _input->seek(_id3v2TagSize, SEEK_SET);

    _aacdec = NeAACDecOpen();

    auto config = NeAACDecGetCurrentConfiguration(_aacdec);
    config->defObjectType = LC;
    config->outputFormat = FAAD_FMT_16BIT;
    config->downMatrix = 0;
    // config->useOldADTSFormat = 0;
    // config->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(_aacdec, config);

    if (!_aacLookForHeader()) {
        return ERR_BAD_FILE_FORMAT;
    }

    auto p = _buf.data();
    if (p[0] == 0xFF && (p[1] & 0xF6) == 0xF0) {
        _aacParseAdts();
    } else {
        int skipSize = (p[4] & 0x80) ? 9 : 0;
        uint32_t bitrate = ((p[4 + skipSize] & 0x0F) << 19) | (p[5 + skipSize] << 11) |
            (p[6 + skipSize] << 3) | (p[7 + skipSize] & 0xE0);

        _duration = (fileSize * 8.f) / bitrate + 0.5f;
    }

    _input->seek(_firstFrameOffset, SEEK_SET);
    _buf.clear();

    _sampleRate = 0;
    _channels = 0;
    _buf.fill();
    auto bytes = NeAACDecInit(_aacdec, _buf.data(), _buf.size(), &_sampleRate, &_channels);
    if (bytes < 0) {
        // If some error initializing occured, skip the file
        ERR_LOG0("Failed to initialize decoder library.");
        NeAACDecClose(_aacdec);
        return ERR_BAD_FILE_FORMAT;
    }
    _buf.forward((int)bytes);

    // Override the logic of skipping 0-th output frame.
    NeAACDecPostSeekReset(_aacdec, 1);

    return ERR_OK;
}

ResultCode MDFaad::decodeAac(IFBuffer *fb) {
    NeAACDecFrameInfo frameInfo;

    while (true) {
        auto samples = NeAACDecDecode(_aacdec, &frameInfo, _buf.data(), _buf.size());
        _buf.forward((int)frameInfo.bytesconsumed);
        if (frameInfo.error > 0) {
            ERR_LOG1("Error: %s\n", NeAACDecGetErrorMessage(frameInfo.error));
            if (frameInfo.bytesconsumed == 0) {
                if (!_aacNextFrame()) {
                    return ERR_BAD_FILE_FORMAT;
                }
            }
        } else if (frameInfo.error == 0 && frameInfo.samples > 0) {
            auto size = uint32_t(frameInfo.samples * sizeof(int16_t));
            fb->set(BPS, _channels, (uint32_t)_sampleRate);
            fb->resize(size);
            memcpy(fb->data(), samples, size);
            return ERR_OK;
        }

        if (!_buf.fill()) {
            return ERR_EOF;
        }
    }
}

void MDFaad::close() {
    NeAACDecClose(_aacdec);
    mp4read_close();
}

uint32_t MDFaad::_aacGetId3v2Tag() {
    uint8_t buf[10];

    auto bytes = _input->read(buf, sizeof(buf));
    if (bytes != sizeof(bytes)) {
        return 0;
    }

    if (memcmp(buf, "ID3", 3) == 0) {
        auto tagsize = (buf[6] << 21) | (buf[7] << 14) | (buf[8] << 7) | (buf[9] << 0);
        tagsize += 10;

        return tagsize;
    } else {
        return 0;
    }
}
