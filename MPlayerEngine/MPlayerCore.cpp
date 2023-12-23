#include "MPlayerCore.h"
#include "MediaInputFile.h"
#include "MDMiniMp3.h"
#include "MDFaad.h"
#include "MDFlac.h"
#include "FBuffer.hpp"

#ifdef _WIN32
#include "win32/MOSoundCard.h"
#elif defined(_MAC_OS)
#include "mac/CoreAudioOutput.h"
#else
#endif


MPlayerCore::MPlayerCore() {
    if (!_thread.joinable()) {
        _thread = std::thread(&MPlayerCore::threadRun, this);
    }

    _cv.notify_one();
}

MPlayerCore::~MPlayerCore() {
    quit();
}

void MPlayerCore::quit() {
    _isQuit = true;
    _cv.notify_one();
    if (_thread.joinable()) {
        _thread.join();
    }
}

// 实现的说明
const char *MPlayerCore::getDescription() {
    return "MPlayer Core Implementation";
}

// 支持的文件扩展名
const char *MPlayerCore::getFileExtentions()  {
    return ".mp3|MP3 files|.mp4|MP4 files|.wma|WMA files|.mp2|MP2 files|.m4a|M4A files|.flac|FLAC files|.aac|AAC files";
}

// 获取媒体的标签
bool MPlayerCore::getMediaInfo(const char *mediaUrl, IMediaInfo *pMedia)  {
    // 不获取信息，由外部获取.
    return false;
}

//
// Player control
//
bool MPlayerCore::play(const char *mediaUrl, IMediaInfo *mediaTagsOut)  {
    {
        std::lock_guard<std::mutex> autolock(_mutex);
        _curMediaUrl = mediaUrl;
    }

    _command = CMD_PLAY;
    _cv.notify_one();
    return true;
}

bool MPlayerCore::pause()  {
    _command = CMD_PAUSE;
    _cv.notify_one();
    return true;
}

bool MPlayerCore::unpause()  {
    _command = CMD_UNPAUSE;
    _cv.notify_one();
    return true;
}

bool MPlayerCore::stop()  {
    _command = CMD_STOP;
    _cv.notify_one();
    return true;
}


bool MPlayerCore::isSeekable()  {
    std::lock_guard<std::mutex> autolock(_mutex);
    return _isSeekable;
}

bool MPlayerCore::seek(int pos)  {
    if (_isSeekable && _command == CMD_NONE) {
        _seekPos = pos;
        _command = CMD_SEEK;
    }
    return true;
}

uint32_t MPlayerCore::getPos() {
    std::lock_guard<std::mutex> autolock(_mutex);
    return _seekPos + _output->getPos();
}

//
// Player settings
//
// 0 ~ 100
bool MPlayerCore::setVolume(int volume) {
    std::lock_guard<std::mutex> autolock(_mutex);
    return _output->setVolume(volume, 0) == ERR_OK;
}

int MPlayerCore::getVolume() {
    std::lock_guard<std::mutex> autolock(_mutex);
    return _output->getVolume();
}

// -100 ~ 100
bool MPlayerCore::setBalance(int balance) {
    return false;
}

int MPlayerCore::getBalance() {
    return 0;
}

bool MPlayerCore::setEQ(const EQualizer *eq) {
    return false;
}

bool MPlayerCore::getEQ(EQualizer *eq) {
    return false;
}

/**
 * To simplify the implementation of decoder/output, MPlayerCore::threadRun() is responsible for
 * the state management of these componenets, receive the commands from user, process the commands.
 *
 * So, the whole MPlayerCore including the decoder/output should have one thread only.
 */
