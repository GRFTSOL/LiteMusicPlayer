// MDWMA.cpp: implementation of the CMDWMA class.
//
//////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT	0x0400

#include <tchar.h>
extern "C"
{
#include "avcodec.h"
#include "avformat.h"

};

//#include "iir.h"

#include "MDWMA.h"

#define SZ_DECODER_DESC		_T("Ogg File decoder")


//////////////////////////////////////////////////////////////////////

CMDWMA::CMDWMA()
{
	OBJ_REFERENCE_INIT;

	m_pInput = NULL;
	m_pPlayer = NULL;
	m_pOutput = NULL;
	m_bPaused = false;
	m_bKillThread = false;
	m_nSeekPos = 0;
	m_bSeekFlag = false;
	m_state = PS_STOPED;

    avcodec_init();
    avcodec_register_all();
    av_register_all();

	m_CodecContext = NULL;
	m_FormatContext = NULL;
	m_nWmaIndex;
    m_Codec = NULL;
}

CMDWMA::~CMDWMA()
{
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
}

OBJ_REFERENCE_IMP(CMDWMA)

cstr_t CMDWMA::getDescription()
{
	return SZ_DECODER_DESC;
}

// get supported file's extentions.
cstr_t CMDWMA::getFileExtentions()
{
	return _T(".wma\0wma files\0");
}

MLRESULT CMDWMA::getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
	pInput->Seek(0, SEEK_SET);
	pInput->GetSize(m_audioInfo.nMediaFileSize);

	memset(&m_audioInfo, 0, sizeof(m_audioInfo));

    AVFormatContext *in = NULL;
	stringex	strFileName;
	ConvertStr(pInput->getSource(), -1, strFileName);

    if (av_open_input_file(&in, strFileName.c_str(), NULL, 0, NULL) < 0)
		return ERR_OPEN_FILE;

    av_find_stream_info(in);

	m_audioInfo.nMediaLength = in->duration / (AV_TIME_BASE / 1000);
	if (!IsEmptyString(in->author))
		pMedia->setAttribute(MA_ARTIST, in->author);
	if (!IsEmptyString(in->album))
		pMedia->setAttribute(MA_ALBUM, in->album);
	if (!IsEmptyString(in->title))
		pMedia->setAttribute(MA_TITLE, in->title);
	if (!IsEmptyString(in->genre))
		pMedia->setAttribute(MA_GENRE, in->genre);
	if (!IsEmptyString(in->comment))
		pMedia->setAttribute(MA_COMMENT, in->comment);
	pMedia->setAttribute(MA_TRACK_NUMB, in->track);

	av_close_input_file(in);

 	pMedia->setAttribute(MA_FILESIZE, m_audioInfo.nMediaFileSize);
 	pMedia->setAttribute(MA_DURATION, m_audioInfo.nMediaLength);
/* 	pMedia->setAttribute(SZ_M_SampleRate, m_audioInfo.nSampleRate, false);
 	pMedia->setAttribute(SZ_M_Channels, m_audioInfo.nChannels, false);
 	pMedia->setAttribute(SZ_M_BPS, m_audioInfo.nBps, false);*/

	return ERR_OK;
}

bool CMDWMA::isSeekable()
{
	return m_audioInfo.m_bSeekable;
}

bool CMDWMA::isUseOutputPlug()
{
	return true;
}

MLRESULT CMDWMA::play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	MLRESULT		nRet;
	stringex	strFileName;

	m_state = PS_STOPED;
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


	//
	// Open wma file, and audio output device
	//
	ConvertStr(m_pInput->getSource(), -1, strFileName);
    if (av_open_input_file(&m_FormatContext, strFileName.c_str(), NULL, 0, NULL) < 0)
		return ERR_DECODE_FAILED;

    for(m_nWmaIndex = 0; m_nWmaIndex < m_FormatContext->nb_streams; m_nWmaIndex++)
	{
        m_CodecContext = &m_FormatContext->streams[m_nWmaIndex]->codec;
        if (m_CodecContext->codec_type == CODEC_TYPE_AUDIO)
			break;
    }

    av_find_stream_info(m_FormatContext);
    m_Codec = avcodec_find_decoder(m_CodecContext->codec_id);
    if (!m_Codec)
		return ERR_NOT_SUPPORT;
 
    if (avcodec_open(m_CodecContext, m_Codec) < 0)
		return ERR_DECODER_UNSUPPORTED_FEATURE;

	m_audioInfo.nBps = 16;
	m_audioInfo.nSampleRate = m_CodecContext->sample_rate;
	m_audioInfo.nChannels = m_CodecContext->channels;
	m_audioInfo.nBitRate = m_CodecContext->bit_rate;
	m_audioInfo.nMediaLength = m_FormatContext->duration / (AV_TIME_BASE / 1000);

 	nRet = m_pOutput->Open(m_audioInfo.nSampleRate, m_audioInfo.nChannels, m_audioInfo.nBps);
 	if (nRet != ERR_OK)
		return nRet;

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

