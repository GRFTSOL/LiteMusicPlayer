#pragma once

#include <mutex>
#include <atomic>
#include <AudioToolbox/AudioQueue.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudio.h>
#include "../IMPlayer.h"


class CoreAudioOutput: public IMediaOutput {
public:
    CoreAudioOutput();
    ~CoreAudioOutput();

    cstr_t getDescription() override { return "CoreAudio Output"; }

    ResultCode waitForWrite(int timeOutMs) override;

    ResultCode write(IFBuffer *fb) override;
    ResultCode flush() override;

    ResultCode play() override;
    ResultCode pause() override;
    bool isPlaying() override;
    ResultCode stop() override;

    uint32_t getPos() override;

    ResultCode setVolume(int volume, int banlance) override;
    int getVolume() override;

protected:
    enum State {
        StateStopped,
        StatePaused,
        StatePlaying
    };

    std::recursive_mutex            _mutex;
    AudioStreamBasicDescription     _audioFormat;
    AudioQueueRef                   _audioQueue = nullptr;
    double                          _volume = 0.01;
    std::atomic<int>                _bufferCount = {0};
    State                           _state = StateStopped;;

    int                             _sampleRate = 44100;

    std::mutex                      _mutexWaitWrite;
    std::condition_variable         _cvWaitWrite;

};
