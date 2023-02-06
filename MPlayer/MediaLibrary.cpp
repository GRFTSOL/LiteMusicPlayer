#include "../Skin/Skin.h"
#include "MediaLibrary.h"
#include "Player.h"


#define ALBUM_ARTIST_SEP        "~@~"
#define ALBUM_ARTIST_SEP_LEN    3

#define MEDIA_LIB_VER_NAME  "version"
#define MEDIA_LIB_VER_CUR   "1"


#define SQL_CREATE_SETTINGS        "CREATE TABLE IF NOT EXISTS settings\
(\
    name text Primary Key,\
    value text\
);"

#define SQL_SETTINGS_INSERT        "INSERT INTO settings VALUES (?, ?) "
#define SQL_SETTINGS_QUERY         "SELECT value FROM settings WHERE name=?"
#define SQL_SETTINGS_SET           "INSERT OR REPLACE INTO settings(name, value) VALUES(?, ?);"

#define SQL_CREATE_MEDIALIB        "CREATE TABLE IF NOT EXISTS medialib    \
(\
  id integer Primary Key,\
  url text,\
  file_deleted integer DEFAULT 0,\
  info_updated integer DEFAULT 0,\
  is_user_rating integer DEFAULT 0,\
  artist text COLLATE NOCASE,\
  album text COLLATE NOCASE,\
  title text COLLATE NOCASE,\
  track integer DEFAULT -1,\
  year integer DEFAULT -1,\
  genre text DEFAULT NULL COLLATE NOCASE,\
  comment text DEFAULT NULL COLLATE NOCASE,\
  length integer DEFAULT 0,\
  filesize integer DEFAULT 0,\
  time_added integer,\
  time_played integer DEFAULT 0,\
  rating integer DEFAULT 300,\
  times_played integer DEFAULT 0,\
  times_play_skipped integer DEFAULT 0,\
  music_hash text DEFAULT NULL,\
  lyrics_file text DEFAULT NULL\
);\
\
CREATE INDEX IF NOT EXISTS mli_url on medialib (url);\
CREATE INDEX IF NOT EXISTS mli_artist on medialib (artist);\
CREATE INDEX IF NOT EXISTS mli_album on medialib (album);\
CREATE INDEX IF NOT EXISTS mli_title on medialib (title);\
CREATE INDEX IF NOT EXISTS mli_genre on medialib (genre);\
CREATE INDEX IF NOT EXISTS mli_comment on medialib (comment);\
CREATE INDEX IF NOT EXISTS mli_time_added on medialib (time_added);\
CREATE INDEX IF NOT EXISTS mli_time_played on medialib (time_played);\
CREATE INDEX IF NOT EXISTS mli_rating on medialib (rating);\
CREATE INDEX IF NOT EXISTS mli_music_hash on medialib (music_hash);\
CREATE INDEX IF NOT EXISTS mli_times_played on medialib (times_played);"

#define DROP_MEDIALIB_TABLE "DROP TABLE IF EXISTS medialib;\
DROP INDEX IF EXISTS mli_url;\
DROP INDEX IF EXISTS mli_artist;\
DROP INDEX IF EXISTS mli_album;\
DROP INDEX IF EXISTS mli_title;\
DROP INDEX IF EXISTS mli_genre;\
DROP INDEX IF EXISTS mli_comment;\
DROP INDEX IF EXISTS mli_time_added;\
DROP INDEX IF EXISTS mli_time_played;\
DROP INDEX IF EXISTS mli_rating;\
DROP INDEX IF EXISTS mli_music_hash;\
DROP INDEX IF EXISTS mli_times_played;"

#define SQL_MAX_MEDIALIB_ID   "select MAX(id)  from medialib"

#define SQL_COUNT_OF_MEDIA    "select count(*)  from medialib"

#define SQL_ADD_MEDIA    "INSERT INTO medialib (url, artist, album, title, track, year, genre, comment, length, filesize, time_added, music_hash, lyrics_file) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"

#define SQL_ADD_MEDIA_FAST    "INSERT INTO medialib (url, title, time_added) "\
                              "VALUES (?, ?, ?);"

#define SQL_QUERY_MEDIA_BY_URL  "select * from medialib where url=? "
#define SQL_QUERY_MEDIA_BY_ID   "select * from medialib where id=? "


#define SQL_CREATE_TABLE_PLAYLIST   "CREATE TABLE IF NOT EXISTS playlists \
(\
  id integer Primary Key,\
  name text,\
  duration integer DEFAULT 0,\
  count integer DEFAULT 0,\
  rating integer DEFAULT 300,\
  is_up_to_date integer DEFAULT 0,\
  media_ids text DEFAULT NULL\
);"

#define SQL_QUERY_ALL_PLAYLISTS     "SELECT * FROM playlists"
#define SQL_ADD_PLAYLIST            "INSERT INTO playlists (name, duration, count, rating, is_up_to_date, media_ids) VALUES ('?, ?, ?, ?, ?, ?"
#define SQL_UPDATE_PLAYLIST_BY_ID   "UPDATE playlists SET name=?, duration=?, count=?, rating=?, is_up_to_date=?, media_ids=?"
#define SQL_DELETE_PLAYLIST_BY_ID   "DELETE FROM playlists WHERE id=?"

#define SQLITE3_BIND_TEXT(sqlstmt, strData)            \
    ret = sqlstmt.bindText(n, strData.c_str(), (int)strData.size());\
    if (ret != ERR_OK)\
        goto RET_FAILED;\
    n++;

#define  MediaGetSqlite3ColumText(mediaAttri)    \
    text = (const char *)sqlStmt.columnText(n++);    \
    if (text)\
        mediaAttri = text;\
    else\
        mediaAttri.resize(0);

