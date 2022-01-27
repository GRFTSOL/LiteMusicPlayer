// MDAgent.cpp: implementation of the CMDAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayer.h"
#include "MDAgent.h"
#include "MPTime.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMDAgent::CMDAgent()
{
    OBJ_REFERENCE_INIT
}

CMDAgent::~CMDAgent()
{

}

LPCXSTR CMDAgent::getDescription()
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->getDescription();
    else
        return "MDAgen: no decoder was specified.";
}

LPCXSTR CMDAgent::getFileExtentions()
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->getFileExtentions();
    else
        return "MDAgen: no decoder was specified.";
}

MLRESULT CMDAgent::getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia)
{
    MLRESULT                nRet;

    CMPAutoPtr<IMediaDecode>    pDecode;
    nRet = m_pPlayer->m_pluginMgrAgent.newDecoder(pInput, &pDecode);
    if (nRet != ERR_OK)
        return nRet;

    nRet = pDecode->getMediaInfo(m_pPlayer, pInput, pMedia);
    if (nRet != ERR_OK)
        return nRet;

    return ERR_OK;
}


//
// decode media file related methods
//

bool CMDAgent::isSeekable()
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->isSeekable();
    else
        return false;
}

bool CMDAgent::isUseOutputPlug()
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->isUseOutputPlug();
    else
        return false;
}

//    ERR_PLAYER_INVALID_FILE:    decoder can't decode the file
//  ERR_DECODER_INNER_ERROR:    inner error occurs at decoder
//  ERR_DECODER_UNSUPPORTED_FEATURE:
//  ERR_DECODER_INIT_FAILED:
MLRESULT CMDAgent::play(IMPlayer *pPlayer, IMediaInput *pInput)
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->play(pPlayer, pInput);
    else
        return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::pause()
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->pause();
    else
        return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::unpause()
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->unpause();
    else
        return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::stop()
{
    MLRESULT    nRet;

    IMediaDecode *pMediaDecode = m_pMediaDecode;
    pMediaDecode->addRef();
    nRet = pMediaDecode->stop();
    pMediaDecode->release();

    return nRet;
}
// media length, pos related functions, unit: ms
uint32_t CMDAgent::getLength()
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->getLength();
    else
        return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::seek(uint32_t nPos)
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->seek(nPos);
    else
        return ERR_NO_DEVICE;
}

uint32_t CMDAgent::getPos()
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->getPos();
    else
        return 0;
}

// volume
MLRESULT CMDAgent::setVolume(int nVolume, int nBanlance)
{
    if (m_pMediaDecode.p)
        return m_pMediaDecode->setVolume(nVolume, nBanlance);
    else
        return ERR_NO_DEVICE;
}

MLRESULT CMDAgent::init(CMPlayer *pPlayer)
{
    m_pPlayer = pPlayer;
    return ERR_OK;
}

MLRESULT CMDAgent::doDecode(IMedia *pMedia)
{
    MLRESULT                    nRet;
    CXStr                        strMedia;
    CMPAutoPtr<IMediaDecode>    pDecoder;
    CMPAutoPtr<IMediaInput>        pInput;
    CMPTime        timePlayed;

    nRet = pMedia->getSourceUrl(&strMedia);
    if (nRet != ERR_OK)
        return nRet;

    nRet = m_pPlayer->m_pluginMgrAgent.newInput(strMedia.c_str(), &pInput);
    if (nRet != ERR_OK)
        return nRet;

    nRet = m_pPlayer->m_pluginMgrAgent.newDecoder(pInput, &pDecoder);
    if (nRet != ERR_OK)
        return nRet;

    nRet = m_pPlayer->makeOutputReadyForDecode();
    if (nRet != ERR_OK)
        return nRet;

    m_pMediaInput = pInput;
    m_pMediaDecode = pDecoder;
    m_pMedia = pMedia;

    m_pMediaDecode->getMediaInfo(m_pPlayer, m_pMediaInput, m_pMedia);

    nRet = m_pMediaDecode->play(m_pPlayer, m_pMediaInput);
    if (nRet != ERR_OK)
    {
        if (nRet == ERR_MI_NOT_FOUND && pMedia->getID() != MEDIA_ID_INVALID)
        {
            // the source can't be opened, set it as deleted.
            m_pPlayer->m_pMediaLib->setDeleted(&pMedia);
        }
        goto R_FAILED;
    }

    if (m_pPlayer->m_nVolume != -1)
    {
        if (m_pPlayer->m_bMute)
            m_pMediaDecode->setVolume(0, 0);
        else
            m_pMediaDecode->setVolume(m_pPlayer->m_nVolume, m_pPlayer->m_nBalance);
    }

    // set the play time
    timePlayed.getCurrentTime();
    pMedia->setAttribute(MA_TIME_PLAYED, timePlayed.m_time);

    if (pMedia->getID() == MEDIA_ID_INVALID && m_pPlayer->isAutoAddToMediaLib())
        m_pPlayer->m_pMediaLib->add(pMedia);

    // If media info in media library isn't up to date,
    // update to media library.
    if (pMedia->getID() != MEDIA_ID_INVALID && !pMedia->isInfoUpdatedToMediaLib())
    {
        m_pPlayer->m_pMediaLib->updateMediaInfo(pMedia);
    }

    return ERR_OK;

R_FAILED:
    m_pMediaInput.release();

    return nRet;
}

void CMDAgent::notifyEod(IMediaDecode *pDecoder, MLRESULT nError)
{
    if (m_pMediaDecode.p == pDecoder)
        m_pMediaDecode.release();
}
