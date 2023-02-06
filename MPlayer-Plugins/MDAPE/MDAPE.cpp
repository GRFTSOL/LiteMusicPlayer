// MDAPE.cpp: implementation of the CMDAPE class.
//
//////////////////////////////////////////////////////////////////////

#include "MDAPE.h"
#define _UINT32ETC
#include "../APESDK/Source\Shared/All.h"
#include "../APESDK/Source\Shared/io.h"
#include "../APESDK/Source\MACLib/MACLib.h"
#include "../APESDK/Source\MACLib/APETag.h"


#define SZ_DECODER_DESC		_T("APE File decoder")


class CMDAPEIO : public CIO
{
public:
    CMDAPEIO(IMediaInput *pInput) { m_pInput = pInput; }
    virtual ~CMDAPEIO() { };

    // open / close
    virtual int Open(const wchar_t * pName)
	{
		m_pInput->Seek(0, SEEK_SET);
		// m_pInput->Open(pName);
		return 0;
	}
    virtual int Close()
	{
		// m_pInput->Close();
		return 0;
	}
    
    // read / write
    virtual int Read(void * pBuffer, unsigned int nBytesToRead, unsigned int * pBytesRead)
	{
		*pBytesRead = m_pInput->Read(pBuffer, nBytesToRead);
		return 0;
	}
    virtual int Write(const void * pBuffer, unsigned int nBytesToWrite, unsigned int * pBytesWritten)
	{
		// m_pInput-
		return ERROR_IO_WRITE;
	}
    
    // seek
    virtual int Seek(int nDistance, unsigned int nMoveMode)
	{
		m_pInput->Seek(nDistance, nMoveMode);
		return 0;
	}

    // creation / destruction
    virtual int Create(const wchar_t * pName)
	{
		return -1;
	}
    virtual int Delete()
	{
		return -1;
	}

    // other functions
    virtual int SetEOF()
	{
		return -1;
	}

    // attributes
    virtual int GetPosition()
	{
		return m_pInput->GetPos();
	}
    virtual int GetSize()
	{
		uint32		nSize = 0;
		m_pInput->GetSize(nSize);
		return nSize;
	}
    virtual int GetName(wchar_t * pBuffer)
	{
		wcscpy(pBuffer, m_pInput->getSource());
		return 0;
	}

protected:
	IMediaInput		*m_pInput;
};

struct APE_Decoder
{
	APE_Decoder(IAPEDecompress *_apeDecompress, IMediaInput *pInput) : apeDecompress(_apeDecompress), io(pInput) { }
	IAPEDecompress		*apeDecompress;
	CMDAPEIO			io;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMDAPE::CMDAPE()
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

	m_lpAPEDecoder = NULL;
}

CMDAPE::~CMDAPE()
{
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
	ASSERT(m_lpAPEDecoder == NULL);
}

OBJ_REFERENCE_IMP(CMDAPE)

cstr_t CMDAPE::getDescription()
{
	return SZ_DECODER_DESC;
}

// get supported file's extentions.
cstr_t CMDAPE::getFileExtentions()
{
	return _T(".ape\0APE Files\0");
}

