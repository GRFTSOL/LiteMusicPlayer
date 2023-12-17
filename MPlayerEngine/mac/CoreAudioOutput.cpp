#include "CoreAudioOutput.h"
#include <chrono>


const int BUFFER_COUNT = 124;


CoreAudioOutput::CoreAudioOutput() {
    _audioFormat = (AudioStreamBasicDescription) { 0 };
    _audioFormat.mFormatID = kAudioFormatLinearPCM;
    _audioFormat.mFormatFlags = kAudioFormatFlagIsFloat;
    _audioFormat.mFramesPerPacket = 1;
    _audioFormat.mBitsPerChannel = 32;
    _audioFormat.mReserved = 0;

    _audioFormat.mChannelsPerFrame = -1;
    _audioFormat.mSampleRate = -1;
    _audioFormat.mBytesPerFrame = -1;
    _audioFormat.mBytesPerPacket = -1;

    _audioQueue = nullptr;
    _bufferCount = 0;
}

CoreAudioOutput::~CoreAudioOutput() {

}

ResultCode CoreAudioOutput::waitForWrite(int timeOutMs) {
    if (_bufferCount < BUFFER_COUNT) {
        return ERR_OK;
    } else {
        std::unique_lock<std::mutex> lock(_mutexWaitWrite);
        _cvWaitWrite.wait_for(lock, std::chrono::microseconds(timeOutMs));
        if (_bufferCount < BUFFER_COUNT) {
            return ERR_OK;
        } else {
            return ERR_BUFFER_FULL;
        }
    }
}

ResultCode CoreAudioOutput::write(IFBuffer *fb) {
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if (_state != StatePlaying) {
        return ERR_PLAYER_INVALID_STATE;
    }

    if (_bufferCount >= BUFFER_COUNT) {
        return ERR_BUFFER_FULL;
    }

    OSStatus result;

    if (fb->sampleRate() != _audioFormat.mSampleRate ||
        fb->channels() != _audioFormat.mChannelsPerFrame ||
        _audioQueue == nullptr)
    {
        _sampleRate = fb->sampleRate();
        _audioFormat.mSampleRate = fb->sampleRate();
        _audioFormat.mChannelsPerFrame = fb->channels();
        _audioFormat.mBytesPerFrame = (_audioFormat.mBitsPerChannel / 8) * _audioFormat.mChannelsPerFrame;
        _audioFormat.mBytesPerPacket = _audioFormat.mBytesPerFrame * _audioFormat.mFramesPerPacket;

        lock.unlock();
        stop();
        lock.lock();

        /* create the queue */
        result = AudioQueueNewOutput(&_audioFormat, [](void *data, AudioQueueRef queue, AudioQueueBufferRef buffer) {
            auto *thiz = (CoreAudioOutput *)data;
            if (thiz->_bufferCount <= 0) {
                // thiz->_state = StateStopped;
                // AudioQueueStop(thiz->_audioQueue, false);
                // thiz->stop();
            } else {
                thiz->_bufferCount--;
            }
            thiz->_cvWaitWrite.notify_one();
        }, this, nullptr, kCFRunLoopCommonModes, 0, &_audioQueue);

        if (result != 0) {
            // std::cerr << "AudioQueueNewOutput failed: " << result << "\n";
            return ERR_SOUND_DEVICE_WRITE;
        }

        /* after the queue is created, but before it's started, let's make
        sure the correct output device is selected */
        /* auto device = GetDefaultDevice();
        if (device) {
            std::string deviceId = device->Id();
            if (deviceId.c_str()) {
                CFStringRef deviceUid = CFStringCreateWithBytes(
                    kCFAllocatorDefault,
                    (const UInt8*) deviceId.c_str(),
                    deviceId.size(),
                    kCFStringEncodingUTF8,
                    false);

                AudioQueueSetProperty(
                    _audioQueue,
                    kAudioQueueProperty_CurrentDevice,
                    &deviceUid,
                    sizeof(deviceUid));

                CFRelease(deviceUid);
            }
            device->Release();
        }*/

        /* get it running! */
        result = AudioQueueStart(_audioQueue, nullptr);
        if (result != 0) {
            // std::cerr << "AudioQueueStart failed: " << result << "\n";
            return ERR_SOUND_DEVICE_WRITE;
        }

        setVolume(_volume * 100, 0);
        play();
    }

    uint32_t bytes = fb->size();

    using DataType = float;
    auto count = fb->size() / sizeof(int16_t);
    bytes = (uint32_t)sizeof(DataType) * count;

    AudioQueueBufferRef audioQueueBuffer = nullptr;
    result = AudioQueueAllocateBuffer(_audioQueue, bytes, &audioQueueBuffer);
    if (result != 0) {
        // std::cerr << "AudioQueueAllocateBuffer failed: " << result << "\n";
        return ERR_SOUND_DEVICE_WRITE;
    }

    audioQueueBuffer->mUserData = nullptr;
    audioQueueBuffer->mAudioDataByteSize = bytes;

    auto *p = (DataType *)audioQueueBuffer->mAudioData;
    int16_t *src = (int16_t *)fb->data();
    for (int i = 0; i < count; i++) {
        p[i] = src[i] / 32767.0;
    }
    // memset(audioQueueBuffer->mAudioData, 10, bytes);
    // memcpy(audioQueueBuffer->mAudioData, (void *)fb->data(), bytes);

    result = AudioQueueEnqueueBuffer(_audioQueue, audioQueueBuffer, 0, nullptr);
    if (result != 0) {
        // std::cerr << "AudioQueueEnqueueBuffer failed: " << result << "\n";
        return ERR_SOUND_DEVICE_WRITE;
    }

    ++_bufferCount;

    return ERR_OK;
}

