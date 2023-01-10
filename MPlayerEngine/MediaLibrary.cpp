#include "../Skin/Skin.h"
#include "MPlayer.h"
#include "MediaLibrary.h"


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
    nRet = sqlstmt.bindText(n, strData.c_str(), (int)strData.size());\
    if (nRet != ERR_OK)\
        goto RET_FAILED;\
    n++;

CMedia *newMedia() {
    CMedia *pMedia;

    pMedia = new CMedia;
    pMedia->addRef();

    return pMedia;
}

cstr_t mediaCategoryTypeToString(MediaCategory::Type type) {
    static cstr_t NAMES[] = { _TLM("All Musics"), _TLM("Playlist"), _TLM("Folder"), _TLM("Artist"), _TLM("Album"), _TLM("Genre") };
    assert(CountOf(NAMES) == MediaCategory::_COUNT);
    assert(type >= 0 && type <= MediaCategory::_COUNT);

    return NAMES[type];
}


/*
How to calculate auto rating value:

default auto rating: 300
max auto rating: 590


*/
int getAutoRating(IMedia *pMedia) {
    int64_t length = 0, rating = 300;
    int64_t times_play_skipped = 0, times_played = 0;

    pMedia->getAttribute(MA_DURATION, &length);
    pMedia->getAttribute(MA_TIMES_PLAY_SKIPPED, &times_play_skipped);
    pMedia->getAttribute(MA_TIMES_PLAYED, &times_played);

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
        m_pMediaLib = nullptr;
    }

    IPlaylist                   *m_pPlaylist;
    CMediaLibrary               *m_pMediaLib;

    MLRESULT query(CMediaLibrary *pMediaLib, cstr_t szSQL) {
        if (!pMediaLib->isOK()) {
            return pMediaLib->m_nInitResult;
        }

        int nRet;

        m_pMediaLib = pMediaLib;

        m_pPlaylist = m_pMediaLib->newPlaylist();
        if (!m_pPlaylist) {
            return ERR_NO_MEM;
        }

        nRet = sqlite3_exec(
            m_pMediaLib->m_db.m_db,
            szSQL,
            &queryCallback,
            this,
            nullptr);
        if (nRet != SQLITE_OK) {
            ERR_LOG2("Exe SQL:%S, : %S", szSQL, m_pMediaLib->m_db.errorMsg());
            m_pPlaylist->release();
            return ERR_SL_EXE_SQL;
        }

        return ERR_OK;
    }

    static int queryCallback(void *args, int numCols, char **results, char ** columnNames) {
        CMLQueryPlaylist *pThis = (CMLQueryPlaylist*)args;

        CMedia *pMedia = newMedia();

        pThis->m_pMediaLib->getMediaCallback(pMedia, numCols, results);

        pThis->m_pPlaylist->insertItem(-1, pMedia);

        pMedia->release();

        return 0;
    }

};


CMediaLibrary::CMediaLibrary() {
    OBJ_REFERENCE_INIT;
}

CMediaLibrary::~CMediaLibrary() {
    close();
}

void CMediaLibrary::getMediaCategories(VecMediaCategories &categoriesOut) {
    updateMediaCategories();

    categoriesOut = m_mediaCategories;
}

MLRESULT CMediaLibrary::getMediaCategory(const MediaCategory &category, IPlaylist **playlist) {
    assert(playlist != nullptr);
    if (category.type == MediaCategory::ALL) {
        m_allMedias->addRef();
        *playlist = m_allMedias;
        return ERR_OK;
    } else if (category.type == MediaCategory::PLAYLIST) {
        *playlist = getPlaylist(atoi(category.value.c_str()));
        return ERR_OK;
    }

    int count = m_allMedias->getCount();
    auto pl = newPlaylist();

    switch (category.type) {
        case MediaCategory::FOLDER: {
            CXStr url;
            for (int i = 0; i < count; i++) {
                CMPAutoPtr<IMedia> media;
                m_allMedias->getItem(i, &media);
                media->getSourceUrl(&url);
                if (startsWith(url.c_str(), category.value.c_str())) {
                    pl->insertItem(-1, media);
                }
            }
            break;
        }
        case MediaCategory::ARTIST: {
            CXStr str;
            for (int i = 0; i < count; i++) {
                CMPAutoPtr<IMedia> media;
                m_allMedias->getItem(i, &media);
                media->getArtist(&str);
                if (category.value == str.c_str()) {
                    pl->insertItem(-1, media);
                }
            }
            break;
        }
        case MediaCategory::ALBUM: {
            CXStr str;
            for (int i = 0; i < count; i++) {
                CMPAutoPtr<IMedia> media;
                m_allMedias->getItem(i, &media);
                media->getArtist(&str);
                if (startsWith(category.value.c_str(), str.c_str())) {
                    SizedString key(category.value.c_str(), category.value.size());
                    key.shrink((int)str.size());
                    if (key.startsWith(ALBUM_ARTIST_SEP)) {
                        key.shrink(ALBUM_ARTIST_SEP_LEN);
                        media->getAlbum(&str);
                        if (key.equal(str.c_str())) {
                            pl->insertItem(-1, media);
                        }
                    }
                }
            }
            break;
        }
        case MediaCategory::GENRE: {
            CXStr str;
            for (int i = 0; i < count; i++) {
                CMPAutoPtr<IMedia> media;
                m_allMedias->getItem(i, &media);
                if (media->getAttribute(MA_GENRE, &str) == ERR_OK &&
                        category.value == str.c_str()) {
                    pl->insertItem(-1, media);
                }
            }
            break;
        }
        default:
            assert(0);
            break;
    }

    *playlist = pl;
    return ERR_OK;
}

