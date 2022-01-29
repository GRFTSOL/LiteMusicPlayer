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

class CMDVorbis : public IMediaDecode  
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
	IMPlayer		*m_pPlayer;
	IMemAllocator	*m_pMemAllocator;
	PLAYER_STATE	m_state;
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
