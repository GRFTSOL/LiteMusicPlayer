#pragma once

#ifndef MPlayerEngine_MDAgent_h
#define MPlayerEngine_MDAgent_h


#include "IMPlayer.h"
#include "MPTools.h"


class CMPlayer;

class CMDAgent : public IMediaDecode {
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
    virtual MLRESULT setVolume(int volume, int nBanlance);

public:
    CMDAgent();
    virtual ~CMDAgent();

    virtual MLRESULT init(CMPlayer *pPlayer);
    virtual MLRESULT doDecode(IMedia *pMedia);

    virtual void notifyEod(IMediaDecode *pDecoder, MLRESULT nError);

protected:
    CMPAutoPtr<IMedia>          m_pMedia;
    CMPAutoPtr<IMediaInput>     m_pMediaInput;
    CMPAutoPtr<IMediaDecode>    m_pMediaDecode;
    CMPAutoPtr<CMPlayer>        m_pPlayer;

};

#endif // !defined(MPlayerEngine_MDAgent_h)
