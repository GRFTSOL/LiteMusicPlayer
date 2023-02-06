// MDVorbis.cpp: implementation of the CMDVorbis class.
//
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "MDVorbis.h"

#define SZ_DECODER_DESC		_T("Ogg File decoder")

ogg_int16_t convbuffer[4096]; /* take 8k out of the data segment, not the stack */
int convsize=4096;

size_t mdov_read(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	IMediaInput	*pInput = (IMediaInput*)datasource;
	return pInput->Read(ptr, size * nmemb);
}

int    mdov_seek(void *datasource, ogg_int64_t offset, int whence)
{
	IMediaInput	*pInput = (IMediaInput*)datasource;
	if (pInput->Seek(offset, whence) == ERR_OK)
		return 0;
	else
		return -1;
}

int    mdov_close(void *datasource)
{
	// don't close file.
	// IMediaInput	*pInput = (IMediaInput*)datasource;
	// pInput->Close();
	return 0;
}

long   mdov_tell(void *datasource)
{
	IMediaInput	*pInput = (IMediaInput*)datasource;
	return pInput->GetPos();
}

bool GetOvfInfo(OggVorbis_File &file, CMDVorbis::AUDIO_INFO &m_audioInfo)
{
	vorbis_info* pvi = ov_info(&file, -1);
	if (pvi)
	{
		m_audioInfo.nBitRate = pvi->bitrate_nominal;
		m_audioInfo.nBps = 16;
		m_audioInfo.nChannels = pvi->channels;
		double	time;
		time = ov_pcm_total(&file, -1) * 1000 / pvi->rate;
		m_audioInfo.nMediaLength = int(time);
		m_audioInfo.nSampleRate = pvi->rate;
		m_audioInfo.m_bSeekable = Tobool(ov_seekable(&file));

		return true;
	}
	else
		return false;
}

#define OV_COMMENT_TITLE		"TITLE"
#define OV_COMMENT_ARTIST		"ARTIST"
#define OV_COMMENT_ALBUM		"ALBUM"
#define OV_COMMENT_GENRE		"GENRE"
#define OV_COMMENT_COMMENT		"COMMENT"
#define OV_COMMENT_DATE			"DATE"
#define OV_COMMENT_TRACKNUM		"TRACKNUMBER"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMDVorbis::CMDVorbis()
{
	OBJ_REFERENCE_INIT;

	m_pInput = NULL;
	m_pPlayer = NULL;
	m_pOutput = NULL;
	m_bPaused = false;
	m_bKillThread = false;
	m_nSeekPos = 0;
	m_bSeekFlag = false;
	m_state = PS_STOPPED;
}

CMDVorbis::~CMDVorbis()
{
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
}

OBJ_REFERENCE_IMP(CMDVorbis)

cstr_t CMDVorbis::getDescription()
{
	return SZ_DECODER_DESC;
}

// get supported file's extentions.
cstr_t CMDVorbis::getFileExtentions()
{
	return _T(".ogg\0ogg files\0");
}

ResultCode CMDVorbis::getMediaInfo(IMediaInput *pInput, IMediaInfo *pMedia)
{
	OggVorbis_File	file;
	ov_callbacks	ovcb;

	ovcb.read_func = mdov_read;
	ovcb.close_func = mdov_close;
	ovcb.seek_func = mdov_seek;
	ovcb.tell_func = mdov_tell;

	pInput->Seek(0, SEEK_SET);
	pInput->GetSize(m_audioInfo.nMediaFileSize);

	memset(&m_audioInfo, 0, sizeof(m_audioInfo));

	if (ov_open_callbacks(pInput, &file, NULL, 0, ovcb) < 0)
		return ERR_FALSE;

	if (!GetOvfInfo(file, m_audioInfo))
	{
		ov_clear(&file);

		return ERR_FALSE;
	}

	vorbis_comment* p = ov_comment(&file, -1);
	tstringex	strValue;
	for (int i = 0; i < p->comments; i++)
	{
		char* s = strchr(p->user_comments[i], '=');
		if (s)
		{
#ifdef _UNICODE
			Utf8ToUCS2(s + 1, -1, strValue);
#else
			Utf8ToMbcs(s + 1, -1, strValue);
#endif	

			if (_strnicmp(p->user_comments[i], OV_COMMENT_DATE, strlen(OV_COMMENT_DATE)) == 0)
 				pMedia->setAttribute(MA_YEAR, strValue.c_str());
			else if (_strnicmp(p->user_comments[i], OV_COMMENT_TRACKNUM, strlen(OV_COMMENT_TRACKNUM)) == 0)
 				pMedia->setAttribute(MA_TRACK_NUMB, strValue.c_str());
			else if (_strnicmp(p->user_comments[i], OV_COMMENT_TITLE, strlen(OV_COMMENT_TITLE)) == 0)
 				pMedia->setAttribute(MA_TITLE, strValue.c_str());
			else if (_strnicmp(p->user_comments[i], OV_COMMENT_ARTIST, strlen(OV_COMMENT_ARTIST)) == 0)
 				pMedia->setAttribute(MA_ARTIST, strValue.c_str());
			else if (_strnicmp(p->user_comments[i], OV_COMMENT_ALBUM, strlen(OV_COMMENT_ALBUM)) == 0)
 				pMedia->setAttribute(MA_ALBUM, strValue.c_str());
			else if (_strnicmp(p->user_comments[i], OV_COMMENT_GENRE, strlen(OV_COMMENT_GENRE)) == 0)
 				pMedia->setAttribute(MA_GENRE, strValue.c_str());
			else if (_strnicmp(p->user_comments[i], OV_COMMENT_COMMENT, strlen(OV_COMMENT_COMMENT)) == 0)
 				pMedia->setAttribute(MA_COMMENT, strValue.c_str());
		}
	}

 	pMedia->setAttribute(MA_FILESIZE, m_audioInfo.nMediaFileSize);
 	pMedia->setAttribute(MA_DURATION, m_audioInfo.nMediaLength);
 	pMedia->setAttribute(MA_SAMPLE_RATE, m_audioInfo.nSampleRate);
 	pMedia->setAttribute(MA_CHANNELS, m_audioInfo.nChannels);
 	pMedia->setAttribute(MA_BPS, m_audioInfo.nBps);

	ov_clear(&file);

	return ERR_OK;
}

