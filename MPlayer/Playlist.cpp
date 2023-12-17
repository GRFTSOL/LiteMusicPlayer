#include "TinyJS/utils/Utils.h"
#include "Player.h"
#include "Playlist.h"


Playlist::Playlist() {
    timeModified = time(nullptr);
}

Playlist::Playlist(PlaylistInfo &info) {
    setInfo(info);
}

Playlist::~Playlist() {
}

void Playlist::setInfo(const PlaylistInfo &info) {
    id = info.id;
    name = info.name;
    rating = info.rating;
    timeModified = info.timeModified;
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

bool Playlist::insertItem(int nIndex, const MediaPtr &media) {
    {
        MutexAutolock lock(m_mutexDataAccess);

        for (auto &item : m_vMedia) {
            if (item->ID == media->ID) {
                return false;
            }
        }

        if (nIndex >= 0 && nIndex < (int)m_vMedia.size()) {
            m_vMedia.insert(m_vMedia.begin() + nIndex, media);
        } else {
            m_vMedia.push_back(media);
            nIndex = (int)m_vMedia.size() - 1;
        }
    }

    refreshTimeModified();
    g_player.notifyPlaylistChanged(this, IMPEvent::PCA_INSERT, nIndex, 0);
    return true;
}

bool Playlist::insert(int position, const VecMediaPtrs &medias) {
    if (position == -1) {
        position = (int)getCount();
    }

    bool modified = false;
    {
        MutexAutolock lock(m_mutexDataAccess);

        for (auto &item : medias) {
            if (item) {
                if (find_if(m_vMedia.begin(), m_vMedia.end(), [&item](const MediaPtr &media) {
                    return media->ID == item->ID;
                }) == m_vMedia.end()) {
                    m_vMedia.insert(m_vMedia.begin() + position++, item);
                    modified = true;
                }
            }
        }
    }

    if (modified) {
        refreshTimeModified();
        g_player.notifyPlaylistChanged(this, IMPEvent::PCA_FULL_UPDATE, 0, 0);
    }

    return modified;
}

void Playlist::clone(Playlist *other) {
    {
        auto medias = other->getAll();
        MutexAutolock lock(m_mutexDataAccess);
        m_vMedia = medias;

        // id = other->id;
        name = other->name;
        rating = other->rating;
        timeModified = other->timeModified;
    }

    g_player.notifyPlaylistChanged(this, IMPEvent::PCA_FULL_UPDATE, 0, 0);
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

        refreshTimeModified();
        m_mutexDataAccess.unlock();
        g_player.notifyPlaylistChanged(this, IMPEvent::PCA_MOVE, nIndexNew, nIndexOld);
    }
}

void Playlist::removeItem(int nIndex) {
    assert(nIndex >= 0 && nIndex < (int)m_vMedia.size());

    if (nIndex >= 0 && nIndex < (int)m_vMedia.size()) {
        m_mutexDataAccess.lock();
        m_vMedia.erase(m_vMedia.begin() + nIndex);

        refreshTimeModified();
        m_mutexDataAccess.unlock();
        g_player.notifyPlaylistChanged(this, IMPEvent::PCA_REMOVE, nIndex, 0);
    }
}

bool Playlist::removeItems(const VecMediaPtrs &medias) {
    bool removed = false;
    {
        MutexAutolock autolock(m_mutexDataAccess);

        for (auto &media : medias) {
            for (auto it = m_vMedia.begin(); it != m_vMedia.end(); ++it) {
                if ((*it)->ID == media->ID) {
                    m_vMedia.erase(it);
                    removed = true;
                    break;
                }
            }
        }
    }

    if (removed) {
        refreshTimeModified();
        g_player.notifyPlaylistChanged(this, IMPEvent::PCA_FULL_UPDATE, 0, 0);
    }

    return removed;
}

void Playlist::clear() {
    {
        MutexAutolock autolock(m_mutexDataAccess);
        m_vMedia.clear();
    }

    g_player.notifyPlaylistChanged(this, IMPEvent::PCA_CLEAR, 0, 0);
}

MediaPtr Playlist::getItemByID(int id, int *indexOut) {
    MutexAutolock autolock(m_mutexDataAccess);

    if (indexOut) {
        *indexOut = -1;
    }

    for (uint32_t i = 0; i < m_vMedia.size(); i++) {
        auto &media = m_vMedia[i];
        if (media->ID == id) {
            if (indexOut) {
                *indexOut = i;
            }
            return media;
        }
    }

    return nullptr;
}

ResultCode Playlist::getItemIndex(const MediaPtr &media, int &nIndex) {
    MutexAutolock autolock(m_mutexDataAccess);

    nIndex = -1;

    if (m_vMedia.size() == 0 || media == nullptr) {
        return ERR_NOT_FOUND;
    }

    for (uint32_t i = 0; i < m_vMedia.size(); i++) {
        auto &pTemp = m_vMedia[i];
        if (pTemp->ID == media->ID) {
            nIndex = i;
            return ERR_OK;
        }
    }

    return ERR_NOT_FOUND;
}

PlaylistInfo Playlist::toPlaylistInfo() {
    MutexAutolock autolock(m_mutexDataAccess);
    PlaylistInfo info;

    info.id = id;
    info.name = name;
    info.rating = rating;
    info.timeModified = timeModified;
    info.isUpToDate = true;
    info.duration = 0;
    info.count = 0;
    info.playlist = shared_from_this();

    for (auto &media : m_vMedia) {
        info.mediaIds.push_back(media->ID);
        if (media->duration == MEDIA_LENGTH_INVALID) {
            info.isUpToDate = false;
        } else {
            info.duration += (media->duration + 500) / 1000;
        }
        info.count++;
    }

    return info;
}

void Playlist::refreshTimeModified() {
    timeModified = time(nullptr);
}

void PlaylistInfo::refreshTimeModified() {
    timeModified = time(nullptr);
}
