// Playlist.cpp: implementation of the CPlaylist class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayer.h"
#include "Playlist.h"


CPlaylist::CPlaylist(CMPlayer *pPlayer) : m_pPlayer(pPlayer)
{
    OBJ_REFERENCE_INIT
    if (m_pPlayer)
        m_pPlayer->addRef();
}

CPlaylist::~CPlaylist()
{
    V_MEDIA::iterator    it, itEnd;

    itEnd = m_vMedia.end();
    for (it = m_vMedia.begin(); it != itEnd; ++it)
    {
        (*it)->release();
    }
    m_vMedia.clear();

    if (m_pPlayer)
        m_pPlayer->release();
}

uint32_t CPlaylist::getCount()
{
    MutexAutolock    autolock(m_mutexDataAccess);

    return m_vMedia.size();
}

MLRESULT CPlaylist::getItem(long nIndex, IMedia **ppMedia)
{
    MutexAutolock    autolock(m_mutexDataAccess);

//    assert(nIndex >= 0 && nIndex < (int)m_vMedia.size());

    if (nIndex >= 0 && nIndex < (int)m_vMedia.size())
    {
        *ppMedia = m_vMedia[nIndex];
        (*ppMedia)->addRef();
        return ERR_OK;
    }

    return ERR_NOT_FOUND;
}

MLRESULT CPlaylist::getName(IString *str)
{
    return ERR_OK;
}

MLRESULT CPlaylist::insertItem(long nIndex, IMedia *pMedia)
{
    {
        MutexAutolock lock(m_mutexDataAccess);

        pMedia->addRef();

        if (nIndex >= 0 && nIndex < (int)m_vMedia.size())
        {
            m_vMedia.insert(m_vMedia.begin() + nIndex, (CMedia*)pMedia);
        }
        else
        {
            m_vMedia.push_back((CMedia*)pMedia);
            nIndex = m_vMedia.size() - 1;
        }
    }

    m_pPlayer->notifyPlaylistChanged(this, IMPEvent::PCA_INSERT, nIndex, 0);

    return ERR_OK;
}

MLRESULT CPlaylist::moveItem(long nIndexOld, long nIndexNew)
{
    CMedia        *pTemp;

    m_mutexDataAccess.lock();

    assert(nIndexOld >= 0 && nIndexOld < (int)m_vMedia.size());
    assert(nIndexNew >= 0 && nIndexNew < (int)m_vMedia.size());

    if (nIndexOld >= 0 && nIndexOld < (int)m_vMedia.size() &&
        nIndexNew >= 0 && nIndexNew < (int)m_vMedia.size())
    {
        if (nIndexOld < nIndexNew)
        {
            pTemp = m_vMedia[nIndexOld];
            for (int i = nIndexOld; i < nIndexNew; i++)
            {
                m_vMedia[i] = m_vMedia[i + 1];
            }
            m_vMedia[nIndexNew] = pTemp;
        }
        else if (nIndexOld > nIndexNew)
        {
            pTemp = m_vMedia[nIndexOld];
            for (int i = nIndexOld; i > nIndexNew; i--)
            {
                m_vMedia[i] = m_vMedia[i - 1];
            }
            m_vMedia[nIndexNew] = pTemp;
        }

        m_mutexDataAccess.unlock();
        m_pPlayer->notifyPlaylistChanged(this, IMPEvent::PCA_MOVE, nIndexNew, nIndexOld);

        return ERR_OK;
    } else {
        m_mutexDataAccess.unlock();
        return ERR_NOT_FOUND;
    }
}

MLRESULT CPlaylist::removeItem(long nIndex)
{
    m_mutexDataAccess.lock();

    assert(nIndex >= 0 && nIndex < (int)m_vMedia.size());

    if (nIndex >= 0 && nIndex < (int)m_vMedia.size())
    {
        CMedia        *pTemp;
        pTemp = m_vMedia[nIndex];
        m_vMedia.erase(m_vMedia.begin() + nIndex);
        pTemp->release();

        m_mutexDataAccess.unlock();
        m_pPlayer->notifyPlaylistChanged(this, IMPEvent::PCA_REMOVE, nIndex, 0);
        return ERR_OK;
    } else {
        m_mutexDataAccess.unlock();
        return ERR_NOT_FOUND;
    }
}

MLRESULT CPlaylist::clear()
{
    {
        MutexAutolock autolock(m_mutexDataAccess);

        if (m_vMedia.size() == 0)
            return ERR_OK;

        for (uint32_t i = 0; i < m_vMedia.size(); i++)
        {
            CMedia        *pTemp;
            pTemp = m_vMedia[i];
            pTemp->release();
        }

        m_vMedia.clear();
    }

    m_pPlayer->notifyPlaylistChanged(this, IMPEvent::PCA_CLEAR, 0, 0);

    return ERR_OK;
}

MLRESULT CPlaylist::getItemIndex(IMedia *pMedia, long &nIndex)
{
    MutexAutolock    autolock(m_mutexDataAccess);

    nIndex = -1;

    if (m_vMedia.size() == 0)
        return ERR_NOT_FOUND;

    for (uint32_t i = 0; i < m_vMedia.size(); i++)
    {
        CMedia        *pTemp;
        pTemp = m_vMedia[i];
        if (pTemp == pMedia)
        {
            nIndex = i;
            return ERR_OK;
        }
    }

    return ERR_NOT_FOUND;
}
