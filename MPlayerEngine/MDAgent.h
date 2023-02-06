#pragma once

#ifndef MPlayerEngine_MDAgent_h
#define MPlayerEngine_MDAgent_h


#include "IMPlayer.h"
#include "MPTools.h"


class CMPlayer;

class CMDAgent : public IMediaDecoder {
    OBJ_REFERENCE_DECL
public:
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

    //    ERR_PLAYER_INVALID_FILE:    decoder can't decode the file
    //  ERR_DECODER_INNER_ERROR:    inner error occurs at decoder
    //  ERR_DECODER_UNSUPPORTED_FEATURE:
    //  ERR_DECODER_INIT_FAILED:
    virtual ResultCode play(IMPlayer *pPlayer, IMediaInput *pInput);
    virtual ResultCode pause();
    virtual ResultCode unpause();
    virtual ResultCode stop();

    // media length, pos related functions, unit: ms
    virtual uint32_t getLength();
    virtual ResultCode seek(uint32_t nPos);
    virtual uint32_t getPos();

    // volume
    virtual ResultCode setVolume(int volume, int nBanlance);

public:
    CMDAgent();
    virtual ~CMDAgent();

    virtual ResultCode init(CMPlayer *pPlayer);
    virtual ResultCode doDecode(cstr_t mediaUrl);

    virtual void notifyEod(IMediaDecoder *pDecoder, ResultCode nError);

protected:
    string                      m_mediaUrl;
    CMPAutoPtr<IMediaInput>     m_pMediaInput;
    CMPAutoPtr<IMediaDecoder>    m_pMediaDecode;
    CMPAutoPtr<CMPlayer>        m_pPlayer;

};

#endif // !defined(MPlayerEngine_MDAgent_h)
