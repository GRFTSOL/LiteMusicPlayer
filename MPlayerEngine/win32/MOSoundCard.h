#pragma once

﻿

#ifndef MPlayerEngine_win32_MOSoundCard_h
#define MPlayerEngine_win32_MOSoundCard_h


#include <mmsystem.h>


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

protected:
    ResultCode doOpen();
    ResultCode doStop();

    void fadeInSoundData(char *buf, int nLen);
    void fadeOutSoundData(char *buf, int nLen);

    WAVEHDR* NewWaveHdr();

public:
    static void CALLBACK MCICallBack(HWAVEOUT hwo, uint32_t msg, uint32_t dwInstance,
        uint32_t dwParam1, uint32_t dwParam2);

private:
    Event                       m_eventCanWrite;
    uint32_t                    m_dwTotolBytesOffset; // 输入到MOSoundCard中的字节偏移总量

    WAVEFORMATEX                m_wfex;
    HWAVEOUT                    m_hwo;

    int                         m_iBytesPerSample;
    uint32_t                    m_dwDataSize;

    typedef    list<WAVEHDR *>        LIST_WAVHDR;

    std::mutex                  m_mutex;
    list<WAVEHDR                *>    m_listFree, m_listPrepared, m_listPausedHdr;
    int                         m_nBuffered;
    int                         m_nBufferedMax;

    FADE_MODE                   m_fadeMode;
    long                        m_nFadeDoneSamples;
    long                        m_nFadeMaxSamples;

    bool                        m_bWrittingPausedData;

    WAVEHDR                     *m_pFadeOutEndHdr;

};

#endif // !defined(MPlayerEngine_win32_MOSoundCard_h)
