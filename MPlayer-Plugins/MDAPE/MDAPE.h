// MDAPE.h: interface for the CMDAPE class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDAPE_H__7EF5925F_70AF_47DB_9E69_2C6478FA8770__INCLUDED_)
#define AFX_MDAPE_H__7EF5925F_70AF_47DB_9E69_2C6478FA8770__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../base/base.h"
#include "../../MPlayerEngine/IMPlayer.h"

class CMDAPE : public IMediaDecoder  
{
OBJ_REFERENCE_DECL
public:
	CMDAPE();
	virtual ~CMDAPE();

	//
	// individual methods
	//

	virtual cstr_t getDescription();
	virtual cstr_t getFileExtentions();	// get supported file's extentions.
	virtual ResultCode getMediaInfo(IMediaInput *pInput, IMediaInfo *pMedia);

	//
	// decode media file related methods
	//

	virtual bool isSeekable();
	virtual bool isUseOutputPlug();

	virtual ResultCode play(IMPlayer *pPlayer, IMediaInput *pInput);
	virtual ResultCode pause();
	virtual ResultCode unpause();
	virtual bool IsPaused();
	virtual ResultCode stop();

	// time length
	virtual uint32 getLength();
	virtual ResultCode Seek(uint32 dwPos);
	virtual uint32 GetPos();

	// volume
	virtual ResultCode setVolume(int volume, int nBanlance);

protected:
	struct AUDIO_INFO
	{
		int			nMediaLength;	// ms
		int			nBitRate;		// 
		uint32		nMediaFileSize;	// byte
		int			nBps;
		int			nSampleRate;
		int			nChannels;
	};

	static void DecodeThread(LPVOID lpParam);

	void DecodeThreadProc();
	bool OutputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

public:
	IMediaInput		*m_pInput;
	PlayerState	m_state;
	bool			m_bPaused;
	bool			m_bKillThread;
	int32			m_nSeekPos;
	bool			m_bSeekFlag;

	AUDIO_INFO		m_audioInfo;

	CThread			m_threadDecode;

	LPVOID			m_lpAPEDecoder;

};

#endif // !defined(AFX_MDAPE_H__7EF5925F_70AF_47DB_9E69_2C6478FA8770__INCLUDED_)
