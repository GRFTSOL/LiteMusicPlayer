#pragma once

#ifndef MPlayerEngine_MDRow_h
#define MPlayerEngine_MDRow_h


#include "IMPlayer.h"


class CMDRow : public IMediaDecoder {
OBJ_REFERENCE_DECL
public:
    CMDRow();
    virtual ~CMDRow();

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
    static void decodeThread(void *lpParam);

    void decodeThreadProc();

protected:
    IMediaOutput                *m_pOutput;
    IMediaInput                 *m_pInput;
    PlayerState                 m_state;
    bool                        m_bPaused;
    bool                        m_bKillThread;
    int32_t                     m_nSeekPos;
    bool                        m_bSeekFlag;
    uint32_t                    m_nLengthMs;

    CThread                     m_threadDecode;

};

#endif // !defined(MPlayerEngine_MDRow_h)
