// DspDemo.h: interface for the CDspDemo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSPDEMO_H__39A6AE2E_E737_4081_8959_E85041B73335__INCLUDED_)
#define AFX_DSPDEMO_H__39A6AE2E_E737_4081_8959_E85041B73335__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"

class CDspDemo : public IDSP  
{
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
    CMPAutoPtr<IMPlayer>    m_pPlayer;
    string                m_buffPitch;

};

#endif // !defined(AFX_DSPDEMO_H__39A6AE2E_E737_4081_8959_E85041B73335__INCLUDED_)
