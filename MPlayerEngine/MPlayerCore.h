#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include "IPlayerCore.hpp"
#include "IMPlayer.h"


class MPlayerCore: public IPlayerCore {
public:
    MPlayerCore();
    ~MPlayerCore();

    void quit() override;

    // 实现的说明
    const char *getDescription() override;

    // 支持的文件扩展名
    const char *getFileExtentions() override;

    // 获取媒体的标签
    bool getMediaInfo(const char *mediaUrl, IMediaInfo *pMedia) override;

    //
    // Player control
    //
    bool play(const char *mediaUrl, IMediaInfo *mediaTagsOut) override;
    bool pause() override;
    bool unpause() override;
    bool stop() override;

    bool isSeekable() override;
    bool seek(int pos) override;

    //
    // Current playing Media state
    //
    uint32_t getDuration() override { return _mediaDuration; }
    uint32_t getPos() override;
    PlayerState getState() override { return _state; }

    //
    // Player settings
    //
    // 0 ~ 100
    bool setVolume(int volume) override;
    int getVolume() override;

    // -100 ~ 100
    bool setBalance(int balance) override;
    int getBalance() override;

    bool setEQ(const EQualizer *eq) override;
    bool getEQ(EQualizer *eq) override;

    void notifyEndOfPlaying();

protected:
    enum Command {
        CMD_NONE,
        CMD_SET_OUTPUT,
        CMD_PLAY,
        CMD_UNPAUSE,
        CMD_PAUSE,
        CMD_STOP,
        CMD_SEEK,
        CMD_QUIT,
    };

    void threadRun();

    Command waitForCommand();

    IMediaDecoderPtr newMediaDecoder(IMediaInput *input);
    IMediaInputPtr newMediaInput(cstr_t mediaUrl);
    IMediaOutputPtr newMediaOutput();

    PlayerState                 _state = PS_STOPPED;

    std::mutex                  _mutex;
    std::condition_variable     _cv;
    std::thread                 _thread;
    volatile bool               _isQuit = false;

    Command                     _command = CMD_NONE;
    string                      _curMediaUrl;
    int                         _seekPos = 0;
    int                         _mediaDuration = 0;
    bool                        _isSeekable = false;

    IMediaOutputPtr             _output = nullptr;

};
