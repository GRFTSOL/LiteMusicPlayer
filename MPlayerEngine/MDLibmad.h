#pragma once

#ifndef MPlayerEngine_MDLibmad_h
#define MPlayerEngine_MDLibmad_h


#include "IMPlayer.h"
#include "../MPlayer-Plugins/libmad-0.15.1b/mad.h"
#include "audio.h"


struct MPEG_AUDIO_INFO {
    mad_header                  header;

    int                         nSamplesPerFrame;
    int                         nFrameSize;
    int                         nFrameCount;
    int                         nMediaLength;       // ms

    bool                        bVbr;
    int                         nBitRate;           // same as header.bitrate if not vbr.

    uint32_t                    nMediaFileSize;     // uint8_t
    uint32_t                    nFirstFramePos;
};

class CMDLibmad : public IMediaDecoder {
OBJ_REFERENCE_DECL
public:
    CMDLibmad();
    virtual ~CMDLibmad();

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
    ResultCode getHeadInfo(IMediaInput *pInput);

    static void decodeThread(void *lpParam);
    int mad_decoder_run(struct mad_decoder *decoder);
    int run_sync(struct mad_decoder *decoder);

    uint32_t decodeThreadProc();

    // bool outputWrite(IFBuffer *pBuf, int nChannels, int nSampleRate);

public:
    IMediaOutput                *m_pOutput;
    IMediaInput                 *m_pInput;
    PlayerState                 m_state;
    bool                        m_bPaused;
    bool                        m_bKillThread;
    int32_t                     m_nSeekPos;
    uint32_t                    m_nWritePosByte;
    bool                        m_bSeekFlag;

    string                      m_bufInput;
    CThread                     m_threadDecode;

    MPEG_AUDIO_INFO             m_audioInfo;

    mad_decoder                 m_decoder;
    audio_stats                 m_audioStats;

    // Xing or Info Vbri seek table
    bool                        m_bXingTable;
    uint32_t                    m_nFileSizeOrg;
    int                         m_nTableSize;
    int                         *m_pnToc;           // 100 percent
    int                         m_nFramesPerEntry;

    bool getVbrInfo(mad_header *header, unsigned char const *frameBuff, int nBuffLen);

    uint32_t seekXingTable(uint32_t nSeekToMs);
    uint32_t seekVbriTable(uint32_t nSeekToMs);

};

#endif // !defined(MPlayerEngine_MDLibmad_h)
