// MDLibmad.h: interface for the CMDLibmad class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDLIBMAD_H__27C13E01_9EDD_4EFC_A2D2_7DD21F6F4DF3__INCLUDED_)
#define AFX_MDLIBMAD_H__27C13E01_9EDD_4EFC_A2D2_7DD21F6F4DF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"
#include "../MPlayer-Plugins/libmad-0.15.1b/mad.h"
#include "audio.h"

struct MPEG_AUDIO_INFO
{
    mad_header    header;

    int            nSamplesPerFrame;
    int            nFrameSize;
    int            nFrameCount;
    int            nMediaLength;    // ms

    bool        bVbr;
    int            nBitRate;        // same as header.bitrate if not vbr.

    uint32_t        nMediaFileSize;    // uint8_t
    uint32_t        nFirstFramePos;
};

class CMDLibmad : public IMediaDecode  
{
OBJ_REFERENCE_DECL
public:
    CMDLibmad();
    virtual ~CMDLibmad();

    //
    // individual methods
    //

    virtual LPCXSTR getDescription();
    virtual LPCXSTR getFileExtentions();
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
    virtual MLRESULT setVolume(int nVolume, int nBanlance);

protected:
    MLRESULT getHeadInfo(IMediaInput *pInput);

    static void decodeThread(void *lpParam);
    int mad_decoder_run(struct mad_decoder *decoder);
    int run_sync(struct mad_decoder *decoder);

    uint32_t decodeThreadProc();

    // bool outputWrite(IFBuffer *pBuf, int nChannels, int nSampleRate);

public:
    IMediaOutput    *m_pOutput;
    IMediaInput        *m_pInput;
    IMPlayer        *m_pPlayer;
    IMemAllocator    *m_pMemAllocator;
    PLAYER_STATE    m_state;
    bool            m_bPaused;
    bool            m_bKillThread;
    int32_t            m_nSeekPos;
    uint32_t            m_nWritePosByte;
    bool            m_bSeekFlag;

    string        m_bufInput;
    CThread            m_threadDecode;

    MPEG_AUDIO_INFO    m_audioInfo;

    mad_decoder     m_decoder;
    audio_stats        m_audioStats;

    // Xing or Info Vbri seek table
    bool            m_bXingTable;
    uint32_t            m_nFileSizeOrg;
    int                m_nTableSize;
    int                *m_pnToc;    // 100 percent
    int                m_nFramesPerEntry;

    bool getVbrInfo(mad_header *header, unsigned char const *frameBuff, int nBuffLen);

    uint32_t seekXingTable(uint32_t nSeekToMs);
    uint32_t seekVbriTable(uint32_t nSeekToMs);

};

#endif // !defined(AFX_MDLIBMAD_H__27C13E01_9EDD_4EFC_A2D2_7DD21F6F4DF3__INCLUDED_)