ResultCode CMDAPE::getMediaInfo(IMediaInput *pInput, IMediaInfo *pMedia)
{
	int				nErrCode;
	IAPEDecompress	*apeDecompress;
	CMDAPEIO		io(pInput);
	CAPETag			*pTag;

	pInput->Seek(0, SEEK_SET);

	apeDecompress = CreateIAPEDecompressEx(&io, &nErrCode);
	if (!apeDecompress)
		return ERR_FALSE;

	memset(&m_audioInfo, 0, sizeof(m_audioInfo));
	m_audioInfo.nSampleRate = apeDecompress->GetInfo(APE_INFO_SAMPLE_RATE);
	m_audioInfo.nBps = apeDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE);
	m_audioInfo.nChannels = apeDecompress->GetInfo(APE_INFO_CHANNELS);
	m_audioInfo.nMediaLength = apeDecompress->GetInfo(APE_INFO_LENGTH_MS);
	pInput->GetSize(m_audioInfo.nMediaFileSize);

	pTag = (CAPETag *)apeDecompress->GetInfo(APE_INFO_TAG);
	if (pTag)
	{
		WCHAR		szValue[MAX_PATH];
		int			nBuffSize;

		nBuffSize = CountOf(szValue);
		nErrCode = pTag->GetFieldString(APE_TAG_FIELD_TITLE, szValue, &nBuffSize);
		if (nErrCode == -1)
			pMedia->setAttribute(MA_TITLE, szValue);

		nBuffSize = CountOf(szValue);
		nErrCode = pTag->GetFieldString(APE_TAG_FIELD_ARTIST, szValue, &nBuffSize);
		if (nErrCode == -1)
			pMedia->setAttribute(MA_ARTIST, szValue);

		nBuffSize = CountOf(szValue);
		nErrCode = pTag->GetFieldString(APE_TAG_FIELD_ALBUM, szValue, &nBuffSize);
		if (nErrCode == -1)
			pMedia->setAttribute(MA_ALBUM, szValue);

		nBuffSize = CountOf(szValue);
		nErrCode = pTag->GetFieldString(APE_TAG_FIELD_COMMENT, szValue, &nBuffSize);
		if (nErrCode == -1)
			pMedia->setAttribute(MA_COMMENT, szValue);

		nBuffSize = CountOf(szValue);
		nErrCode = pTag->GetFieldString(APE_TAG_FIELD_YEAR, szValue, &nBuffSize);
		if (nErrCode == -1)
			pMedia->setAttribute(MA_YEAR, szValue);

		nBuffSize = CountOf(szValue);
		nErrCode = pTag->GetFieldString(APE_TAG_FIELD_TRACK, szValue, &nBuffSize);
		if (nErrCode == -1)
			pMedia->setAttribute(MA_TRACK_NUMB, szValue);

		nBuffSize = CountOf(szValue);
		nErrCode = pTag->GetFieldString(APE_TAG_FIELD_GENRE, szValue, &nBuffSize);
		if (nErrCode == -1)
			pMedia->setAttribute(MA_GENRE, szValue);
	}

	pMedia->setAttribute(MA_FILESIZE, m_audioInfo.nMediaFileSize);
	pMedia->setAttribute(MA_DURATION, m_audioInfo.nMediaLength);
	pMedia->setAttribute(MA_SAMPLE_RATE, m_audioInfo.nSampleRate);
	pMedia->setAttribute(MA_CHANNELS, m_audioInfo.nChannels);
	pMedia->setAttribute(MA_BPS, m_audioInfo.nBps);

	DestroyIAPEDecompress(apeDecompress);

	return ERR_OK;
}

bool CMDAPE::isSeekable()
{
	return true;
}

bool CMDAPE::isUseOutputPlug()
{
	return true;
}

ResultCode CMDAPE::play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	ResultCode		nRet;
	IAPEDecompress	*apeDecompress = NULL;
	APE_Decoder		*pAPEDecoder = NULL;
	int				nErrCode;

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

	pAPEDecoder = new APE_Decoder(apeDecompress, m_pInput);

	apeDecompress = CreateIAPEDecompressEx(&pAPEDecoder->io, &nErrCode);
	if (!apeDecompress)
	{
		nRet = ERR_PLAYER_INVALID_FILE;
		goto R_FAILED;
	}
	pAPEDecoder->apeDecompress = apeDecompress;

	memset(&m_audioInfo, 0, sizeof(m_audioInfo));
	m_audioInfo.nSampleRate = apeDecompress->GetInfo(APE_INFO_SAMPLE_RATE);
	m_audioInfo.nBps = apeDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE);
	m_audioInfo.nChannels = apeDecompress->GetInfo(APE_INFO_CHANNELS);
	m_audioInfo.nMediaLength = apeDecompress->GetInfo(APE_INFO_LENGTH_MS);
	m_audioInfo.nBitRate = apeDecompress->GetInfo(APE_INFO_AVERAGE_BITRATE);
	m_pInput->GetSize(m_audioInfo.nMediaFileSize);

	nRet = m_pOutput->Open(m_audioInfo.nSampleRate, m_audioInfo.nChannels, m_audioInfo.nBps);
	if (nRet != ERR_OK)
		goto R_FAILED;

	m_state = PS_PLAYING;
	m_bKillThread = false;
	m_lpAPEDecoder = pAPEDecoder;

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
	if (pAPEDecoder)
	{
		delete pAPEDecoder;
		pAPEDecoder = NULL;
	}
	if (apeDecompress)
	{
		DestroyIAPEDecompress(apeDecompress);
	}
	return nRet;
}

ResultCode CMDAPE::pause()
{
	if (m_state != PS_PLAYING)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PAUSED;
	ASSERT(m_pOutput);
	return m_pOutput->pause(true);
}

ResultCode CMDAPE::unpause()
{
	if (m_state != PS_PAUSED)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PLAYING;
	ASSERT(m_pOutput);
	return m_pOutput->pause(false);
}

