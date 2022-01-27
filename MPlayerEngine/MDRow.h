// MDRow.h: interface for the CMDRow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDROW_H__71277116_FBC6_4744_99BB_6224B78CF6A3__INCLUDED_)
#define AFX_MDROW_H__71277116_FBC6_4744_99BB_6224B78CF6A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"

class CMDRow : public IMediaDecode
{
OBJ_REFERENCE_DECL
public:
    CMDRow();
    virtual ~CMDRow();

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
    static void decodeThread(void *lpParam);

    void decodeThreadProc();

protected:
    IMediaOutput    *m_pOutput;
    IMediaInput        *m_pInput;
    IMPlayer        *m_pPlayer;
    IMemAllocator    *m_pMemAllocator;
    PLAYER_STATE    m_state;
    bool            m_bPaused;
    bool            m_bKillThread;
    int32_t            m_nSeekPos;
    bool            m_bSeekFlag;
    uint32_t            m_nLengthMs;

    CThread            m_threadDecode;

};

#endif // !defined(AFX_MDROW_H__71277116_FBC6_4744_99BB_6224B78CF6A3__INCLUDED_)
