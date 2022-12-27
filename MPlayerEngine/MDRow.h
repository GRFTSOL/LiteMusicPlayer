#pragma once

#ifndef MPlayerEngine_MDRow_h
#define MPlayerEngine_MDRow_h


#include "IMPlayer.h"


class CMDRow : public IMediaDecode {
OBJ_REFERENCE_DECL
public:
    CMDRow();
    virtual ~CMDRow();

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
    static void decodeThread(void *lpParam);

    void decodeThreadProc();

protected:
    IMediaOutput                *m_pOutput;
    IMediaInput                 *m_pInput;
    IMPlayer                    *m_pPlayer;
    IMemAllocator               *m_pMemAllocator;
    PLAYER_STATE                m_state;
    bool                        m_bPaused;
    bool                        m_bKillThread;
    int32_t                     m_nSeekPos;
    bool                        m_bSeekFlag;
    uint32_t                    m_nLengthMs;

    CThread                     m_threadDecode;

};

#endif // !defined(MPlayerEngine_MDRow_h)