bool CMDAPE::IsPaused()
{
	return m_state == PS_PAUSED;
}

ResultCode CMDAPE::stop()
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

uint32 CMDAPE::getLength()
{
	return m_audioInfo.nMediaLength;
}

ResultCode CMDAPE::Seek(uint32 dwPos)
{
	m_bSeekFlag = true;
	m_nSeekPos = dwPos;

	ASSERT(m_pOutput);
	if (m_pOutput && m_state == PS_PLAYING)
		m_pOutput->Flush();

	return ERR_OK;
}

uint32 CMDAPE::GetPos()
{
	if (m_pOutput && !m_bSeekFlag)
		return m_nSeekPos + m_pOutput->GetPos();
	else
		return m_nSeekPos;
}

ResultCode CMDAPE::setVolume(int volume, int nBanlance)
{
	if (!m_pOutput)
		return ERR_PLAYER_INVALID_STATE;

	return m_pOutput->setVolume(volume, nBanlance);
}

void CMDAPE::DecodeThread(LPVOID lpParam)
{
	CMDAPE		*pMDRow;

	pMDRow = (CMDAPE *)lpParam;

	pMDRow->AddRef();
	pMDRow->DecodeThreadProc();
	pMDRow->Release();
}

void CMDAPE::DecodeThreadProc()
{
	#define DECODE_BUFF_SIZE (1024 * 8)

	int				nErrCode;
	IAPEDecompress	*apeDecompress;
	APE_Decoder		*pAPEDecoder = (APE_Decoder*)m_lpAPEDecoder;
	int				nBlockCount, nSamplePerBlock, nBlockRetrived;
	int				nBlockAlign;

	apeDecompress = pAPEDecoder->apeDecompress;

	nBlockAlign = apeDecompress->GetInfo(APE_INFO_BLOCK_ALIGN);
	nSamplePerBlock = nBlockAlign / (m_audioInfo.nBps / 8) / m_audioInfo.nChannels;
	nBlockCount = apeDecompress->GetInfo(APE_INFO_TOTAL_BLOCKS);

	IFBuffer	*pBuf = NULL;
	pBuf = m_pMemAllocator->AllocFBuffer(DECODE_BUFF_SIZE);

	while (!m_bKillThread)
	{
		if (m_bSeekFlag)
		{
			// apeDecompress->Seek()
			int		nBlockSeek = (int)((double)m_nSeekPos * m_audioInfo.nSampleRate / 1000 / nSamplePerBlock);
			apeDecompress->Seek(nBlockSeek);
			m_bSeekFlag = false;
		}

#define BUFF_LEN		nBlockAlign	* 1024

		ASSERT(pBuf->capacity() >= DECODE_BUFF_SIZE);
		if (DECODE_BUFF_SIZE - pBuf->size() <= BUFF_LEN)
		{
			if (!OutputWrite(pBuf, m_audioInfo.nBps, m_audioInfo.nChannels, m_audioInfo.nSampleRate))
				break;
			pBuf = m_pMemAllocator->AllocFBuffer(DECODE_BUFF_SIZE);
		}

		nErrCode = apeDecompress->GetData(pBuf->data() + pBuf->size(), 1024, &nBlockRetrived);
		if (nErrCode != ERROR_SUCCESS || nBlockRetrived == 0)
			break;

		pBuf->resize(pBuf->size() + nBlockRetrived * nBlockAlign);
	}

	if (m_bKillThread && pBuf)
	{
		pBuf->Release();
		pBuf = NULL;
	}

	if (pBuf && pBuf->size())
	{
		OutputWrite(pBuf, m_audioInfo.nBps, m_audioInfo.nChannels, m_audioInfo.nSampleRate);
	}

	while (!m_bKillThread && m_pOutput->IsPlaying())
	{
		Sleep(10);
	}

	DestroyIAPEDecompress(apeDecompress);
	delete pAPEDecoder;
	m_lpAPEDecoder = NULL;

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

bool CMDAPE::OutputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate)
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

IMediaDecoder *MPlayerGetMDAPE()
{
	return new CMDAPE;
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
			*description = SZ_DECODER_DESC;
		if (interfacePtr) {
			IMediaDecoder *pDecoder = new CMDAPE;
			pDecoder->AddRef();
			*interfacePtr = pDecoder;
		}

		return ERR_OK;
	}
	else
		return ERR_FALSE;
}

#endif // _WIN32_WCE
