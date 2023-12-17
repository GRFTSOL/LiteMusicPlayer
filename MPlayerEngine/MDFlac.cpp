#include "MDFlac.h"
#include "../MediaTags/MediaTags.h"
#include "../MediaTags/MP3InfoReader.h"


#define BPS                     16
#define INPUT_BUFF_SIZE         (1024 * 32)

MDFlac::MDFlac() {
}

MDFlac::~MDFlac() {
}

cstr_t MDFlac::getDescription() {
    return "Mp3 file decoder";
}

cstr_t MDFlac::getFileExtentions() {
    return ".mp3|mp3 files";
}

ResultCode MDFlac::open(IMediaInput *input) {
    _input = input;
    _input->seek(0, SEEK_SET);

    _decoder = FLAC__stream_decoder_new();
    if (_decoder == NULL) {
        return ERR_BAD_FILE_FORMAT;
    }

    FLAC__stream_decoder_set_md5_checking(_decoder, true);

    int ret = FLAC__stream_decoder_init_file(_decoder, _input->getSource(), writeCallback, metadataCallback, errorCallback, this);
    if (ret != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        ERR_LOG1("ERROR: initializing decoder: %s", FLAC__StreamDecoderInitStatusString[ret]);
        return ERR_OPEN_FILE;
    }

    return ERR_OK;
}

ResultCode MDFlac::decode(IFBuffer *fb) {
    assert(fb);

    _curFb = fb;
    if (FLAC__stream_decoder_process_single(_decoder)) {
        _curFb = nullptr;
        return ERR_OK;
    }
    _curFb = nullptr;

    return ERR_EOF;
}

bool MDFlac::isSeekable() {
    return true;
}

uint32_t MDFlac::getDuration() {
    return _duration;
}

ResultCode MDFlac::seek(uint32_t pos) {
    uint64_t sample = pos * _sampleRate / 1000;
    FLAC__stream_decoder_seek_absolute(_decoder, sample);

    return ERR_OK;
}

FLAC__StreamDecoderWriteStatus MDFlac::writeCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *mdflac) {
    auto thiz = (MDFlac *)mdflac;
    assert(thiz->_curFb);

    if (buffer [0] == NULL) {
        DBG_LOG0("ERROR: buffer [0] is NULL");
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    auto fb = thiz->_curFb;
    fb->set(thiz->_bps, thiz->_channels, thiz->_sampleRate);

    if (thiz->_bps == 16) {
        auto size = sizeof(uint16_t) * frame->header.blocksize * thiz->_channels;
        fb->resize((uint32_t)size);

        auto p = (uint16_t *)fb->data();
        for (int k = 0; k < frame->header.blocksize; k++) {
            for (auto i = 0; i < thiz->_channels; i++) {
                if (buffer[i]) {
                    *p++ = buffer[i][k];
                }
            }
        }
    } else if (thiz->_bps == 32) {
        auto size = sizeof(uint32_t) * frame->header.blocksize * thiz->_channels;
        fb->resize((uint32_t)size);

        auto p = (uint32_t *)fb->data();
        for (int k = 0; k < frame->header.blocksize; k++) {
            for (auto i = 0; i < thiz->_channels; i++) {
                if (buffer[i]) {
                    *p++ = buffer[i][k];
                }
            }
        }
    } else {
        assert(0);
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void MDFlac::metadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *mdflac) {
    auto thiz = (MDFlac *)mdflac;

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        auto &info = metadata->data.stream_info;
        thiz->_duration = info.total_samples * 1000LL / info.sample_rate;
        thiz->_totalSamples = info.total_samples;
        thiz->_sampleRate = info.sample_rate;
        thiz->_channels = info.channels;
        thiz->_bps = info.bits_per_sample;
    }
}

void MDFlac::errorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *mdflac) {
    ERR_LOG1("Got error callback: %s", FLAC__StreamDecoderErrorStatusString[status]);
}