ResultCode CoreAudioOutput::flush() {
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if (_state != StateStopped && _audioQueue) {
        AudioQueueStop(_audioQueue, true);
        AudioQueueStart(_audioQueue, nullptr);
        if (_state == StatePaused) {
            AudioQueuePause(_audioQueue);
        }
    }

    return ERR_OK;
}

ResultCode CoreAudioOutput::play() {
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if (_audioQueue) {
        AudioQueueStart(_audioQueue, nullptr);
    }
    _state = StatePlaying;

    return ERR_OK;
}

ResultCode CoreAudioOutput::pause() {
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if (_audioQueue) {
        AudioQueuePause(_audioQueue);
    }
    _state = StatePaused;

    return ERR_OK;
}

bool CoreAudioOutput::isPlaying() {
    if (_bufferCount == 0) {
        stop();
    }
    return _state == StatePlaying;
}

ResultCode CoreAudioOutput::stop() {
    AudioQueueRef queue = nullptr;

    {
        /* AudioQueueStop/AudioQueueDispose will trigger the callback, which
        will try to dispose of the samples on a separate thread and deadlock.
        cache the queue and reset outside of the critical section */
        std::unique_lock<std::recursive_mutex> lock(_mutex);
        queue = _audioQueue;
        _audioQueue = nullptr;
        _state = StateStopped;
        _bufferCount = 0;
    }

    if (queue) {
        AudioQueueStop(queue, true);
        AudioQueueDispose(queue, true);
    }

    return ERR_OK;
}

uint32_t CoreAudioOutput::getPos() {
    AudioTimeStamp timeStamp;
    auto result = AudioQueueGetCurrentTime(_audioQueue, nullptr, &timeStamp, nullptr);
    if (result == 0) {
        return timeStamp.mSampleTime * 1000 / _sampleRate;
    }

    return 0;
}

ResultCode CoreAudioOutput::setVolume(int volume, int banlance) {
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    _volume = volume / 100.0;

    if (_audioQueue) {
        OSStatus result = AudioQueueSetParameter(_audioQueue,
            kAudioQueueParam_Volume, _volume);
        if (result != 0) {
            // std::cerr << "AudioQueueSetParameter(_volume) failed: " << result << "\n";
        }
    }

    return ERR_OK;
}

int CoreAudioOutput::getVolume() {
    return _volume;
}
