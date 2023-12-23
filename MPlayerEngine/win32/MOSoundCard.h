#pragma once

#ifndef MPlayerEngine_win32_MOSoundCard_h
#define MPlayerEngine_win32_MOSoundCard_h

#include "../IMPlayer.h"
#include <mmsystem.h>


class MOSoundCard : public IMediaOutput {
OBJ_REFERENCE_DECL
public:
    enum FADE_MODE {
        FADE_NONE,
        FADE_IN,
        FADE_OUT,
    };

    MOSoundCard();
    virtual ~MOSoundCard();

    cstr_t getDescription() override { return "MCI Soundcard Output"; }

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
    ResultCode doOpen(int sampleRate, int numChannels, int bitsPerSamp);
    ResultCode doStop();

    void fadeInSoundData(char *buf, int nLen);
    void fadeOutSoundData(char *buf, int nLen);

public:
    static void CALLBACK mciCallBack(HWAVEOUT hwo, uint32_t msg, DWORD_PTR user, DWORD_PTR param1, DWORD_PTR param2);

private:
    WAVEFORMATEX                    m_wfex;
    HWAVEOUT                        m_hwo = nullptr;

    int                             m_volume = 0;

    int                             m_channels = 0;
    int                             m_sampleRate = 0;
    int                             m_bitsPerSamp = 0;

    int                             m_iBytesPerSample = 0;

    std::mutex                      _mutexWaitWrite;
    std::condition_variable         _cvWaitWrite;

    std::atomic<int>                m_nBuffered = 0;
    int                             m_nBufferedMax = 0;

};

#endif // !defined(MPlayerEngine_win32_MOSoundCard_h)
