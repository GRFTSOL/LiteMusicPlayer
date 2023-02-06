

#ifndef MPlayerEngine_linux_MOSoundCard_h
#define MPlayerEngine_linux_MOSoundCard_h

#pragma once

class CMOSoundCard : public IMediaOutput {
OBJ_REFERENCE_DECL
public:
    enum FADE_MODE {
        FADE_NONE,
        FADE_IN,
        FADE_OUT,
    };

    CMOSoundCard();
    virtual ~CMOSoundCard();

    cstr_t getDescription();

    virtual ResultCode init(IMPlayer *pPlayer) { return ERR_OK; }
    virtual ResultCode quit() { return ERR_OK; }

    virtual ResultCode open(int nSampleRate, int nNumChannels, int nBitsPerSamp);

    virtual ResultCode waitForWrite();

    virtual ResultCode write(IFBuffer *pBuf);
    virtual ResultCode flush();

    virtual ResultCode pause(bool bPause);
    virtual bool isPlaying();
    virtual ResultCode stop();

    virtual bool isOpened();

    // volume
    ResultCode setVolume(int volume, int nBanlance);

    virtual uint32_t getPos();

private:
    Event                       m_eventCanWrite;
    uint32_t                    m_dwTotolBytesOffset; // 输入到MOSoundCard中的字节偏移总量

    int                         m_iBytesPerSample;
    uint32_t                    m_dwDataSize;

    std::mutex                  m_mutex;
    int                         m_nBuffered;
    int                         m_nBufferedMax;

    FADE_MODE                   m_fadeMode;
    long                        m_nFadeDoneSamples;
    long                        m_nFadeMaxSamples;

    bool                        m_bWrittingPausedData;

};

#endif // !defined(MPlayerEngine_linux_MOSoundCard_h)