bool CMDVorbis::isSeekable()
{
	return m_audioInfo.m_bSeekable;
}

bool CMDVorbis::isUseOutputPlug()
{
	return true;
}

ResultCode CMDVorbis::play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	ResultCode		nRet;

	m_state = PS_STOPPED;
	m_bPaused = false;
	m_nSeekPos = 0;
	m_bSeekFlag = false;

	m_pPlayer = pPlayer;
	m_pPlayer->AddRef();

	m_pInput = pInput;
	m_pInput->AddRef();

	nRet = m_pPlayer->QueryInterface(MPIT_OUTPUT, (LPVOID *)&m_pOutput);
	if (nRet != ERR_OK)
		goto R_FAILED;

	nRet = m_pPlayer->QueryInterface(MPIT_MEMALLOCATOR, (LPVOID *)&m_pMemAllocator);
	if (nRet != ERR_OK)
		goto R_FAILED;

	m_pInput->Seek(0, SEEK_SET);

	m_ovcb.read_func = mdov_read;
	m_ovcb.close_func = mdov_close;
	m_ovcb.seek_func = mdov_seek;
	m_ovcb.tell_func = mdov_tell;

	if (ov_open_callbacks(m_pInput, &m_file, NULL, 0, m_ovcb) < 0)
	{
		nRet = ERR_PLAYER_INVALID_FILE;
		goto R_FAILED;
	}

	if (!GetOvfInfo(m_file, m_audioInfo))
	{
		ov_clear(&m_file);
		nRet = ERR_PLAYER_INVALID_FILE;
		goto R_FAILED;
	}

	nRet = m_pOutput->Open(m_audioInfo.nSampleRate, m_audioInfo.nChannels, m_audioInfo.nBps);
	if (nRet != ERR_OK)
	{
		ov_clear(&m_file);
		goto R_FAILED;
	}

	m_state = PS_PLAYING;
	m_bKillThread = false;

	m_threadDecode.Create(DecodeThread, this);
	m_threadDecode.SetPriority(THREAD_PRIORITY_HIGHEST);

	return ERR_OK;

R_FAILED:
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
	if (m_pInput)
	{
		m_pInput->Release();
		m_pInput = NULL;
	}
	if (m_pOutput)
	{
		m_pOutput->Release();
		m_pOutput = NULL;
	}
	if (m_pMemAllocator)
	{
		m_pMemAllocator->Release();
		m_pMemAllocator = NULL;
	}
	return nRet;
}

ResultCode CMDVorbis::pause()
{
	if (m_state != PS_PLAYING)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PAUSED;
	ASSERT(m_pOutput);
	return m_pOutput->pause(true);
}

ResultCode CMDVorbis::unpause()
{
	if (m_state != PS_PAUSED)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PLAYING;
	ASSERT(m_pOutput);
	return m_pOutput->pause(false);
}

bool CMDVorbis::IsPaused()
{
	return m_state == PS_PAUSED;
}