MLRESULT CMediaLibrary::getAllArtist(IVString **ppvArtist) {
    return queryVStr("select artist from medialib group by artist", ppvArtist);
}

MLRESULT CMediaLibrary::getAllAlbum(IVString **ppvAlbum) {
    return queryVStr("select album from medialib group by album", ppvAlbum);
}

MLRESULT CMediaLibrary::getAllGenre(IVString **ppvAlbum) {
    return queryVStr("select genre from medialib group by genre", ppvAlbum);
}

MLRESULT CMediaLibrary::getAllYear(IVInt **ppvYear) {
    if (!isOK()) {
        return m_nInitResult;
    }

    *ppvYear = nullptr;

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    sqlQuery.prepare(&m_db, "select year from medialib group by year");

    auto pvInt = new CVInt();
    pvInt->addRef();

    int nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW) {
        pvInt->push_back(sqlQuery.columnInt(0));

        nRet = sqlQuery.step();
    }
    if (pvInt->size() == 0) {
        pvInt->release();
        pvInt = nullptr;
        if (nRet == ERR_OK) {
            nRet = ERR_NOT_FOUND;
        }
    } else {
        nRet = ERR_OK;
    }

    *ppvYear = pvInt;

    return nRet;
}

MLRESULT CMediaLibrary::getAlbumOfArtist(cstr_t szArtist, IVString **ppvAlbum) {
    if (!isOK()) {
        return m_nInitResult;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    CSqlite3Stmt sqlQuery;

    *ppvAlbum = nullptr;

    int nRet = sqlQuery.prepare(&m_db, "select album from medialib where artist=? group by album");
    if (nRet != ERR_OK) {
        return nRet;
    }

    auto pvStr = new CVXStr();
    pvStr->addRef();

    sqlQuery.bindStaticText(1, szArtist);

    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW) {
        auto text = sqlQuery.columnText(0);
        if (text) {
            pvStr->push_back(text);
        }

        nRet = sqlQuery.step();
    }
    if (pvStr->size() == 0) {
        pvStr->release();
        pvStr = nullptr;
        if (nRet == ERR_OK) {
            nRet = ERR_NOT_FOUND;
        }
    } else {
        nRet = ERR_OK;
    }

    *ppvAlbum = pvStr;

    return nRet;
}

uint32_t CMediaLibrary::getMediaCount() {
    return sqlite_query_int_value(m_db.m_db, SQL_COUNT_OF_MEDIA);
}

MLRESULT CMediaLibrary::getMediaByUrl(cstr_t szUrl, IMedia **ppMedia) {
    assert(ppMedia);

    if (!isOK()) {
        return m_nInitResult;
    }

    *ppMedia = nullptr;

    RMutexAutolock autolock(m_mutexDataAccess);

    m_sqlQueryByUrl.bindStaticText(1, szUrl);

    int nRet = m_sqlQueryByUrl.step();
    if (nRet == ERR_SL_OK_ROW) {
        CMedia *media;

        media = newMedia();
        sqliteQueryMedia(&m_sqlQueryByUrl, media);
        *ppMedia = media;

        if (media->m_bIsFileDeleted) {
            // recover to undeleted.
            if (undeleteMedia(media->getID()) == ERR_OK) {
                media->m_bIsFileDeleted = false;
            }
        }
        nRet = ERR_OK;
    } else {
        nRet = ERR_NOT_FOUND;
    }

    m_sqlQueryByUrl.reset();

    return nRet;
}

