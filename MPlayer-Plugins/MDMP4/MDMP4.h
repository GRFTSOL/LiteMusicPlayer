// MDMP4.h: interface for the CMDMP4 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDMP4_H__1B71B4A4_E1A4_42A8_8A4E_AEFE1D6EAE1A__INCLUDED_)
#define AFX_MDMP4_H__1B71B4A4_E1A4_42A8_8A4E_AEFE1D6EAE1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMDMP4 : public IMediaDecoder  
{
OBJ_REFERENCE_DECL
public:
	CMDMP4();
	virtual ~CMDMP4();

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

	struct AUDIO_INFO
	{
		int				nMediaLength;	// ms
		int				nBitRate;		// 
		uint32			nMediaFileSize;	// byte
		int				Bps;
		unsigned long	nSampleRate;
		unsigned char	nChannels;

		bool			m_bSeekable;
	};

protected:
	static void DecodeThread(LPVOID lpParam);

	void DecodeThreadProc();

public:
	IMediaOutput	*m_pOutput;
	IMediaInput		*m_pInput;
	PlayerState	m_state;
	bool			m_bPaused;
	bool			m_bKillThread;
	int32			m_nSeekPos;
	bool			m_bSeekFlag;

	AUDIO_INFO		m_audioInfo;

	CThread			m_threadDecode;

protected:
	ResultCode PreDecode();
	ResultCode AACPlayThreadFun();
	ResultCode MP4PlayThreadFun();
	bool			m_bIsSeekable;

};

#endif // !defined(AFX_MDMP4_H__1B71B4A4_E1A4_42A8_8A4E_AEFE1D6EAE1A__INCLUDED_)