void MPlayerCore::threadRun() {
    _output = newMediaOutput();

    while (!_isQuit) {
        // 停止状态，等待接受播放歌曲
        if (_command != CMD_PLAY) {
            notifyEndOfPlaying();
        }

        auto cmd = waitForCommand();
        if (cmd == CMD_QUIT) {
            return;
        }
        assert(cmd == CMD_PLAY || _isQuit);
        if (cmd != CMD_PLAY) {
            continue;
        }

        // 打开新的歌曲
        auto input = newMediaInput(_curMediaUrl.c_str());
        if (input->open(_curMediaUrl.c_str()) != ERR_OK) {
            notifyEndOfPlaying();
            continue;
        }

        auto decoder = newMediaDecoder(input.get());
        int ret = decoder->open(input.get());
        if (ret != ERR_OK) {
            DBG_LOG1("Failed to play media: %s", _curMediaUrl.c_str());
            Sleep(300);
            notifyEndOfPlaying();
            continue;
        }

        auto _stop = [decoder, input, this]() {
            _state = PS_STOPPED;
            _output->stop();
        };

        _isSeekable = decoder->isSeekable();

        // 开始播放循环
        _mediaDuration = decoder->getDuration();
        _seekPos = 0;
        _state = PS_PLAYING;
        _output->stop();
        _output->play();
        while (_state != PS_STOPPED && !_isQuit) {
            while (_state == PS_PAUSED || (_command != CMD_NONE && _state != PS_STOPPED)) {
                // 暂停状态, 等待其他命令
                auto cmd = waitForCommand();
                switch (cmd) {
                    case CMD_NONE:
                        // 未收到命令被唤醒了？
                        assert(0);
                        break;
                    case CMD_SET_OUTPUT:
                        _output = newMediaOutput();
                        break;
                    case CMD_PLAY:
                        _command = cmd;
                        // 退出播放循环
                        _stop();
                        break;
                    case CMD_UNPAUSE:
                        // 继续播放
                        if (_state == PS_PAUSED) {
                            _output->play();
                        }
                        _state = PS_PLAYING;
                        break;
                    case CMD_PAUSE:
                        // 暂停
                        _state = PS_PAUSED;
                        _output->pause();
                        break;
                    case CMD_STOP:
                        _stop();
                        break;
                    case CMD_SEEK:
                        assert(decoder->isSeekable());
                        decoder->seek(_seekPos);
                        _output->flush();
                        break;
                    case CMD_QUIT:
                        return;
                }
            }

            int emptyBufCount = 0;
            while (_state == PS_PLAYING && !_isQuit) {
                // 等待 100ms 超时，检查是否有新的命令
                bool canWrite = _output->waitForWrite(100) == ERR_OK;
                if (_command != CMD_NONE) {
                    // 接收到新的命令，在外层循环中执行
                    break;
                }

                if (canWrite) {
                    auto buf = std::make_shared<FBuffer>();
                    if (decoder->decode(buf.get()) == ERR_OK) {
                        if (buf->size() > 0) {
                            _output->write(buf.get());
                            emptyBufCount = 0;
                            continue;
                        } else if (emptyBufCount < 20) {
                            emptyBufCount++;
                            continue;
                        }
                    }

                    // 遇到错误，或者解码结束
                    Sleep(50); // Wait 50ms to check playing status again.
                    if (!_output->isPlaying()) {
                        // 播放结束
                        notifyEndOfPlaying();
                        break;
                    }
                }
            }
        }
    }
}

MPlayerCore::Command MPlayerCore::waitForCommand() {
    if (_command == CMD_NONE) {
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait(lock);
    }

    auto cmd = _command;
    _command = CMD_NONE;
    return cmd;
}

IMediaDecoderPtr MPlayerCore::newMediaDecoder(IMediaInput *input) {
    StringView ext(fileGetExt(input->getSource()));
    if (ext.iEqual(".mp3")) {
        return std::make_shared<MDMiniMp3>();
    } else if (ext.iEqual(".mp4") || ext.iEqual(".m4a") || ext.iEqual(".aac")) {
        return std::make_shared<MDFaad>();
    } else if (ext.iEqual(".flac")) {
        return std::make_shared<MDFlac>();
    } else {
        return std::make_shared<MDMiniMp3>();
    }
}

IMediaInputPtr MPlayerCore::newMediaInput(cstr_t mediaUrl) {
    auto input = std::make_shared<MediaInputFile>();
    return input;
}

IMediaOutputPtr MPlayerCore::newMediaOutput() {
#ifdef _WIN32
    return std::make_shared<MOSoundCard>();
#elif defined(_MAC_OS)
    return std::make_shared<CoreAudioOutput>();
#else
#endif
}

void MPlayerCore::notifyEndOfPlaying() {
    _state = PS_STOPPED;

    if (m_callback) {
        m_callback->onEndOfPlaying();
    }
}