MediaPtr newMedia() {
    return make_shared<Media>();
}

cstr_t mediaCategoryTypeToString(MediaCategory::Type type) {
    static cstr_t NAMES[] = { _TLM("All Musics"), _TLM("Playlist"), _TLM("Folder"), _TLM("Artist"), _TLM("Album"), _TLM("Genre") };
    assert(CountOf(NAMES) == MediaCategory::_COUNT);
    assert(type >= 0 && type <= MediaCategory::_COUNT);

    return NAMES[type];
}

MediaPtr sqliteQueryMedia(CSqlite3Stmt &sqlStmt) {
    int n = 0;
    const char *text;
    string str;

    auto media = make_shared<Media>();

    media->ID = sqlStmt.columnInt(n++);

    MediaGetSqlite3ColumText(media->url);
    media->isFileDeleted = tobool(sqlStmt.columnInt(n++));
    media->infoUpdated = tobool(sqlStmt.columnInt(n++));
    media->isUserRating = tobool(sqlStmt.columnInt(n++));
    MediaGetSqlite3ColumText(media->artist);
    MediaGetSqlite3ColumText(media->album);
    MediaGetSqlite3ColumText(media->title);
    media->trackNumb = sqlStmt.columnInt(n++);
    media->year = sqlStmt.columnInt(n++);
    MediaGetSqlite3ColumText(media->genre);
    MediaGetSqlite3ColumText(media->comments);
    media->duration = sqlStmt.columnInt(n++);
    media->fileSize = sqlStmt.columnInt(n++);
    media->timeAdded = sqlStmt.columnInt64(n++);
    media->timePlayed = sqlStmt.columnInt64(n++);
    media->rating = sqlStmt.columnInt(n++) / 100;
    media->countPlayed = sqlStmt.columnInt(n++);
    media->countPlaySkipped = sqlStmt.columnInt(n++);
    MediaGetSqlite3ColumText(media->musicHash);
    MediaGetSqlite3ColumText(media->lyricsFile);

    return media;
}

PlaylistPtr sqliteQueryPlaylist(CMediaLibrary *library, CSqlite3Stmt &stmt) {
    auto playlist = library->newPlaylist();

    int ret = stmt.step();
    while (ret == ERR_SL_OK_ROW) {
        auto media = sqliteQueryMedia(stmt);
        playlist->insertItem(-1, media);

        ret = stmt.step();
    }

    stmt.reset();
    return playlist;
}

/*
How to calculate auto rating value:

default auto rating: 300
max auto rating: 590


*/
int getAutoRating(Media *media) {
    int64_t length = media->duration, rating = 300;
    int64_t times_play_skipped = media->countPlaySkipped, times_played = media->countPlayed;

    if (length > 30 * 1000) { // x seconds
        times_play_skipped -= 1;

        if (times_play_skipped < 0) {
            times_play_skipped = 0;
        }

        int64_t value = times_played - times_play_skipped * 5;
        if (value > 0) {
            rating = 400 + value;
        } else if (value < 0) {
            rating = 300 + value;
        }

        if (rating > 590) {
            rating = 589;
        } else if (rating <= 100) {
            rating = 101;
        }
    }

    return (int)rating;
}

void appendQeuryStatment(MediaLibOrderBy orderBy, int nTopN, string &strSql) {
    if (orderBy == MLOB_NONE) {
    } else if (orderBy == MLOB_ARTIST) {
        strSql += " order by artist";
    } else if (orderBy == MLOB_ALBUM) {
        strSql += " order by album";
    } else if (orderBy == MLOB_GENRE) {
        strSql += " order by genre";
    } else if (orderBy == MLOB_ARTIST_ALBUM) {
        strSql += " order by artist, album";
    } else if (orderBy == MLOB_RATING) {
        strSql += " order by rating desc";
    }

    if (nTopN != -1) {
        strSql += " limit";
        strSql += itos(nTopN);
    }
}

//////////////////////////////////////////////////////////////////////////

class CMLQueryPlaylist {
public:
    CMLQueryPlaylist() {
        m_pPlaylist = nullptr;
        m_mediaLib = nullptr;
    }

    PlaylistPtr                 m_pPlaylist;
    CMediaLibrary               *m_mediaLib;

    PlaylistPtr query(CMediaLibrary *pMediaLib, cstr_t szSQL) {
        if (!pMediaLib->isOK()) {
            return nullptr;
        }

        m_mediaLib = pMediaLib;

        m_pPlaylist = m_mediaLib->newPlaylist();

        int ret = sqlite3_exec(
            m_mediaLib->m_db.m_db,
            szSQL,
            &queryCallback,
            this,
            nullptr);
        if (ret != SQLITE_OK) {
            ERR_LOG2("Exe SQL:%S, : %S", szSQL, m_mediaLib->m_db.errorMsg());
            return nullptr;
        }

        return m_pPlaylist;
    }

    static int queryCallback(void *args, int numCols, char **results, char ** columnNames) {
        CMLQueryPlaylist *pThis = (CMLQueryPlaylist*)args;

        MediaPtr media = newMedia();

        pThis->m_mediaLib->getMediaCallback(media.get(), numCols, results);

        pThis->m_pPlaylist->insertItem(-1, media);

        return 0;
    }

};


CMediaLibrary::CMediaLibrary() {
}

CMediaLibrary::~CMediaLibrary() {
    close();
}

void CMediaLibrary::getMediaCategories(VecMediaCategories &categoriesOut) {
    updateMediaCategories();

    categoriesOut = m_mediaCategories;
}

