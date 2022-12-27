#pragma once

#ifndef MPlayerEngine_DspDemo_h
#define MPlayerEngine_DspDemo_h


#include "IMPlayer.h"


class CDspDemo : public IDSP {
    OBJ_REFERENCE_DECL

public:
    CDspDemo();
    virtual ~CDspDemo();

    virtual MLRESULT init(IMPlayer *pPlayer);
    virtual MLRESULT quit();

    virtual void process(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);
    virtual void processEcho(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);
    virtual void processPitch(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

    virtual void processFadeInout(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

    virtual void processVoice(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

protected:
    CMPAutoPtr<IMPlayer>        m_pPlayer;
    string                      m_buffPitch;

};

#endif // !defined(MPlayerEngine_DspDemo_h)
