#pragma once

#ifndef MPlayerEngine_MediaLibrary_h
#define MPlayerEngine_MediaLibrary_h


#include "../third-parties/sqlite/Sqlite3.hpp"

#include "Playlist.h"


class CPlayer;

/**
 * 按照分类统计出媒体库的歌曲.
 *
 * 在搜索时，可以根据分类名来搜索歌曲.
 */
struct MediaCategory {
    enum Type {
        ALL,
        PLAYLIST,
        FOLDER,
        ARTIST,
        ALBUM,
        GENRE,
        _COUNT,
    };

    Type                        type;
    string                      name;
    string                      value;
    uint32_t                    mediaCount;     // 歌曲数量
    uint32_t                    mediaDuration;  // 总共时长，单位秒

    MediaCategory(Type type, cstr_t name, cstr_t value, uint32_t mediaCount = 0, uint32_t mediaDuration = 0) : type(type), name(name), value(value), mediaCount(mediaCount), mediaDuration(mediaDuration) {
    }

};

using VecMediaCategories = vector<MediaCategory>;

cstr_t mediaCategoryTypeToString(MediaCategory::Type type);

/**
 * 存储在数据库中的 playlist 信息.
 */
struct PlaylistInfo {
    int                         id;
    string                      name;
    int                         duration; // 总时长，单位秒
    int                         count;
    int                         rating;
    bool                        isUpToDate; // 是否 duration, count 已经更新.
    vector<int>                 mediaIds;

};

using VecPlaylistInfo = vector<PlaylistInfo>;

enum MediaLibOrderBy {
    MLOB_NONE,
    MLOB_ARTIST,
    MLOB_ALBUM,
    MLOB_ARTIST_ALBUM,
    MLOB_GENRE,
    MLOB_RATING,

};

class CMediaLibrary {
public:
    CMediaLibrary();
    virtual ~CMediaLibrary();

    void getMediaCategories(VecMediaCategories &categoriesOut);
    PlaylistPtr getMediaCategory(const MediaCategory &category);

    VecStrings getAllArtist();
    VecStrings getAllAlbum();
    VecStrings getAllGenre();
    VecStrings getAlbumOfArtist(cstr_t szArtist);

    uint32_t getMediaCount();

    MediaPtr getMediaByUrl(cstr_t szUrl);
    MediaPtr getMediaByID(int id);

    MediaPtr add(cstr_t szMediaUrl);

    // add media to media library fast, didn't update media info.
    MediaPtr addFast(cstr_t szMediaUrl);

    ResultCode updateMediaInfo(Media *media);

    // removes the specified item from the media library
    ResultCode remove(Media *media, bool bDeleteFile);

    // If the media file was removed temporarily, set this flag on.
    ResultCode setDeleted(Media *media);

    const PlaylistPtr &getAll();

    PlaylistPtr getAll(MediaLibOrderBy orderBy, int nTopN);

    PlaylistPtr getByArtist(cstr_t szArtist, MediaLibOrderBy orderBy, int nTopN);

    PlaylistPtr getByAlbum(cstr_t szAlbum, MediaLibOrderBy orderBy, int nTopN);

    PlaylistPtr getByAlbum(cstr_t szArtist, cstr_t szAlbum, MediaLibOrderBy orderBy, int nTopN);

    PlaylistPtr getByTitle(cstr_t szTitle);

    PlaylistPtr getByGenre(cstr_t szGenre, MediaLibOrderBy orderBy, int nTopN);

    PlaylistPtr getByYear(int nYear, MediaLibOrderBy orderBy, int nTopN);

    PlaylistPtr getByRating(int nRating, MediaLibOrderBy orderBy, int nTopN);

    PlaylistPtr getRecentPlayed(uint32_t nCount);

    PlaylistPtr getRecentPlayed(int nDayAgoBegin, int nDayAgoEnd);

    PlaylistPtr getTopPlayed(uint32_t nCount);

    PlaylistPtr getTopRating(uint32_t nCount);

    PlaylistPtr getRecentAdded(uint32_t nCount);

    // 0-5, 0 for unrate.
    ResultCode rate(Media *media, uint32_t nRating);

    ResultCode updatePlayedTime(Media *media);
    ResultCode markPlayFinished(Media *media);
    ResultCode markPlaySkipped(Media *media);

public:
    int init();
    void close();

    bool isOK() { return m_nInitResult == ERR_OK; }

    PlaylistPtr queryPlaylist(cstr_t szSQL, cstr_t szClause);

    VecStrings queryVStr(cstr_t szSql);

    ResultCode executeSQL(cstr_t szSql);

    void getMediaCallback(Media *media, int numCols, char **results);

    ResultCode undeleteMedia(long nMediaID);

    ResultCode add(const MediaPtr &media);

    string getSettingValue(cstr_t name);
    void setSettingValue(cstr_t name, cstr_t value);

    PlaylistPtr newPlaylist();

protected:
    ResultCode doAddMedia(const MediaPtr &media);

    void updateMediaInMem(Media *media);
    void removeMediaInMem(Media *media);

    int upgradeCheck();

    void loadPlaylists();
    int savePlaylist(const PlaylistInfo &playlist);
    PlaylistPtr getPlaylist(int playlistId);
    void deltePlaylist(int playlistId);
    void updateMediaCategories();

protected:
    friend class CMLQueryPlaylist;

    PlaylistPtr                 m_allMedias;

    VecPlaylistInfo             m_playlists;
    VecMediaCategories          m_mediaCategories;
    uint64_t                    m_timeMediaCategoryUpdated;

    CSqlite3                    m_db;
    std::recursive_mutex        m_mutexDataAccess;

    CSqlite3Stmt                m_sqlAdd, m_sqlAddFast, m_sqlQueryByUrl, m_stmtQueryByID;
    int                         m_nInitResult;

};

#endif // !defined(MPlayerEngine_MediaLibrary_h)
