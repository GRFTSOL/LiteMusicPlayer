#pragma once

#ifndef MPlayerEngine_MDFaad_h
#define MPlayerEngine_MDFaad_h

#include "IMPlayer.h"
#include "InputBuffer.hpp"
#include "../third-parties/faad2/include/neaacdec.h"


class MDFaad : public IMediaDecoder {
public:
    MDFaad();
    ~MDFaad();

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
    ResultCode openMp4();
    ResultCode openAac();
    ResultCode decodeMp4(IFBuffer *fbOut);
    ResultCode decodeAac(IFBuffer *fbOut);

    void close();

    uint32_t _aacGetId3v2Tag();
    bool _aacLookForHeader();
    void _aacParseAdts();
    bool _aacNextFrame();
    bool _aacFindNextFrame();

protected:
    IMediaInput                 *_input = nullptr;
    InputBuffer                 _buf;

    NeAACDecHandle              _aacdec;

    uint8_t                     *_bufDec = nullptr;
    unsigned long               _sampleRate = 0;
    uint8_t                     _channels = 0;
    uint32_t                    _frameSize = 1024;
    uint32_t                    _duration = 0;
    bool                        _isMp4 = true;  // false: aac

    // For .aac file
    uint32_t                    _id3v2TagSize = 0;
    uint32_t                    _firstFrameOffset = 0;
    VecInts                     _frameOffsets;

};

#endif // !defined(MPlayerEngine_MDFaad_h)
