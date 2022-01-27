// MDFlac.h: interface for the CMDFlac class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDFLAC_H__0FE3C45D_D618_4ECA_B832_06A85D44CD37__INCLUDED_)
#define AFX_MDFLAC_H__0FE3C45D_D618_4ECA_B832_06A85D44CD37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../../MPlayerEngine/IMPlayer.h"

struct AUDIO_INFO
{
	int				nMediaLength;	// ms
	int				nBitRate;		// 
	uint32			nMediaFileSize;	// byte
	int				nBps;
	unsigned long	nSampleRate;
	unsigned char	nChannels;

	bool			m_bSeekable;
	uint32			nTotalSamples;
};

class CMDFlac : public IMediaDecode
{
OBJ_REFERENCE_DECL
public:
	CMDFlac();
	virtual ~CMDFlac();

	//
	// individual methods
	//

	virtual LPCXSTR getDescription();
	virtual LPCXSTR getFileExtentions();	// get supported file's extentions.
	virtual MLRESULT getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia);

	//
	// decode media file related methods
	//

	virtual bool isSeekable();
	virtual bool isUseOutputPlug();

	virtual MLRESULT play(IMPlayer *pPlayer, IMediaInput *pInput);
	virtual MLRESULT pause();
	virtual MLRESULT unpause();
	virtual bool IsPaused();
	virtual MLRESULT stop();

	// time length
	virtual uint32 getLength();
	virtual MLRESULT Seek(uint32 dwPos);
	virtual uint32 GetPos();

	// volume
	virtual MLRESULT setVolume(int nVolume, int nBanlance);

	bool OutputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

protected:
	static void DecodeThread(LPVOID lpParam);

	void DecodeThreadProc();
public:
	IMediaOutput	*m_pOutput;
	IMediaInput		*m_pInput;
	IMPlayer		*m_pPlayer;
	IMemAllocator	*m_pMemAllocator;
	PLAYER_STATE	m_state;
	bool			m_bPaused;
	bool			m_bKillThread;
	int32			m_nSeekPos;
	bool			m_bSeekFlag;

	AUDIO_INFO		m_audioInfo;

	CThread			m_threadDecode;

	FLAC__StreamDecoder *m_pFlacDecoder;

	// Buffer for flac decoder
	int				m_nBytesPerSample;

};

#endif // !defined(AFX_MDFLAC_H__0FE3C45D_D618_4ECA_B832_06A85D44CD37__INCLUDED_)
