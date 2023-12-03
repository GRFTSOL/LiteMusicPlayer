#pragma once

#include "Media.h"
#include "../Utils/Utils.h"


class CMPlayer;
class Playlist;

using PlaylistPtr = std::shared_ptr<Playlist>;
using VecPlaylistPtrs = std::vector<PlaylistPtr>;

struct PlaylistBrief {
    int                         id = -1;
    string                      name;
    int                         duration = 0; // 总时长，单位秒
    int                         count = 0;
    int                         rating = 0;
    bool                        isUpToDate = false; // 是否 duration, count 已经更新.
    int64_t                     timeModified = 0;
};
using VecPlaylistBriefs = vector<PlaylistBrief>;

/**
 * 存储在数据库中的 playlist 信息.
 */
struct PlaylistInfo : public PlaylistBrief {
    vector<int>                 mediaIds;

    PlaylistPtr                 playlist;

    void refreshTimeModified();

};

using VecPlaylistInfo = vector<PlaylistInfo>;

class Playlist : public std::enable_shared_from_this<Playlist> {
public:
    Playlist();
    Playlist(PlaylistInfo &info);
    virtual ~Playlist();

    void setInfo(const PlaylistInfo &info);

    uint32_t getCount();

    MediaPtr getItem(int nIndex);

    bool insertItem(int nIndex, const MediaPtr &media);
    bool insert(int position, const VecMediaPtrs &medias);
    bool insert(int position, Playlist *other) { return insert(position, other->getAll()); }
    bool append(Playlist *other) { return insert(-1, other); }
    void clone(Playlist *other);

    void moveItem(int nIndexOld, int nIndexNew);

    void removeItem(int nIndex);
    bool removeItems(const VecMediaPtrs &medias);

    void clear();

    MediaPtr getItemByID(int id, int *indexOut = nullptr);
    ResultCode getItemIndex(const MediaPtr &media, int &nIndex);

    VecMediaPtrs getAll() const { return m_vMedia; }

    PlaylistInfo toPlaylistInfo();
    void refreshTimeModified();

public:
    int                         id = -1;
    string                      name;
    int                         rating = 0;
    int64_t                     timeModified = 0;

protected:
    std::mutex                  m_mutexDataAccess;
    VecMediaPtrs                m_vMedia;

};
