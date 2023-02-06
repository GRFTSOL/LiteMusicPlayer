#include "Player.h"
#include "Playlist.h"


Playlist::Playlist() {
}

Playlist::~Playlist() {
}

uint32_t Playlist::getCount() {
    MutexAutolock autolock(m_mutexDataAccess);

    return (uint32_t)m_vMedia.size();
}

MediaPtr Playlist::getItem(int nIndex) {
    MutexAutolock autolock(m_mutexDataAccess);

    if (nIndex >= 0 && nIndex < (int)m_vMedia.size()) {
        return m_vMedia[nIndex];
    }

    return nullptr;
}

void Playlist::insertItem(int nIndex, const MediaPtr &media) {
    {
        MutexAutolock lock(m_mutexDataAccess);

        if (nIndex >= 0 && nIndex < (int)m_vMedia.size()) {
            m_vMedia.insert(m_vMedia.begin() + nIndex, media);
        } else {
            m_vMedia.push_back(media);
            nIndex = (int)m_vMedia.size() - 1;
        }
    }

    g_player.notifyPlaylistChanged(this, IMPEvent::PCA_INSERT, nIndex, 0);
}

void Playlist::moveItem(int nIndexOld, int nIndexNew) {
    assert(nIndexOld >= 0 && nIndexOld < (int)m_vMedia.size());
    assert(nIndexNew >= 0 && nIndexNew < (int)m_vMedia.size());

    if (nIndexOld >= 0 && nIndexOld < (int)m_vMedia.size() &&
        nIndexNew >= 0 && nIndexNew < (int)m_vMedia.size()) {
        m_mutexDataAccess.lock();
        if (nIndexOld < nIndexNew) {
            auto pTemp = m_vMedia[nIndexOld];
            for (int i = nIndexOld; i < nIndexNew; i++) {
                m_vMedia[i] = m_vMedia[i + 1];
            }
            m_vMedia[nIndexNew] = pTemp;
        } else if (nIndexOld > nIndexNew) {
            auto pTemp = m_vMedia[nIndexOld];
            for (int i = nIndexOld; i > nIndexNew; i--) {
                m_vMedia[i] = m_vMedia[i - 1];
            }
            m_vMedia[nIndexNew] = pTemp;
        }

        m_mutexDataAccess.unlock();
        g_player.notifyPlaylistChanged(this, IMPEvent::PCA_MOVE, nIndexNew, nIndexOld);
    }
}

void Playlist::removeItem(int nIndex) {
    assert(nIndex >= 0 && nIndex < (int)m_vMedia.size());

    if (nIndex >= 0 && nIndex < (int)m_vMedia.size()) {
        m_mutexDataAccess.lock();
        m_vMedia.erase(m_vMedia.begin() + nIndex);

        m_mutexDataAccess.unlock();
        g_player.notifyPlaylistChanged(this, IMPEvent::PCA_REMOVE, nIndex, 0);
    }
}

void Playlist::clear() {
    {
        MutexAutolock autolock(m_mutexDataAccess);
        m_vMedia.clear();
    }

    g_player.notifyPlaylistChanged(this, IMPEvent::PCA_CLEAR, 0, 0);
}

ResultCode Playlist::getItemIndex(const MediaPtr &media, int &nIndex) {
    MutexAutolock autolock(m_mutexDataAccess);

    nIndex = -1;

    if (m_vMedia.size() == 0) {
        return ERR_NOT_FOUND;
    }

    for (uint32_t i = 0; i < m_vMedia.size(); i++) {
        auto &pTemp = m_vMedia[i];
        if (pTemp == media) {
            nIndex = i;
            return ERR_OK;
        }
    }

    return ERR_NOT_FOUND;
}