MLRESULT CMDWMA::pause()
{
	if (m_state != PS_PLAYING)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PAUSED;
	ASSERT(m_pOutput);
	return m_pOutput->pause(true);
}

MLRESULT CMDWMA::unpause()
{
	if (m_state != PS_PAUSED)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PLAYING;
	ASSERT(m_pOutput);
	return m_pOutput->pause(false);
}

bool CMDWMA::IsPaused()
{
	return m_state == PS_PAUSED;
}

MLRESULT CMDWMA::stop()
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

uint32 CMDWMA::getLength()
{
	return m_audioInfo.nMediaLength;
}

MLRESULT CMDWMA::Seek(uint32 dwPos)
{
	m_bSeekFlag = true;
	m_nSeekPos = dwPos;

	ASSERT(m_pOutput);
	if (m_pOutput && m_state == PS_PLAYING)
		m_pOutput->Flush();

	return ERR_OK;
}

uint32 CMDWMA::GetPos()
{
	if (m_pOutput && !m_bSeekFlag)
		return m_nSeekPos + m_pOutput->GetPos();
	else
		return m_nSeekPos;
}

MLRESULT CMDWMA::setVolume(int volume, int nBanlance)
{
	if (!m_pOutput)
		return ERR_PLAYER_INVALID_STATE;

	return m_pOutput->setVolume(volume, nBanlance);
}

void CMDWMA::DecodeThread(LPVOID lpParam)
{
	CMDWMA		*pMDRow;

	pMDRow = (CMDWMA *)lpParam;

	pMDRow->AddRef();
	pMDRow->DecodeThreadProc();
	pMDRow->Release();
}


void CMDWMA::DecodeThreadProc()
{
	WMADecode();

	while (!m_bKillThread && m_pOutput->IsPlaying())
	{
		Sleep(10);
	}

	m_state = PS_STOPED;

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

void CMDWMA::WMADecode()
{
#define		BLOCK_SIZE		512*2

	IFBuffer		*pBuf = NULL;
	uint8_t			*wma_outbuf;

    wma_outbuf = ( uint8_t *)malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE);

	//
	// decode loop
	//
	uint8_t		*inbuf_ptr;
	int			out_size, size, len;
	AVPacket	pkt;

	while (!m_bKillThread)
	{
		if (m_bSeekFlag)
		{
			av_seek_frame(m_FormatContext, m_nWmaIndex, (int64_t)m_nSeekPos * 1000L);
			m_bSeekFlag = false;
		}

		if (av_read_frame(m_FormatContext, &pkt) < 0)
			break;

		size = pkt.size;
		inbuf_ptr = pkt.data;

		if (size == 0)
			break;

		while(size > 0)
		{
			len = avcodec_decode_audio(m_CodecContext, (short *)wma_outbuf, &out_size,
									   inbuf_ptr, size);
			if (len < 0)
			{
				break;
			}

			if (out_size <= 0)
			{
				continue;
			}

			for (int pos = 0; pos < out_size; pos += BLOCK_SIZE)
			{
				pBuf = m_pMemAllocator->AllocFBuffer(BLOCK_SIZE);
				if (pos + BLOCK_SIZE < out_size)
				{
					memcpy(pBuf->data(), wma_outbuf + pos, BLOCK_SIZE);
					pBuf->resize(BLOCK_SIZE);
				}
				else
				{
					memcpy(pBuf->data(), wma_outbuf + pos, out_size - pos);
					pBuf->resize(out_size - pos);
				}

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

			// wma_playbuff(out_size);

			size -= len;
			inbuf_ptr += len;
			if (pkt.data)
				av_free_packet(&pkt);
		}
	}

	if (wma_outbuf)
		free(wma_outbuf);
	if (pkt.data)
		av_free_packet(&pkt);
	if (m_CodecContext)
	{
		avcodec_close(m_CodecContext);
		m_CodecContext= NULL;
	}
	if (m_FormatContext)
	{
		av_close_input_file(m_FormatContext);
		m_FormatContext = NULL;
	}
	m_Codec = NULL;
}

/*bool CMDWMA::OutputWrite(IFBuffer *pBuf)
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

IMediaDecode *MPlayerGetMDWMA()
{
	return new CMDWMA;
}

#else

//
// Query all Plugins and its description in a DLL
// Query will start with nIndex = 0, till !ERR_OK returned.
//
// nIndex = 0, return its interface type, and description, and lpInterface.
// lpInterface can be NULL.
//
extern "C" __declspec(dllexport) MLRESULT ZikiPlayerQueryPluginIF(
	int nIndex,
	MPInterfaceType *pInterfaceType,
	IString *strDescription,
	LPVOID *lpInterface
	)
{
	if (nIndex == 0)
	{
		*pInterfaceType = MPIT_DECODE;
		if (strDescription)
			strDescription->copy(SZ_DECODER_DESC);
		if (lpInterface)
		{
			IMediaDecode	*pDecoder = new CMDWMA;
			pDecoder->AddRef();
			*lpInterface = pDecoder;
		}

		return ERR_OK;
	}
	else
		return ERR_FALSE;
}

#endif // _WIN32_WCE
