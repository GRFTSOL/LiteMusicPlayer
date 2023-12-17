#pragma once

#ifndef MPlayerEngine_MDFlac_h
#define MPlayerEngine_MDFlac_h

#include "IMPlayer.h"
#include "InputBuffer.hpp"
#include "../third-parties/flac/include/FLAC/stream_decoder.h"


class MDFlac : public IMediaDecoder {
public:
    MDFlac();
    ~MDFlac();

    //
    // individual methods
    //
    cstr_t getDescription() override;
    cstr_t getFileExtentions() override;

    ResultCode open(IMediaInput *input) override;

    // Decode to @bufOut
    ResultCode decode(IFBuffer *fbOut) override;

    bool isSeekable() override;

    // media length, pos related functions, unit: ms
    uint32_t getDuration() override;
    ResultCode seek(uint32_t pos) override;

protected:
    static FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *mdflac);
    static void metadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *mdflac);
    static void errorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *mdflac);

protected:
    IMediaInput                 *_input = nullptr;

    FLAC__StreamDecoder         *_decoder = nullptr;
    int64_t                     _totalSamples = 0;
    int                         _sampleRate = 0;
    int                         _channels = 0;
    int                         _bps = 0;
    uint32_t                    _duration = 0;
    IFBuffer                    *_curFb = nullptr;

};

#endif // !defined(MPlayerEngine_MDFlac_h)
