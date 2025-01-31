#pragma once

#ifndef _IMPLAYER_H_
#define _IMPLAYER_H_

#include "../Utils/Utils.h"
#include "IPlayerCore.hpp"


/**
 * 传统的播放器实现架构，从 Input, Decoder, Output, DSP, Visualizer 都定义接口，来实现一遍.
 */

struct IMediaDecoder;
struct IMediaOutput;
struct IMediaInput;
struct IVisualizer;
struct IDSP;

typedef int ResultCode;

enum MPLAYER_ERROR_CODE {
    ERR_AUDIO_DECODE_INITFAILED = ERR_PLAYER_ERROR_BASE + 1,
    ERR_END_OF_STREAM,
    ERR_DECODE_FAILED,
    ERR_BUFFER_FULL,                 // 缓冲区已经写满了，需要等待
    ERR_NOT_INITED,                  // 未初始化
    ERR_NOT_SUPPORT,
    ERR_READ_ONLY,

    ERR_MI_OPEN_SRC,
    ERR_MI_NOT_FOUND,                // the media source doesn't exist.
    ERR_MI_SEEK,
    ERR_MI_READ,
    ERR_MI_WRITE,

    ERR_PLAYER_INVALID_STATE,        // Invalid state of current object.
    ERR_PLAYER_INVALID_FILE,
    ERR_SOUND_DEVICE_OPEN,
    ERR_SOUND_DEVICE_WRITE,
    ERR_DECODER_INNER_ERROR,
    ERR_DECODER_UNSUPPORTED_FEATURE,
    ERR_DECODER_INIT_FAILED,
    ERR_DISK_FULL,
    ERR_CREATE_THREAD,
    ERR_INVALID_HANDLE,
    ERR_NO_DEVICE,

    ERR_END_OF_PLAYLIST,             // end of playlist, prev, next will return this value
    ERR_EMPTY_PLAYLIST,
};

enum MPInterfaceType {
    MPIT_INVALID                = 0,
    MPIT_INPUT                  = 1,
    MPIT_OUTPUT                 = 2,
    MPIT_DECODE                 = 3,
    MPIT_VIS                    = 4,
    MPIT_DSP                    = 5,
    MPIT_GENERAL_PLUGIN         = 6,
};

struct IFBuffer {
    virtual ~IFBuffer() { }

    virtual void set(int bps, int channels, int sampleRate) = 0;

    virtual uint8_t *data() = 0;
    virtual uint32_t size() = 0;
    virtual uint32_t capacity() = 0;
    virtual void resize(uint32_t size) = 0;
    virtual ResultCode reserve(uint32_t capacity) = 0;

    virtual int bps() = 0;
    virtual int channels() = 0;
    virtual int sampleRate() = 0;

};

#define VIS_N_WAVE_SAMPLE   512
#define VIS_N_SPTR_SAMPLE   512

struct VisParam {
    int                         nChannels;          // number of channels
    unsigned char               spectrumData[2][VIS_N_WAVE_SAMPLE]; //
    unsigned char               waveformData[2][VIS_N_SPTR_SAMPLE];
};

struct IVisualizer {
    virtual ~IVisualizer() { }

    virtual int render(VisParam *visParam) = 0;
};

struct IDSP {
    virtual ~IDSP() { }

    virtual void process(IFBuffer *fb) = 0;
};

struct IMediaOutput {
    virtual ~IMediaOutput() { }

    virtual cstr_t getDescription() = 0;

    virtual ResultCode waitForWrite(int timeOutMs) = 0;

    virtual ResultCode write(IFBuffer *fb) = 0;
    virtual ResultCode flush() = 0;

    virtual ResultCode play() = 0;
    virtual ResultCode pause() = 0;
    virtual bool isPlaying() = 0;
    virtual ResultCode stop() = 0;

    virtual uint32_t getPos() = 0;

    // volume
    virtual ResultCode setVolume(int volume, int banlance) = 0;
    virtual int getVolume() = 0;

};

using IMediaOutputPtr = std::shared_ptr<IMediaOutput>;

struct IMediaInput {
    virtual ~IMediaInput() { }

    // if FAILED, return ERR_MI_OPEN_SRC, ERR_MI_NOT_FOUND
    virtual ResultCode open(cstr_t mediaUr) = 0;
    virtual uint32_t read(void *buf, uint32_t size) = 0;
    // nOrigin: SEEK_SET, SEEK_CUR, SEEK_END
    virtual ResultCode seek(uint32_t offset, int origin) = 0;
    virtual ResultCode getSize(uint32_t &size) = 0;
    virtual uint32_t getPos() = 0;

    virtual bool isEOF() = 0;
    virtual bool isError() = 0;

    virtual void close() = 0;

    virtual cstr_t getSource() = 0;

};

using IMediaInputPtr = std::shared_ptr<IMediaInput>;

struct IMediaDecoder {
    virtual ~IMediaDecoder() { }

    //
    // individual methods
    //

    virtual cstr_t getDescription() = 0;
    // get supported file extensions. Example: .mp3|MP3 files|.mp2|MP2 files
    virtual cstr_t getFileExtentions() = 0;

    virtual ResultCode open(IMediaInput *input) = 0;

    // Decode to @bufOut
    virtual ResultCode decode(IFBuffer *fbOut) = 0;

    virtual bool isSeekable() = 0;

    // media length, pos related functions, unit: ms
    virtual uint32_t getDuration() = 0;
    virtual ResultCode seek(uint32_t pos) = 0;

};

using IMediaDecoderPtr = std::shared_ptr<IMediaDecoder>;

//
// A plugin DLL may contain serveral plugins, use MusicPlayerQueryPluginIF to query all the plugin interface
// that it supported.
//
// query will start with nIndex = 0, till !ERR_OK returned.
//
// nIndex = 0, return its interface type, and description, and lpInterface.
// strDescription and lpInterface can be nullptr, and only its description or lpInterface is queried.
//
// extern "C" __declspec(dllexport) ResultCode MusicPlayerQueryPluginIF(int index, MPInterfaceType *interfaceType, const char **description, void **interfacePtr);
typedef ResultCode (*PlayerQueryPluginIF_t)(int index, MPInterfaceType *interfaceType, const char **description, void **interfacePtr);

#define SZ_FUNC_ZP_QUERY_PLUGIN_IF  "MusicPlayerQueryPluginIF"

#endif // _IMPLAYER_H_
