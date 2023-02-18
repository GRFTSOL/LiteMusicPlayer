// MDFlac.cpp: implementation of the CMDFlac class.
//
//////////////////////////////////////////////////////////////////////


#include "../../base/base.h"

#include "playback.h"
#include "tagz.h"

#include "MDFlac.h"
#include "FLacMetadataInfo.h"


#define SZ_DECODER_DESC		_T("FLAC File decoder")


void AudioInfoFromStreamInfo(AUDIO_INFO &m_audioInfo, const FLAC__StreamMetadata_StreamInfo *streaminfo)
{
	m_audioInfo.nTotalSamples = (unsigned)(streaminfo->total_samples&0xfffffffful);
	m_audioInfo.nBps = streaminfo->bits_per_sample;
	m_audioInfo.nChannels = streaminfo->channels;
	m_audioInfo.nSampleRate = streaminfo->sample_rate;

	m_audioInfo.nMediaLength = (uint32)((double)m_audioInfo.nTotalSamples / (double)m_audioInfo.nSampleRate * 1000.0 + 0.5);
	m_audioInfo.nBitRate = 
		 int((double)m_audioInfo.nMediaFileSize * 8 * 1000 / m_audioInfo.nMediaLength);
}

/* (24/8) for max bytes per sample, and 2 for DSPs */
/*
void FLAC_plugin__show_error(const char *message,...)
{
	char foo[512];
	va_list args;
	va_start(args, message);
	vsprintf(foo, message, args);
	va_end(args);
	MessageBoxA(NULL, foo, "FLAC Plug-in Error", MB_ICONSTOP);
}*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMDFlac::CMDFlac()
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

	m_nBytesPerSample = 0;

	m_pFlacDecoder = FLAC__stream_decoder_new();
}

CMDFlac::~CMDFlac()
{
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
	if (m_pFlacDecoder)
	{
		FLAC__stream_decoder_finish(m_pFlacDecoder);
		FLAC__stream_decoder_delete(m_pFlacDecoder);
	}
}

OBJ_REFERENCE_IMP(CMDFlac)

cstr_t CMDFlac::getDescription()
{
	return SZ_DECODER_DESC;
}

// get supported file's extentions.
cstr_t CMDFlac::getFileExtentions()
{
	return _T(".flac\0flac files\0");
}

const char *FP_tags_get_tag_utf8(const FLAC__StreamMetadata *tags, const char *name)
{
	const int i = FLAC__metadata_object_vorbiscomment_find_entry_from(tags, /*offset=*/0, name);
	return (i < 0? 0 : strchr((const char *)(tags->data.vorbis_comment.comments[i].entry), '=') + 1);
}

void FP_tags_get_tag_ucs2(const FLAC__StreamMetadata *tags, const char *name, wstringex &strValue)
{
	strValue.clear();
	const char *utf8 = FP_tags_get_tag_utf8(tags, name);
	if (utf8)
		Utf8ToUCS2(utf8, -1, strValue);
}

#define SetMediaAttribute(MName, FlacName)	FP_tags_get_tag_ucs2(tags, FlacName, strValue); \
                                pMedia->setAttribute(MName, strValue.c_str());

// #define SetMediaAttribute(MName, FlacName)	szValue = (wchar_t *)FLAC_plugin__tags_get_tag_ucs2(tags, FlacName); \
//                                 pMedia->setAttribute(MName, szValue); \
//                                 free(szValue)

ResultCode CMDFlac::getMediaInfo(IMediaInput *pInput, IMediaInfo *pMedia)
{
	FLAC__StreamMetadata *streaminfo;
	FLAC__StreamMetadata *tags = NULL;
	float	ratioCompress;
	wstringex strValue;
	// wchar_t	*szValue;
	int		nRet;

	pInput->GetSize(m_audioInfo.nMediaFileSize);

	nRet = MetadataInfo_get_streaminfo_and_tags(pInput, &streaminfo, &tags);
	if (nRet != ERR_OK)
		return nRet;

	if (streaminfo)
	{
		AudioInfoFromStreamInfo(m_audioInfo, &(streaminfo->data.stream_info));

		ratioCompress = (float)(m_audioInfo.nBitRate * 1000000 / (streaminfo->data.stream_info.sample_rate*streaminfo->data.stream_info.channels*streaminfo->data.stream_info.bits_per_sample));

		m_nBytesPerSample = m_audioInfo.nBps;

		pMedia->setAttribute(MA_FILESIZE, m_audioInfo.nMediaFileSize);
		pMedia->setAttribute(MA_DURATION, m_audioInfo.nMediaLength);
		pMedia->setAttribute(MA_SAMPLE_RATE, m_audioInfo.nSampleRate);
		pMedia->setAttribute(MA_CHANNELS, m_audioInfo.nChannels);
		pMedia->setAttribute(MA_BITS_PER_SAMPLE, m_audioInfo.nBps);
		pMedia->setAttribute(MA_BITRATE, m_audioInfo.nBitRate);

		FLAC__metadata_object_delete(streaminfo);
	}

	if (tags)
	{
		SetMediaAttribute(MA_TITLE, "TITLE");
		SetMediaAttribute(MA_ARTIST, "ARTIST");
		SetMediaAttribute(MA_ALBUM, "ALBUM");
		SetMediaAttribute(MA_COMMENT, "COMMENT");
		SetMediaAttribute(MA_YEAR, "DATE");
		SetMediaAttribute(MA_TRACK_NUMB, "TRACKNUMBER");
		SetMediaAttribute(MA_GENRE, "GENRE");
		FLAC__metadata_object_delete(tags);
	}

	return ERR_OK;
}

