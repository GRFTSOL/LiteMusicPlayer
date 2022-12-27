#pragma once

﻿

#ifndef MPlayerEngine_MORaw_h
#define MPlayerEngine_MORaw_h


#include "IMPlayer.h"


class CMORaw : public IMediaOutput {
OBJ_REFERENCE_DECL
public:
    CMORaw();
    virtual ~CMORaw();

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

protected:
    uint32_t                    m_dwTotolBytesOffset; // 输入到MOSoundCard中的字节偏移总量
    int                         m_nBps, m_nSampleRate, m_nChannels;
    int                         m_nBytesPerSample;
    FILE                        *m_fp;
    bool                        m_bPaused;
    Event                       m_eventCanWrite;

};

#endif // !defined(MPlayerEngine_MORaw_h)
