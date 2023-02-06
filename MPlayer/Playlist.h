#pragma once

#include "Media.h"
#include "../Utils/Utils.h"


class CMPlayer;

class Playlist {
public:
    Playlist();
    virtual ~Playlist();

    uint32_t getCount();

    MediaPtr getItem(int nIndex);

    void insertItem(int nIndex, const MediaPtr &media);

    void moveItem(int nIndexOld, int nIndexNew);

    void removeItem(int nIndex);

    void clear();

    ResultCode getItemIndex(const MediaPtr &media, int &nIndex);

protected:
    std::mutex                  m_mutexDataAccess;
    VecMediaPtrs                m_vMedia;

};

using PlaylistPtr = std::shared_ptr<Playlist>;
