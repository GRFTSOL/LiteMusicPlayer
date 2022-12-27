

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

    virtual MLRESULT init(IMPlayer *pPlayer) { return ERR_OK; }
    virtual MLRESULT quit() { return ERR_OK; }

    virtual MLRESULT open(int nSampleRate, int nNumChannels, int nBitsPerSamp);

    virtual MLRESULT waitForWrite();

    virtual MLRESULT write(IFBuffer *pBuf);
    virtual MLRESULT flush();

    virtual MLRESULT pause(bool bPause);
    virtual bool isPlaying();
    virtual MLRESULT stop();

    virtual bool isOpened();

    // volume
    MLRESULT setVolume(int volume, int nBanlance);

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
