// MDAgent.h: interface for the CMDAgent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDAGENT_H__F55CB451_C05E_470A_BCB1_5EB56764D42E__INCLUDED_)
#define AFX_MDAGENT_H__F55CB451_C05E_470A_BCB1_5EB56764D42E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"
#include "MPTools.h"

class CMPlayer;

class CMDAgent : public IMediaDecode
{
    OBJ_REFERENCE_DECL
public:
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

    //    ERR_PLAYER_INVALID_FILE:    decoder can't decode the file
    //  ERR_DECODER_INNER_ERROR:    inner error occurs at decoder
    //  ERR_DECODER_UNSUPPORTED_FEATURE:
    //  ERR_DECODER_INIT_FAILED:
    virtual MLRESULT play(IMPlayer *pPlayer, IMediaInput *pInput);
    virtual MLRESULT pause();
    virtual MLRESULT unpause();
    virtual MLRESULT stop();

    // media length, pos related functions, unit: ms
    virtual uint32_t getLength();
    virtual MLRESULT seek(uint32_t nPos);
    virtual uint32_t getPos();

    // volume
    virtual MLRESULT setVolume(int nVolume, int nBanlance);

public:
    CMDAgent();
    virtual ~CMDAgent();

    virtual MLRESULT init(CMPlayer *pPlayer);
    virtual MLRESULT doDecode(IMedia *pMedia);

    virtual void notifyEod(IMediaDecode *pDecoder, MLRESULT nError);

protected:
    CMPAutoPtr<IMedia>                m_pMedia;
    CMPAutoPtr<IMediaInput>            m_pMediaInput;
    CMPAutoPtr<IMediaDecode>        m_pMediaDecode;
    CMPAutoPtr<CMPlayer>            m_pPlayer;

};

#endif // !defined(AFX_MDAGENT_H__F55CB451_C05E_470A_BCB1_5EB56764D42E__INCLUDED_)
