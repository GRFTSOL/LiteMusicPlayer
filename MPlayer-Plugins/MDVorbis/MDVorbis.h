// MDVorbis.h: interface for the CMDVorbis class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDVORBIS_H__59302819_7454_4FA2_8FC1_AA5878C39F87__INCLUDED_)
#define AFX_MDVORBIS_H__59302819_7454_4FA2_8FC1_AA5878C39F87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../base/base.h"
#include "../../MPlayerEngine/IMPlayer.h"

#include <ivorbisfile.h>

class CMDVorbis : public IMediaDecoder  
{
OBJ_REFERENCE_DECL
public:
	CMDVorbis();
	virtual ~CMDVorbis();

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
		int			nMediaLength;	// ms
		int			nBitRate;		// 
		uint32		nMediaFileSize;	// byte
		int			nBps;
		int			nSampleRate;
		int			nChannels;

		bool		m_bSeekable;
	};

protected:
	static void DecodeThread(LPVOID lpParam);

	void DecodeThreadProc();

	void VorbisDecode();

	// bool OutputWrite(IFBuffer *pBuf);

protected:
	IMediaOutput	*m_pOutput;
	IMediaInput		*m_pInput;
	PlayerState	m_state;
	bool			m_bPaused;
	bool			m_bKillThread;
	int32			m_nSeekPos;
	bool			m_bSeekFlag;

	AUDIO_INFO		m_audioInfo;

	CThread			m_threadDecode;

	OggVorbis_File	m_file;
	ov_callbacks	m_ovcb;

};

#endif // !defined(AFX_MDVORBIS_H__59302819_7454_4FA2_8FC1_AA5878C39F87__INCLUDED_)
