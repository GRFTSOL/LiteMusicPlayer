#define MINIMP3_IMPLEMENTATION
#include "MDMiniMp3.h"
#include "../MediaTags/MediaTags.h"
#include "../MediaTags/MP3InfoReader.h"


#define BPS                     16
#define INPUT_BUFF_SIZE         (1024 * 32)

MDMiniMp3::MDMiniMp3() {
    memset(&_mp3dec, 0, sizeof(_mp3dec));
    mp3dec_init(&_mp3dec);
}

MDMiniMp3::~MDMiniMp3() {
}

cstr_t MDMiniMp3::getDescription() {
    return "Mp3 file decoder";
}

cstr_t MDMiniMp3::getFileExtentions() {
    return ".mp3|mp3 files";
}

ResultCode MDMiniMp3::open(IMediaInput *input) {
    _input = input;
    _input->seek(0, SEEK_SET);

    _buf.bind(input, INPUT_BUFF_SIZE);

    return getHeadInfo();
}

ResultCode MDMiniMp3::decode(IFBuffer *fb) {
    assert(fb);

    mp3dec_frame_info_t info;
    int16_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];

    while (true) {
        if (_buf.size() < 1024 * 2) {
            _buf.fill();
        }

        StringView buf = _buf.buf();

        auto samples = mp3dec_decode_frame(&_mp3dec, (uint8_t *)buf.data, buf.len, pcm, &info);
        if (samples > 0) {
            fb->set(BPS, info.channels, _info.sampleRate);

            auto size = uint32_t(samples * sizeof(int16_t) * info.channels);
            fb->resize(size);
            memcpy(fb->data(), pcm, size);

            _buf.forward(info.frame_bytes);

            return ERR_OK;
        }

        if (info.frame_bytes > 0) {
            _buf.forward(info.frame_bytes);
        } else {
            if (!_buf.fill()) {
                return ERR_EOF;
            }
        }
    }
}

bool MDMiniMp3::isSeekable() {
    return true;
}

uint32_t MDMiniMp3::getDuration() {
    return _info.duration;
}

ResultCode MDMiniMp3::seek(uint32_t pos) {
    // do seek.
    size_t offset;

    if (!_info.toc.empty()) {
        if (_info.isXingToc) {
            offset = seekXingTable(pos);
        } else {
            offset = seekVbriTable(pos);
        }
    } else {
        double msPerFrame = (double)_info.samplesPerFrame / _info.sampleRate * 1000;
        offset = (int)((double)pos / msPerFrame * _info.frameSize) + _info.firstFrameOffset;
    }

    assert(offset <= _info.fileSize);
    _input->seek((uint32_t)offset, SEEK_SET);
    _buf.clear();

    return ERR_OK;
}

size_t MDMiniMp3::seekXingTable(uint32_t nSeekToMs) {
    float percent = nSeekToMs * 100 / (float)_info.duration - 1;

    // interpolate in TOC to get file seek point in bytes
    int a = (int)percent;
    if (a >= 100) {
        return _info.fileSize;
    } else if (a <= 0) {
        return _info.firstFrameOffset;
    }

    float fa = (float)_info.toc[a];

    float fb;
    if (a < 99) {
        fb = (float)_info.toc[a + 1];
    } else {
        fb = 256.0f;
    }

    float fx = fa + (fb - fa) * (percent - a);

    return (int)((1.0f / 256.0f) * fx * (_info.bytesFrames)) + _info.firstFrameOffset;
}

size_t MDMiniMp3::seekVbriTable(uint32_t nSeekToMs) {
    if (nSeekToMs > _info.duration) {
        nSeekToMs = _info.duration;
    }

    int i = 0;
    uint32_t seekOffset = 0;
    float timeOffset = 0.0f;
    float lengthPerTocEntry = _info.duration / (float)_info.toc.size();
    for (; timeOffset <= nSeekToMs; i++) {
        seekOffset += _info.toc[i];
        timeOffset += lengthPerTocEntry;
    }

    // Searched too far; correct result
    auto fraction = ((int)((((timeOffset - nSeekToMs) / lengthPerTocEntry)
        + (1.0f / (2.0f * _info.tocVbriFramesPerEntry))) * _info.tocVbriFramesPerEntry));

    seekOffset -= (uint32_t)((float)_info.toc[i - 1] * (float)(fraction)
        / (float)_info.tocVbriFramesPerEntry);

    return seekOffset + _info.firstFrameOffset;
}

ResultCode MDMiniMp3::getHeadInfo() {
    FilePtr fp;
    if (fp.open(_input->getSource(), "rb")) {
        readMP3Info(fp.ptr(), _info);
    }

    return ERR_OK;
}