bool CMDFlac::isSeekable()
{
	return true;
}

bool CMDFlac::isUseOutputPlug()
{
	return true;
}

ResultCode CMDFlac::play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	ResultCode		nRet;
	stringex	strFile;

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

	ASSERT(m_pFlacDecoder != 0);

	/* init decoder */
	nRet = FLAC_plugin__decoder_init(this);
	if (nRet != ERR_OK)
		goto R_FAILED;

	nRet = m_pOutput->Open(m_audioInfo.nSampleRate, m_audioInfo.nChannels, m_audioInfo.nBps);
	if (nRet != ERR_OK)
	{
		FLAC__stream_decoder_finish(m_pFlacDecoder);
		goto R_FAILED;
	}

	m_state = PS_PLAYING;
	m_bKillThread = false;

	m_threadDecode.Create(DecodeThread, this);
	m_threadDecode.SetPriority(THREAD_PRIORITY_TIME_CRITICAL);

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

ResultCode CMDFlac::pause()
{
	if (m_state != PS_PLAYING)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PAUSED;
	ASSERT(m_pOutput);
	return m_pOutput->pause(true);
}

ResultCode CMDFlac::unpause()
{
	if (m_state != PS_PAUSED)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PLAYING;
	ASSERT(m_pOutput);
	return m_pOutput->pause(false);
}

bool CMDFlac::IsPaused()
{
	return m_state == PS_PAUSED;
}

ResultCode CMDFlac::stop()
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

uint32 CMDFlac::getLength()
{
	return m_audioInfo.nMediaLength;
}

ResultCode CMDFlac::Seek(uint32 dwPos)
{
	m_bSeekFlag = true;
	m_nSeekPos = dwPos;

	ASSERT(m_pOutput);
	if (m_pOutput && m_state == PS_PLAYING)
		m_pOutput->Flush();

	return ERR_OK;
}

uint32 CMDFlac::GetPos()
{
	if (m_pOutput && !m_bSeekFlag)
		return m_nSeekPos + m_pOutput->GetPos();
	else
		return m_nSeekPos;
}

ResultCode CMDFlac::setVolume(int volume, int nBanlance)
{
	if (!m_pOutput)
		return ERR_PLAYER_INVALID_STATE;

	return m_pOutput->setVolume(volume, nBanlance);
}

void CMDFlac::DecodeThread(LPVOID lpParam)
{
	CMDFlac		*pMDRow;

	pMDRow = (CMDFlac *)lpParam;

	pMDRow->AddRef();
	pMDRow->DecodeThreadProc();
	pMDRow->Release();
}

void CMDFlac::DecodeThreadProc()
{
	while (!m_bKillThread)
	{
		// seek needed
		if (m_bSeekFlag)
		{
			FLAC_plugin__seek(this);
			m_bSeekFlag = false;
			m_pOutput->Flush();
		}
		else
		{
			// decode
			if (FLAC__stream_decoder_get_state(m_pFlacDecoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
			{
				break;
			}
			else if (!FLAC__stream_decoder_process_single(m_pFlacDecoder))
			{
				break;
			}
		}
	}

	FLAC__stream_decoder_finish(m_pFlacDecoder);

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

bool CMDFlac::OutputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate)
{
	m_pOutput->WaitForWrite();

	if (m_bKillThread)
		return false;

	if (m_bSeekFlag)
	{
		pBuf->Release();
		return true;
	}

	m_pPlayer->OutputWrite(pBuf, nBps, nChannels, nSampleRate);

	return true;
}


#ifdef _WIN32_WCE

IMediaDecoder *MPlayerGetMDFlac()
{
	return new CMDFlac;
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
			IMediaDecoder *pDecoder = new CMDFlac;
			pDecoder->AddRef();
			*interfacePtr = pDecoder;
		}

		return ERR_OK;
	}
	else
		return ERR_FALSE;
}

#endif // _WIN32_WCE
