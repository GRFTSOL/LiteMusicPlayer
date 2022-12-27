#pragma once

#ifndef MPlayerEngine_MDWave_h
#define MPlayerEngine_MDWave_h


#include "IMPlayer.h"


class CMDWave : public IMediaDecode {
OBJ_REFERENCE_DECL
public:
    CMDWave();
    virtual ~CMDWave();

    //
    // individual methods
    //

    virtual cstr_t getDescription();
    virtual cstr_t getFileExtentions();
    virtual MLRESULT getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia);

    //
    // decode media file related methods
    //

    virtual bool isSeekable();
    virtual bool isUseOutputPlug();

    virtual MLRESULT play(IMPlayer *pPlayer, IMediaInput *pInput);
    virtual MLRESULT pause();
    virtual MLRESULT unpause();
    virtual MLRESULT stop();

    // time length
    virtual uint32_t getLength();
    virtual MLRESULT seek(uint32_t dwPos);
    virtual uint32_t getPos();

    // volume
    virtual MLRESULT setVolume(int volume, int nBanlance);

protected:
    struct WaveFileInfo {
        short                       format_tag, channels, block_align, bits_per_sample;
        long                        samples_per_sec, avg_bytes_per_sec;
        unsigned long               position, length;
        uint32_t                    nMediaFileSize;
        int                         nMediaLength;       // ms
        long                        data_offset;
    };

    MLRESULT getHeadInfo(IMediaInput *pInput);

    static void decodeThread(void *lpParam);

    void decodeThreadProc();

public:
    IMediaOutput                *m_pOutput;
    IMediaInput                 *m_pInput;
    IMPlayer                    *m_pPlayer;
    IMemAllocator               *m_pMemAllocator;
    PLAYER_STATE                m_state;
    bool                        m_bPaused;
    bool                        m_bKillThread;
    int32_t                     m_nSeekPos;
    bool                        m_bSeekFlag;

    WaveFileInfo                m_audioInfo;

    string                      m_bufInput;
    CThread                     m_threadDecode;

};

#endif // !defined(MPlayerEngine_MDWave_h)