ResultCode CMDVorbis::stop()
{
	m_bKillThread = true;

	if (m_state == PS_PAUSED)
	{
		ASSERT(m_pOutput);
		m_pOutput->Flush();
	}

	m_threadDecode.Join();

	return ERR_OK;
}

uint32 CMDVorbis::getLength()
{
	return m_audioInfo.nMediaLength;
}

ResultCode CMDVorbis::Seek(uint32 dwPos)
{
	m_bSeekFlag = true;
	m_nSeekPos = dwPos;

	ASSERT(m_pOutput);
	if (m_pOutput && m_state == PS_PLAYING)
		m_pOutput->Flush();

	return ERR_OK;
}

uint32 CMDVorbis::GetPos()
{
	if (m_pOutput && !m_bSeekFlag)
		return m_nSeekPos + m_pOutput->GetPos();
	else
		return m_nSeekPos;
}

ResultCode CMDVorbis::setVolume(int volume, int nBanlance)
{
	if (!m_pOutput)
		return ERR_PLAYER_INVALID_STATE;

	return m_pOutput->setVolume(volume, nBanlance);
}

void CMDVorbis::DecodeThread(LPVOID lpParam)
{
	CMDVorbis		*pMDRow;

	pMDRow = (CMDVorbis *)lpParam;

	pMDRow->AddRef();
	pMDRow->DecodeThreadProc();
	pMDRow->Release();
}


void CMDVorbis::DecodeThreadProc()
{
	VorbisDecode();

	while (!m_bKillThread && m_pOutput->IsPlaying())
	{
		Sleep(10);
	}

	m_state = PS_STOPPED;

	m_pOutput->stop();
	m_pOutput->Release();
	m_pOutput = NULL;

	m_pInput->Release();
	m_pInput = NULL;

	m_pMemAllocator->Release();
	m_pMemAllocator = NULL;

	m_pPlayer->notifyEod(this, ERR_OK);
	m_pPlayer->Release();
	m_pPlayer = NULL;
}

void CMDVorbis::VorbisDecode()
{
	#define DECODE_BUFF_SIZE (1024 * 8)

	IFBuffer	*pBuf = NULL;

	while (!m_bKillThread)
	{
		if (m_bSeekFlag)
		{
			ov_pcm_seek_page(&m_file, (double)m_nSeekPos * m_audioInfo.nSampleRate / 1000);
			m_bSeekFlag = false;
		}

		pBuf = m_pMemAllocator->AllocFBuffer(DECODE_BUFF_SIZE);
		int ret = ov_read(&m_file, (char*)pBuf->data(), pBuf->capacity(), NULL);
		if (ret <= 0)		// eof
		{
			pBuf->Release();
			break;
		}
		pBuf->resize(ret);

		m_pOutput->WaitForWrite();

		if (m_bKillThread)
		{
			pBuf->Release();
			break;
		}

		if (m_bSeekFlag)
			pBuf->Release();
		else
		{
			m_pPlayer->OutputWrite(pBuf, m_audioInfo.nBps, m_audioInfo.nChannels, m_audioInfo.nSampleRate);
		}
	}

	ov_clear(&m_file);
}

/*bool CMDVorbis::OutputWrite(IFBuffer *pBuf)
{
	m_pOutput->WaitForWrite();

	if (m_bKillThread)
		return false;

	if (m_bSeekFlag)
	{
		pBuf->Release();
		return true;
	}

	if (m_pPlayer->IsDspActive())
		m_pPlayer->DspProcess(pBuf, m_audioInfo.nBps, m_audioInfo.nChannels, m_audioInfo.nSampleRate);

	m_pPlayer->AddVisData(pBuf, m_audioInfo.nBps, m_audioInfo.nChannels, m_audioInfo.nSampleRate);
	m_pOutput->Write(pBuf);

	return true;
}*/


#ifdef _WIN32_WCE

IMediaDecoder *MPlayerGetMDOgg()
{
	return new CMDVorbis;
}

#else

//
// Query all Plugins and its description in a DLL
// Query will start with nIndex = 0, till !ERR_OK returned.
//
// nIndex = 0, return its interface type, and description, and lpInterface.
// lpInterface can be NULL.
//
extern "C" __declspec(dllexport) ResultCode DHPlayerQueryPluginIF(
	int index,
	MPInterfaceType *interfaceType,
	const char **description,
	LPVOID *interfacePtr
	)
{
	if (index == 0)
	{
		*interfaceType = MPIT_DECODE;
		if (description)
			description = SZ_DECODER_DESC;
		if (interfacePtr) {
			IMediaDecoder	*pDecoder = new CMDVorbis;
			pDecoder->AddRef();
			*interfacePtr = pDecoder;
		}

		return ERR_OK;
	}
	else
		return ERR_FALSE;
}

#endif // _WIN32_WCE