MLRESULT CMediaLibrary::getMediaByID(int id, IMedia **ppMedia) {
    assert(ppMedia);

    if (!isOK()) {
        return m_nInitResult;
    }

    *ppMedia = nullptr;

    RMutexAutolock autolock(m_mutexDataAccess);

    m_stmtQueryByID.bindInt(1, id);

    int nRet = m_stmtQueryByID.step();
    if (nRet == ERR_SL_OK_ROW) {
        nRet = ERR_OK;
        CMedia *media;

        media = newMedia();
        sqliteQueryMedia(&m_stmtQueryByID, media);
        if (!media->m_bIsFileDeleted) {
            *ppMedia = media;
            nRet = ERR_NOT_FOUND;
        }
    } else {
        nRet = ERR_NOT_FOUND;
    }

    m_stmtQueryByID.reset();

    return nRet;
}

MLRESULT CMediaLibrary::add(cstr_t szMediaUrl, IMedia **ppMedia) {
    if (!isOK()) {
        return m_nInitResult;
    }

    *ppMedia = nullptr;

    int nRet = getMediaByUrl(szMediaUrl, ppMedia);
    if (nRet != ERR_NOT_FOUND) {
        return nRet;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    CMedia *pMedia = newMedia();

    pMedia->setSourceUrl(szMediaUrl);

    m_player->loadMediaTagInfo(pMedia, true);

    pMedia->m_timePlayed = time(nullptr);
    pMedia->m_timeAdded = time(nullptr);

    nRet = doAddMedia(pMedia);
    if (nRet == ERR_OK) {
        *ppMedia = pMedia;
    } else {
        pMedia->release();
    }

    return ERR_OK;
}

// add media to media library fast, won't update media info.
MLRESULT CMediaLibrary::addFast(cstr_t szMediaUrl, IMedia **ppMedia) {
    if (!isOK()) {
        return m_nInitResult;
    }

    *ppMedia = nullptr;

    int nRet = getMediaByUrl(szMediaUrl, ppMedia);
    if (nRet != ERR_NOT_FOUND) {
        return nRet;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    CMedia *pMedia = newMedia();

    pMedia->setSourceUrl(szMediaUrl);
    pMedia->m_timePlayed = time(nullptr);
    pMedia->m_timeAdded = time(nullptr);

    pMedia->m_strTitle = urlGetTitle(szMediaUrl);

    int n = 1;
    nRet = m_sqlAddFast.bindText(n++, szMediaUrl);
    if (nRet != ERR_OK) {
        goto RET_FAILED;
    }

    SQLITE3_BIND_TEXT(m_sqlAddFast, pMedia->m_strTitle);
    m_sqlAddFast.bindInt64(n++, pMedia->m_timeAdded);

    nRet = m_sqlAddFast.step();
    if (nRet != ERR_OK) {
        goto RET_FAILED;
    }

    pMedia->m_nID = m_db.LastInsertRowId();

    m_sqlAddFast.reset();

    m_allMedias->insertItem(-1, pMedia);

    *ppMedia = pMedia;

    return nRet;

RET_FAILED:
    if (pMedia) {
        delete pMedia;
    }

    return nRet;
}

// update Media Info to DB
MLRESULT CMediaLibrary::updateMediaInfo(IMedia *pMedia) {
    if (!isOK()) {
        return m_nInitResult;
    }

    assert(pMedia->getID() != MEDIA_ID_INVALID);
    if (pMedia->getID() == MEDIA_ID_INVALID) {
        return ERR_ML_NOT_FOUND;
    }

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;
    int nRet;
    int n = 1;
    int64_t value;
    CXStr str;

    sqlQuery.prepare(&m_db, "update medialib set url=?, info_updated=1, artist=?, album=?, title=?, track=?,"\
        "year=?, genre=?, comment=?, length=?, filesize=?, lyrics_file=? where id=?");

    pMedia->getSourceUrl(&str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    pMedia->getAttribute(MA_ARTIST, &str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    pMedia->getAttribute(MA_ALBUM, &str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    pMedia->getAttribute(MA_TITLE, &str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    pMedia->getAttribute(MA_TRACK_NUMB, &value);
    sqlQuery.bindInt(n++, (int)value);

    pMedia->getAttribute(MA_YEAR, &value);
    sqlQuery.bindInt(n++, (int)value);

    pMedia->getAttribute(MA_GENRE, &str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    pMedia->getAttribute(MA_COMMENT, &str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    pMedia->getAttribute(MA_DURATION, &value);
    sqlQuery.bindInt(n++, (int)value);

    pMedia->getAttribute(MA_FILESIZE, &value);
    sqlQuery.bindInt(n++, (int)value);

    pMedia->getAttribute(MA_LYRICS_FILE, &str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    sqlQuery.bindInt(n++, (int)pMedia->getID());
    nRet = sqlQuery.step();
    if (nRet == ERR_OK) {
        pMedia->setInfoUpdatedToMediaLib(true);
    }

    updateMediaInMem(pMedia);

RET_FAILED:
    return nRet;
}

// removes the specified item from the media library
MLRESULT CMediaLibrary::remove(IMedia **pMedia, bool bDeleteFile) {
    if (!isOK()) {
        return m_nInitResult;
    }

    removeMediaInMem(*pMedia);

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    sqlQuery.prepare(&m_db, "delete from medialib where id=?");

    sqlQuery.bindInt(1, (int)(*pMedia)->getID());
    int nRet = sqlQuery.step();
    if (nRet == ERR_OK) {
        if (bDeleteFile) {
            CXStr str;
            (*pMedia)->getSourceUrl(&str);
            deleteFile(str.c_str());
        }

        (*pMedia)->release();
        *pMedia = nullptr;
    }

    return ERR_OK;
}

// If the media file was removed temporarily, set this flag on.
MLRESULT CMediaLibrary::setDeleted(IMedia **pMedia) {
    char szSql[256];

    snprintf(szSql, CountOf(szSql), "update medialib set file_deleted=1 where id=%d", (int)(*pMedia)->getID());

    removeMediaInMem(*pMedia);

    int nRet = executeSQL(szSql);
    if (nRet == ERR_OK) {
        (*pMedia)->release();
        *pMedia = nullptr;
    }

    return nRet;
}

IPlaylist *CMediaLibrary::getAll() {
    return m_allMedias;
}

MLRESULT CMediaLibrary::getAll(IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) {
    string strSql = "select * from medialib where file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    CMLQueryPlaylist query;
    int nRet = query.query(this, strSql.c_str());
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getByArtist(cstr_t szArtist, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) {
    string strSql = "select * from medialib where artist=? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    return queryPlaylist(strSql.c_str(), szArtist, ppPlaylist);
}

MLRESULT CMediaLibrary::getByAlbum(cstr_t szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) {
    string strSql = "select * from medialib where album=? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    return queryPlaylist(strSql.c_str(), szAlbum, ppPlaylist);
}

MLRESULT CMediaLibrary::getByAlbum(cstr_t szArtist, cstr_t szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) {
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

    int nRet = query.query(this, strSql.c_str());
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getByTitle(cstr_t szTitle, IPlaylist **ppPlaylist) {
    return queryPlaylist("select * from medialib where title=? and file_deleted!=1", szTitle, ppPlaylist);
}

MLRESULT CMediaLibrary::getByGenre(cstr_t szGenre, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) {
    string strSql;

    strSql = "select * from medialib where genre=? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    return queryPlaylist(strSql.c_str(), szGenre, ppPlaylist);
}

MLRESULT CMediaLibrary::getByYear(int nYear, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) {
    if (!isOK()) {
        return m_nInitResult;
    }

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    string strSql = "select * from medialib where year=? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    int nRet = sqlQuery.prepare(&m_db, strSql.c_str());
    if (nRet != ERR_OK) {
        return nRet;
    }

    CPlaylist *playlist = newPlaylist();

    sqlQuery.bindInt(1, nYear);
    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW) {
        CMedia *media;

        media = newMedia();
        sqliteQueryMedia(&sqlQuery, media);
        playlist->insertItem(-1, media);
        media->release();

        nRet = sqlQuery.step();
    }
    if (playlist->getCount() == 0) {
        playlist->release();
        playlist = nullptr;
        if (nRet == ERR_OK) {
            nRet = ERR_NOT_FOUND;
        }
    } else {
        nRet = ERR_OK;
    }

    *ppPlaylist = playlist;

    return nRet;
}

MLRESULT CMediaLibrary::getByRating(int nRating, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) {
    if (!isOK()) {
        return m_nInitResult;
    }

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    string strSql = "select * from medialib where rating>=? and rating<? and file_deleted!=1";
    appendQeuryStatment(orderBy, nTopN, strSql);

    int nRatingStart = nRating * 100;
    int nRatingEnd = (nRating + 1) * 100 - 1;

    int nRet = sqlQuery.prepare(&m_db, strSql.c_str());
    if (nRet != ERR_OK) {
        return nRet;
    }

    CPlaylist *playlist = newPlaylist();

    sqlQuery.bindInt(1, nRatingStart);
    sqlQuery.bindInt(2, nRatingEnd);
    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW) {
        CMedia *media;

        media = newMedia();
        sqliteQueryMedia(&sqlQuery, media);
        playlist->insertItem(-1, media);
        media->release();

        nRet = sqlQuery.step();
    }
    if (playlist->getCount() == 0) {
        playlist->release();
        playlist = nullptr;
        if (nRet == ERR_OK) {
            nRet = ERR_NOT_FOUND;
        }
    } else {
        nRet = ERR_OK;
    }

    *ppPlaylist = playlist;

    return nRet;
}

MLRESULT CMediaLibrary::getRecentPlayed(uint32_t nCount, IPlaylist **ppPlaylist) {
    CMLQueryPlaylist query;
    char szQuery[256];
    int nRet;

    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted!=1 order by time_played desc limit %d", nCount);

    nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getTopPlayed(uint32_t nCount, IPlaylist **ppPlaylist) {
    char szQuery[256];
    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted!=1 order by times_played desc limit %d", nCount);

    CMLQueryPlaylist query;
    int nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getTopRating(uint32_t nCount, IPlaylist **ppPlaylist) {
    char szQuery[256];
    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted!=1 order by rating desc limit %d", nCount);

    CMLQueryPlaylist query;
    int nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getRecentAdded(uint32_t nCount, IPlaylist **ppPlaylist) {
    char szQuery[256];
    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted!=1 order by time_added desc limit %d", nCount);

    CMLQueryPlaylist query;
    int nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getRecentPlayed(int nDayAgoBegin, int nDayAgoEnd, IPlaylist **ppPlaylist) {
    char szQuery[256];

    auto begin = time(nullptr) - DateTime::SECOND_IN_ONE_DAY * nDayAgoBegin;
    auto end = time(nullptr) - DateTime::SECOND_IN_ONE_DAY * nDayAgoEnd;

    snprintf(szQuery, CountOf(szQuery), "select * from medialib where time_played between %ld and %ld and file_deleted!=1 order by time_played", begin, end);

    CMLQueryPlaylist query;
    int nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getRandom(uint32_t nCount, IPlaylist **ppPlaylist) {
    if (!isOK()) {
        return m_nInitResult;
    }
    return ERR_OK;
}

MLRESULT CMediaLibrary::getRandomByTime(uint32_t nDurationInMin, IPlaylist **ppPlaylist) {
    if (!isOK()) {
        return m_nInitResult;
    }
    return ERR_OK;
}

// 0-5, 0 for unrate.
MLRESULT CMediaLibrary::rate(IMedia *pMedia, uint32_t nRating) {
    if (!isOK()) {
        return m_nInitResult;
    }

    if (!pMedia) {
        return ERR_OK;
    }

    bool bIsUserRating = (nRating != 0);
    if (bIsUserRating) {
        nRating = nRating * 100 + 99;
    } else {
        nRating = getAutoRating(pMedia);
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    char szSql[256];
    snprintf(szSql, CountOf(szSql), "update medialib set is_user_rating=%d, rating=%d where id=%d",
        bIsUserRating, nRating, (int)pMedia->getID());

    pMedia->setAttribute(MA_RATING, nRating / 100);
    pMedia->setAttribute(MA_IS_USER_RATING, bIsUserRating);

    return executeSQL(szSql);
}

MLRESULT CMediaLibrary::updatePlayedTime(IMedia *pMedia) {
    if (!isOK()) {
        return m_nInitResult;
    }

    if (!pMedia || pMedia->getID() == MEDIA_ID_INVALID) {
        return ERR_OK;
    }

    int64_t value;
    RMutexAutolock autolock(m_mutexDataAccess);
    pMedia->getAttribute(MA_TIME_PLAYED, &value);

    char szSql[256];
    snprintf(szSql, CountOf(szSql), "update medialib set time_played=%lld where id=%d", value, (int)pMedia->getID());

    return executeSQL(szSql);
}

MLRESULT CMediaLibrary::markPlayFinished(IMedia *pMedia) {
    if (!isOK()) {
        return m_nInitResult;
    }

    if (!pMedia || pMedia->getID() == MEDIA_ID_INVALID) {
        return ERR_OK;
    }

    char szSql[256];
    int64_t nTimesPlayed = 0, bIsUserRating = 0;

    RMutexAutolock autolock(m_mutexDataAccess);

    pMedia->getAttribute(MA_TIMES_PLAYED, &nTimesPlayed);
    pMedia->setAttribute(MA_TIMES_PLAYED, nTimesPlayed + 1);
    pMedia->getAttribute(MA_IS_USER_RATING, &bIsUserRating);
    if (bIsUserRating) {
        snprintf(szSql, CountOf(szSql), "update medialib set times_played=times_played+1 where id=%d", (int)pMedia->getID());
    } else {
        int nRating = getAutoRating(pMedia);
        snprintf(szSql, CountOf(szSql), "update medialib set times_played=times_played+1, rating=%d where id=%d",
            nRating, (int)pMedia->getID());
        pMedia->setAttribute(MA_RATING, nRating / 100);
    }

    int nRet = executeSQL(szSql);

    updatePlayedTime(pMedia);

    return nRet;
}

MLRESULT CMediaLibrary::markPlaySkipped(IMedia *pMedia) {
    if (!isOK()) {
        return m_nInitResult;
    }

    if (!pMedia || pMedia->getID() == MEDIA_ID_INVALID) {
        return ERR_OK;
    }

    char szSql[256];
    int64_t nTimesPlaySkipped = 0, bIsUserRating = 0;

    RMutexAutolock autolock(m_mutexDataAccess);

    pMedia->getAttribute(MA_TIMES_PLAY_SKIPPED, &nTimesPlaySkipped);
    pMedia->setAttribute(MA_TIMES_PLAY_SKIPPED, nTimesPlaySkipped + 1);
    pMedia->getAttribute(MA_IS_USER_RATING, &bIsUserRating);
    if (bIsUserRating) {
        snprintf(szSql, CountOf(szSql), "update medialib set times_play_skipped=times_play_skipped+1 where id=%d", (int)pMedia->getID());
    } else {
        int nRating = getAutoRating(pMedia);
        snprintf(szSql, CountOf(szSql), "update medialib set times_play_skipped=times_play_skipped+1, rating=%d where id=%d",
            nRating, (int)pMedia->getID());
        pMedia->setAttribute(MA_RATING, nRating / 100);
    }

    int nRet = executeSQL(szSql);

    return nRet;
}

int CMediaLibrary::init(CMPlayer *pPlayer) {
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

    m_player = pPlayer;

    m_nInitResult = getAll(&m_allMedias, MLOB_ARTIST, -1);
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
    m_player.release();
}

MLRESULT CMediaLibrary::queryPlaylist(cstr_t szSQL, cstr_t szClause, IPlaylist **ppPlaylist) {
    if (!isOK()) {
        return m_nInitResult;
    }

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    int nRet = sqlQuery.prepare(&m_db, szSQL);
    if (nRet != ERR_OK) {
        return nRet;
    }

    nRet = sqlQuery.bindText(1, szClause);
    if (nRet != ERR_OK) {
        return nRet;
    }

    CPlaylist *playlist = newPlaylist();

    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW) {
        CMedia *media = newMedia();
        sqliteQueryMedia(&sqlQuery, media);
        playlist->insertItem(-1, media);
        media->release();

        nRet = sqlQuery.step();
    }
    if (playlist->getCount() == 0) {
        playlist->release();
        playlist = nullptr;
        if (nRet == ERR_OK) {
            nRet = ERR_NOT_FOUND;
        }
    } else {
        nRet = ERR_OK;
    }

    *ppPlaylist = playlist;

    return nRet;
}

//
MLRESULT CMediaLibrary::queryVStr(cstr_t szSql, IVString **ppvStr) {
    if (!isOK()) {
        return m_nInitResult;
    }

    *ppvStr = nullptr;

    RMutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt sqlQuery;

    sqlQuery.prepare(&m_db, szSql);

    auto pvStr = new CVXStr();
    pvStr->addRef();

    int nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW) {
        auto text = (const char *)sqlQuery.columnText(0);
        if (text) {
            pvStr->push_back(text);
        }

        nRet = sqlQuery.step();
    }
    if (pvStr->size() == 0) {
        pvStr->release();
        pvStr = nullptr;
        if (nRet == ERR_OK) {
            nRet = ERR_NOT_FOUND;
        }
    } else {
        nRet = ERR_OK;
    }

    *ppvStr = pvStr;

    return nRet;
}

MLRESULT CMediaLibrary::executeSQL(cstr_t szSql) {
    return m_db.exec(szSql);
}

#define  MediaGetSqlite3ColumText(mediaAttri)    \
    text = (const char *)sqlStmt->columnText(n++);    \
    if (text)\
        mediaAttri = text;\
    else\
        mediaAttri.resize(0);

void CMediaLibrary::sqliteQueryMedia(CSqlite3Stmt *sqlStmt, CMedia *pMedia) {
    int n = 0;
    const char *text;
    string str;

    pMedia->m_nID = sqlStmt->columnInt(n++);

    MediaGetSqlite3ColumText(pMedia->m_strUrl);
    pMedia->m_bIsFileDeleted = tobool(sqlStmt->columnInt(n++));
    pMedia->m_bInfoUpdated = tobool(sqlStmt->columnInt(n++));
    pMedia->m_bIsUserRating = tobool(sqlStmt->columnInt(n++));
    MediaGetSqlite3ColumText(pMedia->m_strArtist);
    MediaGetSqlite3ColumText(pMedia->m_strAlbum);
    MediaGetSqlite3ColumText(pMedia->m_strTitle);
    pMedia->m_nTrackNumb = sqlStmt->columnInt(n++);
    pMedia->m_nYear = sqlStmt->columnInt(n++);
    MediaGetSqlite3ColumText(pMedia->m_strGenre);
    MediaGetSqlite3ColumText(pMedia->m_strComment);
    pMedia->m_nLength = sqlStmt->columnInt(n++);
    pMedia->m_nFileSize = sqlStmt->columnInt(n++);
    pMedia->m_timeAdded = sqlStmt->columnInt64(n++);
    pMedia->m_timePlayed = sqlStmt->columnInt64(n++);
    pMedia->m_nRating = sqlStmt->columnInt(n++) / 100;
    pMedia->m_nPlayed = sqlStmt->columnInt(n++);
    pMedia->m_nPlaySkipped = sqlStmt->columnInt(n++);
    MediaGetSqlite3ColumText(pMedia->m_musicHash);
    MediaGetSqlite3ColumText(pMedia->m_strLyricsFile);
}

void CMediaLibrary::getMediaCallback(CMedia *pMedia, int numCols, char **results) {
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

    pMedia->m_nID = atoi(results[n++]);

    FILED_GET_TEXT(pMedia->m_strUrl);

    FILED_GET_BOOL(pMedia->m_bIsFileDeleted);
    FILED_GET_BOOL(pMedia->m_bInfoUpdated);
    FILED_GET_BOOL(pMedia->m_bIsUserRating);

    FILED_GET_TEXT(pMedia->m_strArtist);
    FILED_GET_TEXT(pMedia->m_strAlbum);
    FILED_GET_TEXT(pMedia->m_strTitle);
    FILED_GET_INT(pMedia->m_nTrackNumb);
    FILED_GET_INT(pMedia->m_nYear);

    FILED_GET_TEXT(pMedia->m_strGenre);
    FILED_GET_TEXT(pMedia->m_strComment);
    FILED_GET_INT(pMedia->m_nLength);
    FILED_GET_INT(pMedia->m_nFileSize);
    FILED_GET_INT64(pMedia->m_timeAdded);
    FILED_GET_INT64(pMedia->m_timePlayed);
    FILED_GET_INT(pMedia->m_nRating);
    pMedia->m_nRating /= 100;
    FILED_GET_INT(pMedia->m_nPlayed);
    FILED_GET_INT(pMedia->m_nPlaySkipped);
    FILED_GET_TEXT(pMedia->m_strLyricsFile);

    assert(n <= numCols);
}

MLRESULT CMediaLibrary::undeleteMedia(long nMediaID) {
    char szSql[256];

    snprintf(szSql, CountOf(szSql), "update medialib set file_deleted!=1 where id=%d", (int)nMediaID);

    return executeSQL(szSql);
}

MLRESULT CMediaLibrary::add(IMedia *pMedia) {
    if (!isOK()) {
        return m_nInitResult;
    }

    CMPAutoPtr<IMedia> pMediaExist;
    CXStr str;

    pMedia->getSourceUrl(&str);
    int nRet = getMediaByUrl(str.c_str(), &pMediaExist);
    if (nRet != ERR_NOT_FOUND) {
        return nRet;
    }

    RMutexAutolock autolock(m_mutexDataAccess);

    return doAddMedia((CMedia*)pMedia);
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

MLRESULT CMediaLibrary::doAddMedia(CMedia *pMedia) {
    int nRet;
    int n = 1;

    // url, artist, album, title, track, year, genre, comment, length, filesize, time_added, music_hash, lyrics_file
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strUrl);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strArtist);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strAlbum);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strTitle);
    m_sqlAdd.bindInt(n++, pMedia->m_nTrackNumb);
    m_sqlAdd.bindInt(n++, pMedia->m_nYear);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strGenre);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strComment);
    m_sqlAdd.bindInt(n++, (int)pMedia->m_nLength);
    m_sqlAdd.bindInt(n++, (int)pMedia->m_nFileSize);
    m_sqlAdd.bindInt64(n++, pMedia->m_timeAdded);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_musicHash);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strLyricsFile);

    nRet = m_sqlAdd.step();
    if (nRet != ERR_OK) {
        pMedia->release();
    } else {
        pMedia->m_nID = m_db.LastInsertRowId();
        pMedia->setInfoUpdatedToMediaLib(true);
    }

    m_sqlAdd.reset();

    m_allMedias->insertItem(-1, pMedia);

RET_FAILED:

    return nRet;
}

CPlaylist *CMediaLibrary::newPlaylist() {
    CPlaylist *pPlaylist;

    pPlaylist = new CPlaylist(m_player);
    pPlaylist->addRef();

    return pPlaylist;
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

void CMediaLibrary::updateMediaInMem(IMedia *media) {
    auto id = media->getID();
    auto count =  m_allMedias->getCount();

    for (uint32_t i = 0; i < count; i++) {
        CMPAutoPtr<IMedia> item;
        if (m_allMedias->getItem(i, &item) == ERR_OK) {
            if (item->getID() == id) {
                m_allMedias->removeItem(i);
                m_allMedias->insertItem(i, media);
                return;
            }
        }
    }
}

void CMediaLibrary::removeMediaInMem(IMedia *media) {
    auto id = media->getID();
    auto count =  m_allMedias->getCount();

    for (uint32_t i = 0; i < count; i++) {
        CMPAutoPtr<IMedia> item;
        if (m_allMedias->getItem(i, &item) == ERR_OK) {
            if (item->getID() == id) {
                m_allMedias->removeItem(i);
                return;
            }
        }
    }
}

void CMediaLibrary::loadPlaylists() {
    m_playlists.clear();

    CSqlite3Stmt stmt;
    auto ret = stmt.prepare(&m_db, SQL_QUERY_ALL_PLAYLISTS);
    assert(ret == ERR_OK);

    int nRet = stmt.step();
    while (nRet == ERR_SL_OK_ROW) {
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

        nRet = stmt.step();
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

IPlaylist *CMediaLibrary::getPlaylist(int playlistId) {
    for (auto &info : m_playlists) {
        if (info.id == playlistId) {
            auto playlist = newPlaylist();
            for (auto id : info.mediaIds) {
                CMPAutoPtr<IMedia> m;
                if (getMediaByID(id, &m) == ERR_OK) {
                    playlist->insertItem(-1, m);
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
        CMPAutoPtr<IMedia> media;
        if (m_allMedias->getItem(i, &media) == ERR_OK) {
            CXStr str;

            int duration = media->getDuration() / 1000;
            totalDuration += duration;

            // Artist
            media->getArtist(&str);
            auto it = byArtist.find(str.c_str());
            if (it == byArtist.end()) {
                byArtist.insert(std::pair<string, MediaCategory>(string(str.c_str()), MediaCategory(MediaCategory::ARTIST, str.c_str(), str.c_str(), 1, duration)));
            } else {
                auto &info = (*it).second;
                info.mediaCount++;
                info.mediaDuration += duration;
            }

            // Artist-Album
            string key(str.c_str());
            media->getAlbum(&str);
            key.append(ALBUM_ARTIST_SEP);
            key.append(str.c_str());
            it = byAlbum.find(key);
            if (it == byAlbum.end()) {
                byAlbum.insert(std::pair<string, MediaCategory>(key, MediaCategory(MediaCategory::ALBUM, str.c_str(), key.c_str(), 1, duration)));
            } else {
                auto &info = (*it).second;
                info.mediaCount++;
                info.mediaDuration += duration;
            }

            // Genre
            if (media->getAttribute(MA_GENRE, &str) == ERR_OK) {
                auto it = byGenre.find(str.c_str());
                if (it == byGenre.end()) {
                    byGenre.insert(std::pair<string, MediaCategory>(string(str.c_str()), MediaCategory(MediaCategory::GENRE, str.c_str(), str.c_str(), 1, duration)));
                } else {
                    auto &info = (*it).second;
                    info.mediaCount++;
                    info.mediaDuration += duration;
                }
            }

            VecStrings paths;
            media->getSourceUrl(&str);
            strSplit(str.c_str(), PATH_SEP_CHAR, paths);
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