PlaylistPtr CMediaLibrary::getMediaCategory(const MediaCategory &category) {
    if (category.type == MediaCategory::ALL) {
        return m_allMedias;
    } else if (category.type == MediaCategory::PLAYLIST) {
        return getPlaylist(atoi(category.value.c_str()));
    }

    int count = m_allMedias->getCount();
    auto pl = newPlaylist();

    switch (category.type) {
        case MediaCategory::FOLDER: {
            for (int i = 0; i < count; i++) {
                auto media = m_allMedias->getItem(i);
                auto &url = media->url;
                if (startsWith(url.c_str(), category.value.c_str())) {
                    pl->insertItem(-1, media);
                }
            }
            break;
        }
        case MediaCategory::ARTIST: {
            for (int i = 0; i < count; i++) {
                auto media = m_allMedias->getItem(i);
                if (category.value == media->artist.c_str()) {
                    pl->insertItem(-1, media);
                }
            }
            break;
        }
        case MediaCategory::ALBUM: {
            for (int i = 0; i < count; i++) {
                auto media = m_allMedias->getItem(i);
                auto str = media->artist;
                if (startsWith(category.value.c_str(), str.c_str())) {
                    SizedString key(category.value.c_str(), category.value.size());
                    key.shrink((int)str.size());
                    if (key.startsWith(ALBUM_ARTIST_SEP)) {
                        key.shrink(ALBUM_ARTIST_SEP_LEN);
                        if (key.equal(media->album.c_str())) {
                            pl->insertItem(-1, media);
                        }
                    }
                }
            }
            break;
        }
        case MediaCategory::GENRE: {
            for (int i = 0; i < count; i++) {
                auto media = m_allMedias->getItem(i);
                if (category.value == media->genre) {
                    pl->insertItem(-1, media);
                }
            }
            break;
        }
        default:
            assert(0);
            break;
    }

    return pl;
}

VecStrings CMediaLibrary::getAllArtist() {
    return queryVStr("select artist from medialib group by artist");
}

VecStrings CMediaLibrary::getAllAlbum() {
    return queryVStr("select album from medialib group by album");
}

VecStrings CMediaLibrary::getAllGenre() {
    return queryVStr("select genre from medialib group by genre");
}

