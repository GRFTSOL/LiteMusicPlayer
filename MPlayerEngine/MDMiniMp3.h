#pragma once

#ifndef MPlayerEngine_MDMiniMp3_h
#define MPlayerEngine_MDMiniMp3_h

#include "IMPlayer.h"
#include "InputBuffer.hpp"
#include "../MediaTags/MP3InfoReader.h"
#include "../third-parties/minimp3/minimp3.h"


class MDMiniMp3 : public IMediaDecoder {
public:
    MDMiniMp3();
    ~MDMiniMp3();

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
    ResultCode getHeadInfo();
    size_t seekXingTable(uint32_t nSeekToMs);
    size_t seekVbriTable(uint32_t nSeekToMs);

protected:
    IMediaInput                 *_input = nullptr;
    InputBuffer                 _buf;

    mp3dec_t                    _mp3dec;
    MP3Info                     _info;

};

#endif // !defined(MPlayerEngine_MDMiniMp3_h)
