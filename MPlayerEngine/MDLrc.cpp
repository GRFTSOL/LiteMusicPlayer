#include "MDLrc.h"
#include "../LyricsLib/MLLib.h"

CMDLrc::CMDLrc(void)
{
}

CMDLrc::~CMDLrc(void)
{
}

LPCXSTR CMDLrc::getDescription()
{
    return "LRC decoder";
}

LPCXSTR CMDLrc::getFileExtentions()
{
    return ".lrc|LRC file";
}

MLRESULT CMDLrc::getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
    CMLData data;
    int nRet = data.openLyrics(pInput->getSource(), 0, pInput->getSource());

    m_nLength = data.properties().getMediaLengthInt();

    return nRet;
}

//
// decode media file related methods
//

bool CMDLrc::isSeekable()
{
    return true;
}

bool CMDLrc::isUseOutputPlug()
{
    return false;
}

// ERR_PLAYER_INVALID_FILE: decoder can't decode the file
// ERR_DECODER_INNER_ERROR: inner error occurs at decoder
// ERR_DECODER_UNSUPPORTED_FEATURE:
// ERR_DECODER_INIT_FAILED:
MLRESULT CMDLrc::play(IMPlayer *pPlayer, IMediaInput *pInput)
{
    m_dwBeginTime = getTickCount();
    return ERR_OK;
}

MLRESULT CMDLrc::pause()
{
    if (m_state == PS_PLAYING)
    {
        m_state = PS_PAUSED;
        m_nPausedPos = getTickCount() - m_dwBeginTime;
    }

    return ERR_OK;
}

MLRESULT CMDLrc::unpause()
{
    if (m_state == PS_PAUSED)
    {
        m_state = PS_PLAYING;
        m_dwBeginTime = getTickCount() - m_nPausedPos;
    }

    return ERR_OK;
}

MLRESULT CMDLrc::stop()
{
    m_dwBeginTime = 0;
    m_nPausedPos = 0;
    m_state = PS_STOPED;
    return ERR_OK;
}

// media length, pos related functions, unit: ms
uint32_t CMDLrc::getLength()
{
    return m_nLength;
}

MLRESULT CMDLrc::seek(uint32_t nPos)
{
    if (m_state == PS_STOPED)
        return ERR_OK;

    if (nPos < 0)
        nPos = 0;
    else if (nPos > m_nLength)
        nPos = m_nLength;

    m_dwBeginTime = getTickCount() - nPos;
    m_nPausedPos = nPos;

    return ERR_OK;
}

uint32_t CMDLrc::getPos()
{
    return getTickCount() - m_dwBeginTime;
}

// volume
MLRESULT CMDLrc::setVolume(int nVolume, int nBanlance)
{
    return ERR_OK;
}

bool CMDLrc::init()
{
    return true;
}

MLRESULT CMDLrc::doDecode(IMedia *pMedia)
{
    CXStr                        strMedia;

    MLRESULT nRet = pMedia->getSourceUrl(&strMedia);
    if (nRet != ERR_OK)
        return nRet;

    CMLData data;
    nRet = data.openLyrics(strMedia.c_str(), 0, strMedia.c_str());

    m_nLength = data.properties().getMediaLengthInt();
    m_dwBeginTime = getTickCount();
    m_state = PS_PLAYING;
    m_nPausedPos = 0;

    return ERR_OK;
}

void CMDLrc::notifyEod(IMediaDecode *pDecoder, MLRESULT nError)
{
    m_state = PS_STOPED;
}

bool CMDLrc::isOK()
{
    return true;
}