VecStrings CMediaLibrary::getAlbumOfArtist(cstr_t szArtist) {
    VecStrings results;

    if (!isOK()) {
        return results;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    CSqlite3Stmt sqlQuery;

    int ret = sqlQuery.prepare(&m_db, "select album from medialib where artist=? group by album");
    if (ret != ERR_OK) {
        return results;
    }

    sqlQuery.bindStaticText(1, szArtist);

    ret = sqlQuery.step();
    while (ret == ERR_SL_OK_ROW) {
        auto text = sqlQuery.columnText(0);
        if (text) {
            results.push_back(text);
        }

        ret = sqlQuery.step();
    }

    return results;
}

uint32_t CMediaLibrary::getMediaCount() {
    return sqlite_query_int_value(m_db.m_db, SQL_COUNT_OF_MEDIA);
}

MediaPtr CMediaLibrary::getMediaByUrl(cstr_t szUrl) {
    if (!isOK()) {
        return nullptr;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    m_sqlQueryByUrl.bindStaticText(1, szUrl);

    MediaPtr media;
    if (m_sqlQueryByUrl.step() == ERR_SL_OK_ROW) {
        media = sqliteQueryMedia(m_sqlQueryByUrl);

        if (media->isFileDeleted) {
            // recover to undeleted.
            if (undeleteMedia(media->ID) == ERR_OK) {
                media->isFileDeleted = false;
            }
        }
    }

    m_sqlQueryByUrl.reset();

    return media;
}

MediaPtr CMediaLibrary::getMediaByID(int id) {
    if (!isOK()) {
        return nullptr;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    m_stmtQueryByID.bindInt(1, id);

    MediaPtr media;
    int ret = m_stmtQueryByID.step();
    if (ret == ERR_SL_OK_ROW) {
        media = sqliteQueryMedia(m_stmtQueryByID);
        if (!media->isFileDeleted) {
            media = nullptr;
        }
    }

    m_stmtQueryByID.reset();

    return media;
}

MediaPtr CMediaLibrary::add(cstr_t mediaUrl) {
    if (!isOK()) {
        return nullptr;
    }

    auto media = getMediaByUrl(mediaUrl);
    if (media) {
        return media;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    media = newMedia();

    media->url = mediaUrl;

    g_player.loadMediaTagInfo(media.get());

    media->timePlayed = time(nullptr);
    media->timeAdded = time(nullptr);

    doAddMedia(media);

    return media;
}

// add media to media library fast, won't update media info.
MediaPtr CMediaLibrary::addFast(cstr_t mediaUrl) {
    if (!isOK()) {
        return nullptr;
    }

    auto media = getMediaByUrl(mediaUrl);
    if (media) {
        return media;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    media = newMedia();

    media->url = mediaUrl;
    media->timePlayed = time(nullptr);
    media->timeAdded = time(nullptr);

    media->title = urlGetTitle(mediaUrl);

    int n = 1;
    int ret = m_sqlAddFast.bindText(n++, mediaUrl);
    if (ret != ERR_OK) {
        goto RET_FAILED;
    }

    SQLITE3_BIND_TEXT(m_sqlAddFast, media->title);
    m_sqlAddFast.bindInt64(n++, media->timeAdded);

    ret = m_sqlAddFast.step();
    if (ret != ERR_OK) {
        goto RET_FAILED;
    }

    media->ID = m_db.LastInsertRowId();

    m_sqlAddFast.reset();

    m_allMedias->insertItem(-1, media);

    return media;

RET_FAILED:

    return nullptr;
}

// update Media Info to DB
ResultCode CMediaLibrary::updateMediaInfo(Media *media) {
    if (!isOK()) {
        return m_nInitResult;
    }

    assert(media->ID != MEDIA_ID_INVALID);
    if (media->ID == MEDIA_ID_INVALID) {
        return ERR_NOT_FOUND;
    }

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;
    int ret;
    int n = 1;
    int64_t value;

    sqlQuery.prepare(&m_db, "update medialib set url=?, info_updated=1, artist=?, album=?, title=?, track=?,"\
        "year=?, genre=?, comment=?, length=?, filesize=?, lyrics_file=? where id=?");

    SQLITE3_BIND_TEXT(sqlQuery, media->url);
    SQLITE3_BIND_TEXT(sqlQuery, media->artist);
    SQLITE3_BIND_TEXT(sqlQuery, media->album);
    SQLITE3_BIND_TEXT(sqlQuery, media->title);

    sqlQuery.bindInt(n++, (int)media->trackNumb);
    sqlQuery.bindInt(n++, (int)media->year);

    SQLITE3_BIND_TEXT(sqlQuery, media->genre);
    SQLITE3_BIND_TEXT(sqlQuery, media->comments);

    sqlQuery.bindInt(n++, (int)media->duration);
    sqlQuery.bindInt(n++, (int)media->fileSize);
    SQLITE3_BIND_TEXT(sqlQuery, media->lyricsFile);

    sqlQuery.bindInt(n++, (int)media->ID);
    ret = sqlQuery.step();
    if (ret == ERR_OK) {
        media->infoUpdated = true;
    }

    updateMediaInMem(media);

RET_FAILED:
    return ret;
}

// removes the specified item from the media library
ResultCode CMediaLibrary::remove(Media *media, bool bDeleteFile) {
    if (!isOK()) {
        return m_nInitResult;
    }

    removeMediaInMem(media);

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    sqlQuery.prepare(&m_db, "delete from medialib where id=?");

    sqlQuery.bindInt(1, media->ID);
    int ret = sqlQuery.step();
    if (ret == ERR_OK) {
        if (bDeleteFile) {
            deleteFile(media->url.c_str());
        }
    }

    return ERR_OK;
}

// If the media file was removed temporarily, set this flag on.
ResultCode CMediaLibrary::setDeleted(Media *media) {
    char szSql[256];

    snprintf(szSql, CountOf(szSql), "update medialib set file_deleted=1 where id=%d", media->ID);

    removeMediaInMem(media);

    return executeSQL(szSql);
}

const PlaylistPtr &CMediaLibrary::getAll() {
    return m_allMedias;
}

PlaylistPtr CMediaLibrary::getAll(MediaLibOrderBy orderBy, int nTopN) {
    string strSql = "select * from medialib where file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    CMLQueryPlaylist query;
    return query.query(this, strSql.c_str());
}

PlaylistPtr CMediaLibrary::getByArtist(cstr_t szArtist, MediaLibOrderBy orderBy, int nTopN) {
    string strSql = "select * from medialib where artist=? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    return queryPlaylist(strSql.c_str(), szArtist);
}

PlaylistPtr CMediaLibrary::getByAlbum(cstr_t szAlbum, MediaLibOrderBy orderBy, int nTopN) {
    string strSql = "select * from medialib where album=? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    return queryPlaylist(strSql.c_str(), szAlbum);
}

PlaylistPtr CMediaLibrary::getByAlbum(cstr_t szArtist, cstr_t szAlbum, MediaLibOrderBy orderBy, int nTopN) {
    CMLQueryPlaylist query;
    string strArtist = szArtist, strAlbum = szAlbum;

    strrep(strArtist, "'", "''");
    strrep(strAlbum, "'", "''");

    string strSql = "select * from medialib where artist='";
    strSql += strArtist.c_str();
    strSql += "' and album='";
    strSql += strAlbum.c_str();
    strSql += "' and file_deleted!=1";

    appendQeuryStatment(orderBy, nTopN, strSql);

    return query.query(this, strSql.c_str());
}

PlaylistPtr CMediaLibrary::getByTitle(cstr_t szTitle) {
    return queryPlaylist("select * from medialib where title=? and file_deleted!=1", szTitle);
}

PlaylistPtr CMediaLibrary::getByGenre(cstr_t szGenre, MediaLibOrderBy orderBy, int nTopN) {
    string strSql;

    strSql = "select * from medialib where genre=? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    return queryPlaylist(strSql.c_str(), szGenre);
}

PlaylistPtr CMediaLibrary::getByYear(int nYear, MediaLibOrderBy orderBy, int nTopN) {
    if (!isOK()) {
        return nullptr;
    }

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    string strSql = "select * from medialib where year=? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    int ret = sqlQuery.prepare(&m_db, strSql.c_str());
    if (ret != ERR_OK) {
        return nullptr;
    }

    sqlQuery.bindInt(1, nYear);

    return sqliteQueryPlaylist(this, sqlQuery);
}

PlaylistPtr CMediaLibrary::getByRating(int nRating, MediaLibOrderBy orderBy, int nTopN) {
    if (!isOK()) {
        return nullptr;
    }

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    string strSql = "select * from medialib where rating>=? and rating<? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    int nRatingStart = nRating * 100;
    int nRatingEnd = (nRating + 1) * 100 - 1;

    int ret = sqlQuery.prepare(&m_db, strSql.c_str());
    if (ret != ERR_OK) {
        return nullptr;
    }

    sqlQuery.bindInt(1, nRatingStart);
    sqlQuery.bindInt(2, nRatingEnd);

    return sqliteQueryPlaylist(this, sqlQuery);
}

PlaylistPtr CMediaLibrary::getRecentPlayed(uint32_t nCount) {
    CMLQueryPlaylist query;
    char szQuery[256];

    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted!=1 order by time_played desc limit %d", nCount);

    return query.query(this, szQuery);
}

PlaylistPtr CMediaLibrary::getTopPlayed(uint32_t nCount) {
    char szQuery[256];
    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted!=1 order by times_played desc limit %d", nCount);

    CMLQueryPlaylist query;
    return query.query(this, szQuery);
}

PlaylistPtr CMediaLibrary::getTopRating(uint32_t nCount) {
    char szQuery[256];
    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted!=1 order by rating desc limit %d", nCount);

    CMLQueryPlaylist query;
    return query.query(this, szQuery);
}

PlaylistPtr CMediaLibrary::getRecentAdded(uint32_t nCount) {
    char szQuery[256];
    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted!=1 order by time_added desc limit %d", nCount);

    CMLQueryPlaylist query;
    return query.query(this, szQuery);
}

PlaylistPtr CMediaLibrary::getRecentPlayed(int nDayAgoBegin, int nDayAgoEnd) {
    char szQuery[256];

    auto begin = time(nullptr) - DateTime::SECOND_IN_ONE_DAY * nDayAgoBegin;
    auto end = time(nullptr) - DateTime::SECOND_IN_ONE_DAY * nDayAgoEnd;

    snprintf(szQuery, CountOf(szQuery), "select * from medialib where time_played between %ld and %ld and file_deleted!=1 order by time_played", begin, end);

    CMLQueryPlaylist query;
    return query.query(this, szQuery);
}

// 0-5, 0 for unrate.
ResultCode CMediaLibrary::rate(Media *media, uint32_t nRating) {
    if (!isOK()) {
        return m_nInitResult;
    }

    if (!media) {
        return ERR_OK;
    }

    bool bIsUserRating = (nRating != 0);
    if (bIsUserRating) {
        nRating = nRating * 100 + 99;
    } else {
        nRating = getAutoRating(media);
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    char szSql[256];
    snprintf(szSql, CountOf(szSql), "update medialib set is_user_rating=%d, rating=%d where id=%d",
        bIsUserRating, nRating, (int)media->ID);

    media->rating = nRating / 100;
    media->isUserRating = bIsUserRating;

    return executeSQL(szSql);
}

ResultCode CMediaLibrary::updatePlayedTime(Media *media) {
    if (!isOK()) {
        return m_nInitResult;
    }

    if (!media || media->ID == MEDIA_ID_INVALID) {
        return ERR_OK;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    char szSql[256];
    snprintf(szSql, CountOf(szSql), "update medialib set time_played=%lld where id=%d", (int64_t)media->timePlayed, (int)media->ID);

    return executeSQL(szSql);
}

ResultCode CMediaLibrary::markPlayFinished(Media *media) {
    if (!isOK()) {
        return m_nInitResult;
    }

    if (!media || media->ID == MEDIA_ID_INVALID) {
        return ERR_OK;
    }

    char szSql[256];

    RMutexAutolock autolock(m_mutexDataAccess);

    media->countPlayed++;

    if (media->isUserRating) {
        snprintf(szSql, CountOf(szSql), "update medialib set times_played=times_played+1 where id=%d", (int)media->ID);
    } else {
        int nRating = getAutoRating(media);
        snprintf(szSql, CountOf(szSql), "update medialib set times_played=times_played+1, rating=%d where id=%d",
            nRating, (int)media->ID);
        media->rating = nRating / 100;
    }

    int ret = executeSQL(szSql);

    updatePlayedTime(media);

    return ret;
}

ResultCode CMediaLibrary::markPlaySkipped(Media *media) {
    if (!isOK()) {
        return m_nInitResult;
    }

    if (!media || media->ID == MEDIA_ID_INVALID) {
        return ERR_OK;
    }

    char szSql[256];

    RMutexAutolock autolock(m_mutexDataAccess);

    media->countPlaySkipped++;

    if (media->isUserRating) {
        snprintf(szSql, CountOf(szSql), "update medialib set times_play_skipped=times_play_skipped+1 where id=%d", (int)media->ID);
    } else {
        int nRating = getAutoRating(media);
        snprintf(szSql, CountOf(szSql), "update medialib set times_play_skipped=times_play_skipped+1, rating=%d where id=%d",
            nRating, (int)media->ID);
        media->rating = nRating / 100;
    }

    int ret = executeSQL(szSql);

    return ret;
}

int CMediaLibrary::init() {
    string strFile = getAppDataDir() + "medialib.db";

    m_nInitResult = m_db.open(strFile.c_str());
    if (m_nInitResult != ERR_OK) {
        goto INIT_FAILED;
    }

    // 检查 MediaLibrary 数据的版本，并且进行升级
    m_nInitResult = upgradeCheck();
    if (m_nInitResult != ERR_OK) {
        goto INIT_FAILED;
    }

    // create medialib talbe
    m_nInitResult = m_db.exec(SQL_CREATE_MEDIALIB);
    if (m_nInitResult != ERR_OK) {
        return m_nInitResult;
    }

    m_nInitResult = m_sqlAdd.prepare(&m_db, SQL_ADD_MEDIA);
    assert(m_nInitResult == ERR_OK);

    m_nInitResult = m_sqlAddFast.prepare(&m_db, SQL_ADD_MEDIA_FAST);
    assert(m_nInitResult == ERR_OK);

    m_nInitResult = m_sqlQueryByUrl.prepare(&m_db, SQL_QUERY_MEDIA_BY_URL);
    assert(m_nInitResult == ERR_OK);

    m_nInitResult = m_stmtQueryByID.prepare(&m_db, SQL_QUERY_MEDIA_BY_ID);
    assert(m_nInitResult == ERR_OK);

    // 创建 playlists 的 table
    m_nInitResult = m_db.exec(SQL_CREATE_TABLE_PLAYLIST);
    if (m_nInitResult != ERR_OK) {
        return m_nInitResult;
    }

    loadPlaylists();

    m_allMedias = getAll(MLOB_ARTIST, -1);
    if (m_nInitResult != ERR_OK) {
        goto INIT_FAILED;
    }

    return m_nInitResult;

INIT_FAILED:
    close();
    return m_nInitResult;
}

void CMediaLibrary::close() {
    m_sqlAdd.finalize();
    m_sqlAddFast.finalize();
    m_sqlQueryByUrl.finalize();
    m_stmtQueryByID.finalize();
    m_db.close();
}

PlaylistPtr CMediaLibrary::queryPlaylist(cstr_t szSQL, cstr_t szClause) {
    if (!isOK()) {
        return nullptr;
    }

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    int ret = sqlQuery.prepare(&m_db, szSQL);
    if (ret != ERR_OK) {
        return nullptr;
    }

    ret = sqlQuery.bindText(1, szClause);
    if (ret != ERR_OK) {
        return nullptr;
    }

    return sqliteQueryPlaylist(this, sqlQuery);
}

VecStrings CMediaLibrary::queryVStr(cstr_t szSql) {
    VecStrings results;

    if (!isOK()) {
        return results;
    }

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    sqlQuery.prepare(&m_db, szSql);

    int ret = sqlQuery.step();
    while (ret == ERR_SL_OK_ROW) {
        auto text = (const char *)sqlQuery.columnText(0);
        if (text) {
            results.push_back(text);
        }

        ret = sqlQuery.step();
    }

    return results;
}

ResultCode CMediaLibrary::executeSQL(cstr_t szSql) {
    return m_db.exec(szSql);
}

void CMediaLibrary::getMediaCallback(Media *media, int numCols, char **results) {
    int n = 0;

#define FILED_GET_INT(feild)    \
    if (results[n])\
        feild = atoi(results[n]);\
    else\
        feild = false;\
    n++;

#define FILED_GET_INT64(feild)    \
    if (results[n])\
        feild = atol(results[n]);\
    else\
        feild = false;\
    n++;

#define FILED_GET_BOOL(feild)    \
    if (results[n])\
        feild = tobool(atoi(results[n]));\
    else\
        feild = false;\
    n++;

#define FILED_GET_TEXT(feild)    \
    if (results[n])\
        feild = results[n];\
    else\
        feild.resize(0);\
    n++;

    media->ID = atoi(results[n++]);

    FILED_GET_TEXT(media->url);

    FILED_GET_BOOL(media->isFileDeleted);
    FILED_GET_BOOL(media->infoUpdated);
    FILED_GET_BOOL(media->isUserRating);

    FILED_GET_TEXT(media->artist);
    FILED_GET_TEXT(media->album);
    FILED_GET_TEXT(media->title);
    FILED_GET_INT(media->trackNumb);
    FILED_GET_INT(media->year);

    FILED_GET_TEXT(media->genre);
    FILED_GET_TEXT(media->comments);
    FILED_GET_INT(media->duration);
    FILED_GET_INT(media->fileSize);
    FILED_GET_INT64(media->timeAdded);
    FILED_GET_INT64(media->timePlayed);
    FILED_GET_INT(media->rating);
    media->rating /= 100;
    FILED_GET_INT(media->countPlayed);
    FILED_GET_INT(media->countPlaySkipped);
    FILED_GET_TEXT(media->lyricsFile);

    assert(n <= numCols);
}

ResultCode CMediaLibrary::undeleteMedia(long nMediaID) {
    char szSql[256];

    snprintf(szSql, CountOf(szSql), "update medialib set file_deleted!=1 where id=%d", (int)nMediaID);

    return executeSQL(szSql);
}

ResultCode CMediaLibrary::add(const MediaPtr &media) {
    if (!isOK()) {
        return m_nInitResult;
    }

    auto existedMedia = getMediaByUrl(media->url.c_str());
    if (existedMedia) {
        return ERR_EXIST;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    return doAddMedia(media);
}

string CMediaLibrary::getSettingValue(cstr_t name) {
    CSqlite3Stmt stmtQuery;
    auto ret = stmtQuery.prepare(&m_db, SQL_SETTINGS_QUERY);
    assert(ret == ERR_OK);

    ret = stmtQuery.bindStaticText(1, name);
    assert(ret == ERR_OK);

    ret = stmtQuery.step();
    if (ret == ERR_SL_OK_ROW) {
        return stmtQuery.columnText(0);
    }
    assert(ret == ERR_OK);

    return "";
}

void CMediaLibrary::setSettingValue(cstr_t name, cstr_t value) {
    CSqlite3Stmt stmtQuery;
    auto ret = stmtQuery.prepare(&m_db, SQL_SETTINGS_SET);
    assert(ret == ERR_OK);

    ret = stmtQuery.bindStaticText(1, name);
    assert(ret == ERR_OK);

    ret = stmtQuery.bindStaticText(2, value);
    assert(ret == ERR_OK);

    ret = stmtQuery.step();
    assert(ret == ERR_OK);
}

ResultCode CMediaLibrary::doAddMedia(const MediaPtr &media) {
    int ret;
    int n = 1;

    // url, artist, album, title, track, year, genre, comment, length, filesize, time_added, music_hash, lyrics_file
    SQLITE3_BIND_TEXT(m_sqlAdd, media->url);
    SQLITE3_BIND_TEXT(m_sqlAdd, media->artist);
    SQLITE3_BIND_TEXT(m_sqlAdd, media->album);
    SQLITE3_BIND_TEXT(m_sqlAdd, media->title);
    m_sqlAdd.bindInt(n++, media->trackNumb);
    m_sqlAdd.bindInt(n++, media->year);
    SQLITE3_BIND_TEXT(m_sqlAdd, media->genre);
    SQLITE3_BIND_TEXT(m_sqlAdd, media->comments);
    m_sqlAdd.bindInt(n++, (int)media->duration);
    m_sqlAdd.bindInt(n++, (int)media->fileSize);
    m_sqlAdd.bindInt64(n++, media->timeAdded);
    SQLITE3_BIND_TEXT(m_sqlAdd, media->musicHash);
    SQLITE3_BIND_TEXT(m_sqlAdd, media->lyricsFile);

    ret = m_sqlAdd.step();
    if (ret == ERR_OK) {
        media->ID = m_db.LastInsertRowId();
        media->infoUpdated = true;
    }

    m_sqlAdd.reset();

    m_allMedias->insertItem(-1, media);

RET_FAILED:

    return ret;
}

PlaylistPtr CMediaLibrary::newPlaylist() {
    return make_shared<Playlist>();
}

int CMediaLibrary::upgradeCheck() {
    // create version table.
    auto ret = m_db.exec(SQL_CREATE_SETTINGS);
    if (ret != ERR_OK) {
        return ret;
    }

    string version = getSettingValue(MEDIA_LIB_VER_NAME);
    if (version == MEDIA_LIB_VER_CUR) {
        return ERR_OK;
    }

    // 版本不一致，需要升级
    setSettingValue(MEDIA_LIB_VER_NAME, MEDIA_LIB_VER_CUR);

    ret = m_db.exec(DROP_MEDIALIB_TABLE);
    assert(ret == ERR_OK);

    return ERR_OK;
}

void CMediaLibrary::updateMediaInMem(Media *media) {
    auto id = media->ID;
    auto count =  m_allMedias->getCount();

    for (uint32_t i = 0; i < count; i++) {
        auto item = m_allMedias->getItem(i);
        if (item) {
            if (item->ID == id) {
                *(item.get()) = *media;
                return;
            }
        }
    }
}

void CMediaLibrary::removeMediaInMem(Media *media) {
    auto id = media->ID;
    auto count =  m_allMedias->getCount();

    for (uint32_t i = 0; i < count; i++) {
        auto item = m_allMedias->getItem(i);
        if (item) {
            if (item->ID == id) {
                m_allMedias->removeItem(i);
                return;
            }
        }
    }
}

void CMediaLibrary::loadPlaylists() {
    m_playlists.clear();

    CSqlite3Stmt stmt;
    int ret = stmt.prepare(&m_db, SQL_QUERY_ALL_PLAYLISTS);
    assert(ret == ERR_OK);

    ret = stmt.step();
    while (ret == ERR_SL_OK_ROW) {
        PlaylistInfo info;
        int idx = 0;
        info.id = stmt.columnInt(idx++);
        info.name = stmt.columnText(idx++);
        info.duration = stmt.columnInt(idx++);
        info.count = stmt.columnInt(idx++);
        info.rating = stmt.columnInt(idx++);
        info.isUpToDate = stmt.columnInt(idx++) != 0;
        string idsText = stmt.columnText(idx++);

        VecStrings ids;
        strSplit(idsText.c_str(), ',', ids);
        for (auto &id : ids) {
            info.mediaIds.push_back(atoi(id.c_str()));
        }

        m_playlists.push_back(info);

        ret = stmt.step();
    }
}

int CMediaLibrary::savePlaylist(const PlaylistInfo &playlist) {
    CSqlite3Stmt stmt;

    if (playlist.id == -1) {
        auto ret = stmt.prepare(&m_db, SQL_ADD_PLAYLIST);
        assert(ret == ERR_OK);
    } else {
        auto ret = stmt.prepare(&m_db, SQL_UPDATE_PLAYLIST_BY_ID);
        assert(ret == ERR_OK);
    }

    string ids = strJoin(playlist.mediaIds.begin(), playlist.mediaIds.end(), ",");

    int idx = 0;
    auto ret = stmt.bindStaticText(idx++, playlist.name.c_str()); assert(ret == ERR_OK);
    ret = stmt.bindInt(idx++, playlist.duration); assert(ret == ERR_OK);
    ret = stmt.bindInt(idx++, playlist.count); assert(ret == ERR_OK);
    ret = stmt.bindInt(idx++, playlist.rating); assert(ret == ERR_OK);
    ret = stmt.bindInt(idx++, playlist.isUpToDate); assert(ret == ERR_OK);
    ret = stmt.bindStaticText(idx++, ids.c_str()); assert(ret == ERR_OK);

    ret = stmt.step();
    assert(ret == ERR_OK);
    return m_db.LastInsertRowId();
}

PlaylistPtr CMediaLibrary::getPlaylist(int playlistId) {
    for (auto &info : m_playlists) {
        if (info.id == playlistId) {
            auto playlist = newPlaylist();
            for (auto id : info.mediaIds) {
                auto media = getMediaByID(id);
                if (media) {
                    playlist->insertItem(-1, media);
                }
            }

            return playlist;
        }
    }

    return nullptr;
}

void CMediaLibrary::deltePlaylist(int playlistId) {
    string str = stringPrintf("DELETE FROM playlists WHERE id=%d", playlistId);

    auto ret = m_db.exec(str.c_str());
    assert(ret == ERR_OK);

    for (auto it = m_playlists.begin(); it != m_playlists.end(); ++it) {
        auto &info = *it;
        if (info.id == playlistId) {
            m_playlists.erase(it);
            break;
        }
    }
}

/**
 * 根据目录名，将歌曲统计到对应的目录下
 *
 *  在统计的时候使用树状的结构. 目录下的子目录一般也不多，所以没有必要使用 map 之类
 */
struct MediaTree {
    string                      name;
    int                         mediaCount;     // 歌曲数量
    int                         mediaDuration;  // 总共时长，单位秒

    vector<MediaTree>           children;

    MediaTree(const string &name, int duration, int count = 1) : name(name), mediaDuration(duration), mediaCount(count) {
    }
};

void addMediaTree(MediaTree &parent, VecStrings::iterator itPathName, const VecStrings::iterator &end, int duration) {
    parent.mediaDuration += duration;
    parent.mediaCount++;

    if (itPathName == end) {
        return;
    }

    string &name = *itPathName;
    for (auto &child : parent.children) {
        if (child.name == name) {
            addMediaTree(child, ++itPathName, end, duration);
            return;
        }
    }

    // 添加
    parent.children.push_back(MediaTree(*itPathName, duration));
    addMediaTree(parent.children.back(), ++itPathName, end, duration);
}

void mediaTreeToFolderCategory(string path, MediaTree &parent, VecMediaCategories &mediaCategories) {
    path += PATH_SEP_STR + parent.name;

    if (parent.children.size() == 1 && parent.mediaCount == parent.children[0].mediaCount) {
        // 只有一层目录，就不必添加了
        mediaTreeToFolderCategory(path, parent.children[0], mediaCategories);
        return;
    }

    mediaCategories.push_back(MediaCategory(MediaCategory::FOLDER, parent.name.c_str(), path.c_str(), parent.mediaCount, parent.mediaDuration));

    for (auto &child : parent.children) {
        mediaTreeToFolderCategory(path, child, mediaCategories);
    }
}

void CMediaLibrary::updateMediaCategories() {
    const int64_t UPDATE_DURATION = 60 * 3;

    auto now = getTickCount();
    if (abs((int64_t)(now - m_timeMediaCategoryUpdated)) < UPDATE_DURATION) {
        return;
    }
    m_timeMediaCategoryUpdated = now;

    m_mediaCategories.clear();

    uint32_t totalDuration = 0;
    m_mediaCategories.push_back(MediaCategory(MediaCategory::ALL,
        "", "", m_allMedias->getCount(), 0));

    for (auto &info : m_playlists) {
        m_mediaCategories.push_back(MediaCategory(MediaCategory::PLAYLIST,
            info.name.c_str(), info.name.c_str(), info.count, info.duration));
    }

    unordered_map<string, MediaCategory> byArtist;
    unordered_map<string, MediaCategory> byAlbum;
    unordered_map<string, MediaCategory> byGenre;
    MediaTree root("", 0, 0);
    int count = m_allMedias->getCount();

    for (int i = 0; i < count; i++) {
        auto media = m_allMedias->getItem(i);
        if (media) {
            int duration = media->duration / 1000;
            totalDuration += duration;

            // Artist
            auto artist = media->artist.c_str();
            auto it = byArtist.find(artist);
            if (it == byArtist.end()) {
                byArtist.insert(std::pair<string, MediaCategory>(string(artist), MediaCategory(MediaCategory::ARTIST, artist, artist, 1, duration)));
            } else {
                auto &info = (*it).second;
                info.mediaCount++;
                info.mediaDuration += duration;
            }

            // Artist-Album
            auto album = media->album.c_str();
            string key(media->artist);
            key.append(ALBUM_ARTIST_SEP);
            key.append(album);
            it = byAlbum.find(key);
            if (it == byAlbum.end()) {
                byAlbum.insert(std::pair<string, MediaCategory>(key, MediaCategory(MediaCategory::ALBUM, album, key.c_str(), 1, duration)));
            } else {
                auto &info = (*it).second;
                info.mediaCount++;
                info.mediaDuration += duration;
            }

            // Genre
            auto genre = media->genre.c_str();
            it = byGenre.find(genre);
            if (it == byGenre.end()) {
                byGenre.insert(std::pair<string, MediaCategory>(string(genre), MediaCategory(MediaCategory::GENRE, genre, genre, 1, duration)));
            } else {
                auto &info = (*it).second;
                info.mediaCount++;
                info.mediaDuration += duration;
            }

            VecStrings paths;
            strSplit(media->url.c_str(), PATH_SEP_CHAR, paths);
            paths.pop_back(); // 去掉文件名
            addMediaTree(root, paths.begin(), paths.end(), duration);
        }
    }

    m_mediaCategories.front().mediaDuration = totalDuration;

    for (auto &item : byArtist) {
        m_mediaCategories.push_back(item.second);
    }

    for (auto &item : byAlbum) {
        m_mediaCategories.push_back(item.second);
    }

    for (auto &item : byGenre) {
        m_mediaCategories.push_back(item.second);
    }

    for (auto &parent : root.children) {
        auto &children = parent.children;
        for (auto &child : children) {
            mediaTreeToFolderCategory(parent.name, child, m_mediaCategories);
        }
    }

    std::sort(m_mediaCategories.begin(), m_mediaCategories.end(),
            [](const MediaCategory &a, const MediaCategory &b) {
        // 按照 type, mediaCount, name 来排序
        if (a.type < b.type) {
            return true;
        } else if (a.type == b.type) {
            if (a.mediaCount > b.mediaCount) {
                return true;
            } else if (a.mediaCount == b.mediaCount) {
                return a.name < b.name;
            }
        }
        return false;
    });
}
