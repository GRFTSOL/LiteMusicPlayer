#ifndef DSP_SUPEREQ_H
#define DSP_SUPEREQ_H
#pragma once

#include "../../Utils/Utils.h"
#include "IMPlayer.h"


class CDspSuperEQ : public IDSP  
{
protected:
	CDspSuperEQ();
	virtual ~CDspSuperEQ();


public:
	static CDspSuperEQ * GetInstance();
	virtual void AddRef() {};
	virtual void Release() {};

	virtual MLRESULT Init(IMPlayer *pPlayer);
	virtual MLRESULT Quit();

	virtual void Process(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

	bool CreateWnd();
	virtual int Render(VisParam *visParam);
	int Render2(VisParam *visParam);
	int Render3(VisParam *visParam);
	int Render4(VisParam *visParam);
	void ThreadVis();

protected:
	int						m_nLastSRate;
	int						m_nLastChannel;
	int						m_nLastBps;
	CMPAutoPtr<IMPlayer>	m_pPlayer;

	HWND			m_hWnd;

};

#endif
