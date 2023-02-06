#pragma once

#ifndef MPlayerEngine_MDWave_h
#define MPlayerEngine_MDWave_h


#include "IMPlayer.h"


class CMDWave : public IMediaDecoder {
OBJ_REFERENCE_DECL
public:
    CMDWave();
    virtual ~CMDWave();

    //
    // individual methods
    //

    virtual cstr_t getDescription();
    virtual cstr_t getFileExtentions();
    virtual ResultCode getMediaInfo(IMediaInput *pInput, IMediaInfo *pMedia);

    //
    // decode media file related methods
    //

    virtual bool isSeekable();
    virtual bool isUseOutputPlug();

    virtual ResultCode play(IMPlayer *pPlayer, IMediaInput *pInput);
    virtual ResultCode pause();
    virtual ResultCode unpause();
    virtual ResultCode stop();

    // time length
    virtual uint32_t getLength();
    virtual ResultCode seek(uint32_t dwPos);
    virtual uint32_t getPos();

    // volume
    virtual ResultCode setVolume(int volume, int nBanlance);

protected:
    struct WaveFileInfo {
        short                       format_tag, channels, block_align, bits_per_sample;
        long                        samples_per_sec, avg_bytes_per_sec;
        unsigned long               position, length;
        uint32_t                    nMediaFileSize;
        int                         nMediaLength;       // ms
        long                        data_offset;
    };

    ResultCode getHeadInfo(IMediaInput *pInput);

    static void decodeThread(void *lpParam);

    void decodeThreadProc();

public:
    IMediaOutput                *m_pOutput;
    IMediaInput                 *m_pInput;
    PlayerState                 m_state;
    bool                        m_bPaused;
    bool                        m_bKillThread;
    int32_t                     m_nSeekPos;
    bool                        m_bSeekFlag;

    WaveFileInfo                m_audioInfo;

    string                      m_bufInput;
    CThread                     m_threadDecode;

};

#endif // !defined(MPlayerEngine_MDWave_h)
