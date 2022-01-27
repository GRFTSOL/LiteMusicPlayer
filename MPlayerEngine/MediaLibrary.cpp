// MediaLibrary.cpp: implementation of the CMediaLibrary class.
//
//////////////////////////////////////////////////////////////////////

#include "../Skin/Skin.h"
#include "MPlayer.h"
#include "MediaLibrary.h"

#define MEDIA_LIB_VER_CUR        1


/*

CREATE TABLE lib_ver
(
  version integer Primary Key,
);


CREATE TABLE IF NOT EXISTS medialib
(
  id integer Primary Key,
  url text COLLATE NOCASE,
  file_deleted integer,
  info_updated integer,
  is_user_rating integer,
  artist text COLLATE NOCASE,
  album text COLLATE NOCASE,
  title text COLLATE NOCASE,
  track integer,
  year integer,
  genre integer COLLATE NOCASE,
  comment integer COLLATE NOCASE,
  length integer,
  filesize integer,
  time_added integer,
  time_played integer,
  rating integer,
  times_played integer,
  times_play_skipped integer,
  lyrics_file text COLLATE NOCASE
);

CREATE INDEX IF NOT EXISTS mli_url on medialib (url);
CREATE INDEX IF NOT EXISTS mli_artist on medialib (artist);
CREATE INDEX IF NOT EXISTS mli_album on medialib (album);
CREATE INDEX IF NOT EXISTS mli_title on medialib (title);
CREATE INDEX IF NOT EXISTS mli_genre on medialib (genre);
CREATE INDEX IF NOT EXISTS mli_comment on medialib (comment);
CREATE INDEX IF NOT EXISTS mli_time_added on medialib (time_added);
CREATE INDEX IF NOT EXISTS mli_time_played on medialib (time_played);
CREATE INDEX IF NOT EXISTS mli_rating on medialib (rating);
CREATE INDEX IF NOT EXISTS mli_times_played on medialib (times_played);
// CREATE INDEX IF NOT EXISTS ml_ on medialib ();

  update medialib set url=?, info_updated=1, artist=?, album=?, title=?, track=?,
    year=?, genre=?, comment=?, length=?, filesize=?, lyrics_file=?

add Media
    insert into medialib values (nullptr, ?, ?, ?, ?,  ?, ?, ?, ?,  ?, ?, ?, ?,  ?, 0, 0, 0, ?);
                                 id                 title        comment      time_played
*/

#define SQL_CREATE_LIB_VER        "CREATE TABLE lib_ver\
(\
  version integer Primary Key\
);"

#define SQL_INSERT_VER_NUMB        "insert into lib_ver values (%d)"

#define SQL_CREATE_MEDIALIB        "CREATE TABLE IF NOT EXISTS medialib    \
(\
  id integer Primary Key,\
  url text COLLATE NOCASE,\
  file_deleted integer,\
  info_updated integer,\
  is_user_rating integer,\
  artist text COLLATE NOCASE,\
  album text COLLATE NOCASE,\
  title text COLLATE NOCASE,\
  track integer,\
  year integer,\
  genre text COLLATE NOCASE,\
  comment text COLLATE NOCASE,\
  length integer,\
  filesize integer,\
  time_added integer,\
  time_played integer,\
  rating integer,\
  times_played integer,\
  times_play_skipped integer,\
  lyrics_file text COLLATE NOCASE\
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
CREATE INDEX IF NOT EXISTS mli_times_played on medialib (times_played);"

#define SQL_MAX_MEDIALIB_ID        "select MAX(id) from medialib"

#define SQL_COUNT_OF_MEDIA    "select count(*) from medialib"


#define SQL_ADD_MEDIA    "insert into medialib values (nullptr, ?, 0, 1, 0, ?, ?,  ?, ?, ?, ?,  ?, ?, ?, ?,  -1, 300, 0, 0, ?);"

#define SQL_ADD_MEDIA_FAST    "insert into medialib (id, url, file_deleted, info_updated, is_user_rating, artist, title, time_added, time_played, rating, times_played, times_play_skipped) "\
                                "values (nullptr, ?, 0, 0, 1, ?, ?,   ?,  -1, 300, 0, 0);"

#define SQL_QUERY_MEDIA_BY_URL    "select * from medialib where url=? "


#define SQLITE3_BIND_TEXT(sqlstmt, strData)            \
    nRet = sqlstmt.bindText(n, strData.c_str(), strData.size());\
    if (nRet != ERR_OK)\
        goto RET_FAILED;\
    n++;

CMedia *newMedia()
{
    CMedia        *pMedia;

    pMedia = new CMedia;
    pMedia->addRef();

    return pMedia;
}

/*
How to calculate auto rating value:

default auto rating: 300
max auto rating: 590


*/
int getAutoRating(IMedia *pMedia)
{
    int            length, rating = 300;
    int            times_play_skipped, times_played;

    pMedia->getAttribute(MA_DURATION, &length);
    pMedia->getAttribute(MA_TIMES_PLAY_SKIPPED, &times_play_skipped);
    pMedia->getAttribute(MA_TIMES_PLAYED, &times_played);

    if (length > 30 * 1000)    // x seconds
    {
        times_play_skipped -= 1;

        if (times_play_skipped < 0)
            times_play_skipped = 0;

        int        value = times_played - times_play_skipped * 5;
        if (value > 0)
            rating = 400 + value;
        else if (value < 0)
            rating = 300 + value;

        if (rating > 590)
            rating = 589;
        else if (rating <= 100)
            rating = 101;
    }

    return rating;
}

void appendQeuryStatment(MediaLibOrderBy orderBy, int nTopN, string &strSql)
{
    if (orderBy == MLOB_NONE)
    {
    }
    else if (orderBy == MLOB_ARTIST)
        strSql += " order by artist";
    else if (orderBy == MLOB_ALBUM)
        strSql += " order by album";
    else if (orderBy == MLOB_GENRE)
        strSql += " order by genre";
    else if (orderBy == MLOB_ARTIST_ALBUM)
        strSql += " order by artist, album";
    else if (orderBy == MLOB_RATING)
        strSql += " order by rating desc";

    if (nTopN != -1)
    {
        strSql += " limit";
        strSql += itos(nTopN);
    }
}

//////////////////////////////////////////////////////////////////////////

class CMLQueryPlaylist
{
public:
    CMLQueryPlaylist()
    {
        m_pPlaylist = nullptr;
        m_pMediaLib = nullptr;
    }

    IPlaylist                *m_pPlaylist;
    CMediaLibrary            *m_pMediaLib;

    MLRESULT query(CMediaLibrary *pMediaLib, cstr_t szSQL)
    {
        if (!pMediaLib->isOK())
            return pMediaLib->m_nInitResult;

        int        nRet;

        m_pMediaLib = pMediaLib;

        m_pPlaylist = m_pMediaLib->newPlaylist();
        if (!m_pPlaylist)
            return ERR_NO_MEM;

        nRet = sqlite3_exec(
            m_pMediaLib->m_db.m_db,
            szSQL, 
            &queryCallback, 
            this, 
            nullptr);
        if (nRet != SQLITE_OK)
        {
            ERR_LOG2("Exe SQL:%S, : %S", szSQL, m_pMediaLib->m_db.errorMsg());
            m_pPlaylist->release();
            return ERR_SL_EXE_SQL;
        }

        return ERR_OK;
    }

    static int queryCallback(void *args, int numCols, char **results, char ** columnNames)
    {
        CMLQueryPlaylist    *pThis = (CMLQueryPlaylist*)args;

        CMedia        *pMedia = newMedia();

        pThis->m_pMediaLib->getMediaCallback(pMedia, numCols, results);

        pThis->m_pPlaylist->insertItem(-1, pMedia);

        pMedia->release();

        return 0;
    }

};


CMediaLibrary::CMediaLibrary()
{
    OBJ_REFERENCE_INIT;
}

CMediaLibrary::~CMediaLibrary()
{
    close();
}

MLRESULT CMediaLibrary::getAllArtist(IVXStr **ppvArtist)
{
    return queryVStr("select artist from medialib group by artist", ppvArtist);
}

MLRESULT CMediaLibrary::getAllAlbum(IVXStr **ppvAlbum)
{
    return queryVStr("select album from medialib group by album", ppvAlbum);
}

MLRESULT CMediaLibrary::getAllGenre(IVXStr **ppvAlbum)
{
    return queryVStr("select genre from medialib group by genre", ppvAlbum);
}

MLRESULT CMediaLibrary::getAllYear(IVInt **ppvYear)
{
    if (!isOK())
        return m_nInitResult;

    *ppvYear = nullptr;

    MutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt    sqlQuery;
    int                nRet;
    CVInt            *pvInt;

    sqlQuery.prepare(&m_db, "select year from medialib group by year");

    pvInt = new CVInt();
    pvInt->addRef();

    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW)
    {
        pvInt->push_back(sqlQuery.columnInt(0));

        nRet = sqlQuery.step();
    }
    if (pvInt->size() == 0)
    {
        pvInt->release();
        pvInt = nullptr;
        if (nRet == ERR_OK)
            nRet = ERR_NOT_FOUND;
    }
    else
        nRet = ERR_OK;

    *ppvYear = pvInt;

    return nRet;
}

MLRESULT CMediaLibrary::getAlbumOfArtist(LPCXSTR szArtist, IVXStr **ppvAlbum)
{
    if (!isOK())
        return m_nInitResult;

    MutexAutolock autolock(m_mutexDataAccess);

    CSqlite3Stmt    sqlQuery;
    int                nRet;
    string        strUCS2;
    CVXStr            *pvStr;
    cstr_t            text;

    *ppvAlbum = nullptr;

    nRet = sqlQuery.prepare(&m_db, "select album from medialib where artist=? group by album");
    if (nRet != ERR_OK)
        return nRet;

    pvStr = new CVXStr();
    pvStr->addRef();

    sqlQuery.bindStaticText(1, szArtist);

    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW)
    {
        text = sqlQuery.columnText(0);
        if (text) {
            pvStr->push_back(text);
        }

        nRet = sqlQuery.step();
    }
    if (pvStr->size() == 0)
    {
        pvStr->release();
        pvStr = nullptr;
        if (nRet == ERR_OK)
            nRet = ERR_NOT_FOUND;
    }
    else
        nRet = ERR_OK;

    *ppvAlbum = pvStr;

    return nRet;
}

uint32_t CMediaLibrary::getMediaCount()
{
    return sqlite_query_int_value(m_db.m_db, SQL_COUNT_OF_MEDIA);
}

MLRESULT CMediaLibrary::getMediaByUrl(LPCXSTR szUrl, IMedia **ppMedia)
{
    assert(ppMedia);

    if (!isOK())
        return m_nInitResult;

    *ppMedia = nullptr;

    MutexAutolock autolock(m_mutexDataAccess);

    m_sqlQueryByUrl.bindStaticText(1, szUrl);

    int nRet = m_sqlQueryByUrl.step();
    if (nRet == ERR_SL_OK_ROW)
    {
        CMedia        *media;

        media = newMedia();
        sqliteQueryMedia(&m_sqlQueryByUrl, media);
        *ppMedia = media;

        if (media->m_bIsFileDeleted)
        {
            // recover to undeleted.
            if (undeleteMedia(media->getID()) == ERR_OK)
                media->m_bIsFileDeleted = false;
        }
        nRet = ERR_OK;
    }
    else
        nRet = ERR_NOT_FOUND;

    m_sqlQueryByUrl.reset();

    return nRet;
}

MLRESULT CMediaLibrary::add(LPCXSTR szMediaUrl, IMedia **ppMedia)
{
    if (!isOK())
        return m_nInitResult;

    *ppMedia = nullptr;

    int                nRet;
    CMedia            *pMedia;

    nRet = getMediaByUrl(szMediaUrl, ppMedia);
    if (nRet != ERR_NOT_FOUND)
        return nRet;

    MutexAutolock autolock(m_mutexDataAccess);

    pMedia = newMedia();

    pMedia->setSourceUrl(szMediaUrl);

    m_player->loadMediaTagInfo(pMedia, true);

    pMedia->m_nTimePlayed.getCurrentTime();
    pMedia->m_nTimeAdded.getCurrentTime();

    nRet = doAddMedia(pMedia);
    if (nRet == ERR_OK)
        *ppMedia = pMedia;
    else
        pMedia->release();

    return ERR_OK;
}

// add media to media library fast, didn't update media info.
MLRESULT CMediaLibrary::addFast(LPCXSTR szMediaUrl, LPCXSTR szArtist, LPCXSTR szTitle, IMedia **ppMedia)
{
    if (!isOK())
        return m_nInitResult;

    int            nRet;
    int            n = 1;
    CMedia        *pMedia = nullptr;

    *ppMedia = nullptr;

    nRet = getMediaByUrl(szMediaUrl, ppMedia);
    if (nRet != ERR_NOT_FOUND)
        return nRet;

    MutexAutolock autolock(m_mutexDataAccess);

    pMedia = newMedia();

    pMedia->setSourceUrl(szMediaUrl);

    pMedia->m_nTimePlayed.getCurrentTime();
    pMedia->m_nTimeAdded.getCurrentTime();

    nRet = m_sqlAddFast.bindText(n++, szMediaUrl);
    if (nRet != ERR_OK)
        goto RET_FAILED;

    SQLITE3_BIND_TEXT(m_sqlAddFast, pMedia->m_strArtist);

    SQLITE3_BIND_TEXT(m_sqlAddFast, pMedia->m_strTitle);

    m_sqlAddFast.bindInt(n++, pMedia->m_nTimeAdded.m_time);

    nRet = m_sqlAddFast.step();
    if (nRet != ERR_OK)
        goto RET_FAILED;

    pMedia->m_nID =  m_db.LastInsertRowId();

    m_sqlAddFast.reset();

    *ppMedia = pMedia;

    return nRet;

RET_FAILED:
    if (pMedia)
        delete pMedia;

    return nRet;
}

// update Media Info to DB
MLRESULT CMediaLibrary::updateMediaInfo(IMedia *pMedia)
{
    if (!isOK())
        return m_nInitResult;

    assert(pMedia->getID() != MEDIA_ID_INVALID);
    if (pMedia->getID() == MEDIA_ID_INVALID)
        return ERR_ML_NOT_FOUND;

    MutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt    sqlQuery;
    int                nRet;
    int                n = 1;
    int                value;
    CXStr            str;

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
    sqlQuery.bindInt(n++, value);

    pMedia->getAttribute(MA_YEAR, &value);
    sqlQuery.bindInt(n++, value);

    pMedia->getAttribute(MA_GENRE, &str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    pMedia->getAttribute(MA_COMMENT, &str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    pMedia->getAttribute(MA_DURATION, &value);
    sqlQuery.bindInt(n++, value);

    pMedia->getAttribute(MA_FILESIZE, &value);
    sqlQuery.bindInt(n++, value);

    pMedia->getAttribute(MA_LYRICS_FILE, &str);
    SQLITE3_BIND_TEXT(sqlQuery, str);

    sqlQuery.bindInt(n++, pMedia->getID());
    nRet = sqlQuery.step();
    if (nRet == ERR_OK)
        pMedia->setInfoUpdatedToMediaLib(true);

RET_FAILED:
    return nRet;
}

// removes the specified item from the media library
MLRESULT CMediaLibrary::remove(IMedia **pMedia, bool bDeleteFile)
{
    if (!isOK())
        return m_nInitResult;

    MutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt    sqlQuery;
    int                nRet;

    sqlQuery.prepare(&m_db, "delete from medialib where id=?");

    sqlQuery.bindInt(1, (*pMedia)->getID());
    nRet = sqlQuery.step();
    if (nRet == ERR_OK)
    {
        if (bDeleteFile)
        {
            CXStr        str;
            (*pMedia)->getSourceUrl(&str);
            deleteFile(str.c_str());
        }

        (*pMedia)->release();
        *pMedia = nullptr;
    }

    return ERR_OK;
}

// If the media file was removed temporarily, set this flag on.
MLRESULT CMediaLibrary::setDeleted(IMedia **pMedia)
{
    char        szSql[256];
    int            nRet;

    snprintf(szSql, CountOf(szSql), "update medialib set file_deleted=1 where id=%d", (*pMedia)->getID());

    nRet = executeSQL(szSql);
    if (nRet == ERR_OK)
    {
        (*pMedia)->release();
        *pMedia = nullptr;
    }

    return nRet;
}

MLRESULT CMediaLibrary::getAll(IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
    CMLQueryPlaylist    query;
    int                    nRet;
    string                strSql;

    strSql = "select * from medialib where file_deleted=0";

    appendQeuryStatment(orderBy, nTopN, strSql);

    nRet = query.query(this, strSql.c_str());
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getByArtist(LPCXSTR szArtist, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
    string                strSql;

    strSql = "select * from medialib where artist=? and file_deleted=0";
    appendQeuryStatment(orderBy, nTopN, strSql);

    return queryPlaylist(strSql.c_str(), szArtist, ppPlaylist);
}

MLRESULT CMediaLibrary::getByAlbum(LPCXSTR szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
    string                strSql;

    strSql = "select * from medialib where album=? and file_deleted=0";
    appendQeuryStatment(orderBy, nTopN, strSql);

    return queryPlaylist(strSql.c_str(), szAlbum, ppPlaylist);
}

MLRESULT CMediaLibrary::getByAlbum(LPCXSTR szArtist, LPCXSTR szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
    CMLQueryPlaylist    query;
    int                    nRet;
    string            strArtist, strAlbum;
    string                strSql;

    strArtist = szArtist;
    strAlbum = szAlbum;

    strrep(strArtist, "'", "''");
    strrep(strAlbum, "'", "''");

    // char                szQuery[512];
    // sprintf(szQuery, "select * from medialib where artist='%s' and album='%s' and file_deleted=0", strArtist.c_str(), strAlbum.c_str());
    strSql = "select * from medialib where artist='";
    strSql += strArtist.c_str();
    strSql += "' and album='";
    strSql += strAlbum.c_str();
    strSql += "' and file_deleted=0";

    appendQeuryStatment(orderBy, nTopN, strSql);

    nRet = query.query(this, strSql.c_str());
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getByTitle(LPCXSTR szTitle, IPlaylist **ppPlaylist)
{
    return queryPlaylist("select * from medialib where title=? and file_deleted=0", szTitle, ppPlaylist);
}

MLRESULT CMediaLibrary::getByGenre(LPCXSTR szGenre, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
    string                strSql;

    strSql = "select * from medialib where genre=? and file_deleted=0";
    appendQeuryStatment(orderBy, nTopN, strSql);

    return queryPlaylist(strSql.c_str(), szGenre, ppPlaylist);
}

MLRESULT CMediaLibrary::getByYear(int nYear, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
    if (!isOK())
        return m_nInitResult;

    string            strSql;
    MutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt    sqlQuery;
    int                nRet;
    CPlaylist        *playlist = nullptr;

    strSql = "select * from medialib where year=? and file_deleted=0";
    appendQeuryStatment(orderBy, nTopN, strSql);

    nRet = sqlQuery.prepare(&m_db, strSql.c_str());
    if (nRet != ERR_OK)
        return nRet;

    playlist = newPlaylist();

    sqlQuery.bindInt(1, nYear);
    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW)
    {
        CMedia        *media;

        media = newMedia();
        sqliteQueryMedia(&sqlQuery, media);
        playlist->insertItem(-1, media);
        media->release();

        nRet = sqlQuery.step();
    }
    if (playlist->getCount() == 0)
    {
        playlist->release();
        playlist = nullptr;
        if (nRet == ERR_OK)
            nRet = ERR_NOT_FOUND;
    }
    else
        nRet = ERR_OK;

    *ppPlaylist = playlist;

    return nRet;
}

MLRESULT CMediaLibrary::getByRating(int nRating, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
    if (!isOK())
        return m_nInitResult;

    string            strSql;
    MutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt    sqlQuery;
    int                nRet;
    CPlaylist        *playlist = nullptr;
    int                nRatingStart, nRatingEnd;

    strSql = "select * from medialib where rating>=? and rating<? and file_deleted=0";
    appendQeuryStatment(orderBy, nTopN, strSql);

    nRatingStart = nRating * 100;
    nRatingEnd = (nRating + 1) * 100 - 1;

    nRet = sqlQuery.prepare(&m_db, strSql.c_str());
    if (nRet != ERR_OK)
        return nRet;

    playlist = newPlaylist();

    sqlQuery.bindInt(1, nRatingStart);
    sqlQuery.bindInt(2, nRatingEnd);
    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW)
    {
        CMedia        *media;

        media = newMedia();
        sqliteQueryMedia(&sqlQuery, media);
        playlist->insertItem(-1, media);
        media->release();

        nRet = sqlQuery.step();
    }
    if (playlist->getCount() == 0)
    {
        playlist->release();
        playlist = nullptr;
        if (nRet == ERR_OK)
            nRet = ERR_NOT_FOUND;
    }
    else
        nRet = ERR_OK;

    *ppPlaylist = playlist;

    return nRet;
}

MLRESULT CMediaLibrary::getRecentPlayed(uint32_t nCount, IPlaylist **ppPlaylist)
{
    CMLQueryPlaylist    query;
    char                szQuery[256];
    int                    nRet;

    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted=0 order by time_played desc limit %d", nCount);

    nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getTopPlayed(uint32_t nCount, IPlaylist **ppPlaylist)
{
    CMLQueryPlaylist    query;
    char                szQuery[256];
    int                    nRet;

    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted=0 order by times_played desc limit %d", nCount);

    nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getTopRating(uint32_t nCount, IPlaylist **ppPlaylist)
{
    CMLQueryPlaylist    query;
    char                szQuery[256];
    int                    nRet;

    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted=0 order by rating desc limit %d", nCount);

    nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getRecentAdded(uint32_t nCount, IPlaylist **ppPlaylist)
{
    CMLQueryPlaylist    query;
    char                szQuery[256];
    int                    nRet;

    snprintf(szQuery, CountOf(szQuery), "select * from medialib where file_deleted=0 order by time_added desc limit %d", nCount);

    nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getRecentPlayed(int nDayAgoBegin, int nDayAgoEnd, IPlaylist **ppPlaylist)
{
    CMLQueryPlaylist    query;
    char                szQuery[256];
    int                    nRet;
    CMPTime            dateBegin, dateEnd;
    CMPTime            dateCur;
    MPTM            tm;

    dateCur.getCurrentTime();
    dateCur.getMPTM(&tm);

    dateBegin.set(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 0, 0, 0);
    dateEnd = dateBegin;
    dateBegin -= CMPTimeSpan(nDayAgoBegin - 1, 0, 0, 0);
    dateEnd -= CMPTimeSpan(nDayAgoEnd - 1, 0, 0, 0);

    snprintf(szQuery, CountOf(szQuery), "select * from medialib where time_played between %d and %d and file_deleted=0 order by time_played", dateBegin.m_time, dateEnd.m_time);

    nRet = query.query(this, szQuery);
    *ppPlaylist = query.m_pPlaylist;

    return nRet;
}

MLRESULT CMediaLibrary::getRandom(uint32_t nCount, IPlaylist **ppPlaylist)
{
    if (!isOK())
        return m_nInitResult;
    return ERR_OK;
}

MLRESULT CMediaLibrary::getRandomByTime(uint32_t nDurationInMin, IPlaylist **ppPlaylist)
{
    if (!isOK())
        return m_nInitResult;
    return ERR_OK;
}

// 0-5, 0 for unrate.
MLRESULT CMediaLibrary::rate(IMedia *pMedia, uint32_t nRating)
{
    if (!isOK())
        return m_nInitResult;

    if (!pMedia)
        return ERR_OK;

    bool            bIsUserRating = (nRating != 0);
    char            szSql[256];

    if (bIsUserRating)
        nRating = nRating * 100 + 99;
    else
        nRating = getAutoRating(pMedia);

    MutexAutolock autolock(m_mutexDataAccess);

    snprintf(szSql, CountOf(szSql), "update medialib set is_user_rating=%d, rating=%d where id=%d", 
        bIsUserRating, nRating, pMedia->getID());

    pMedia->setAttribute(MA_RATING, nRating / 100);
    pMedia->setAttribute(MA_IS_USER_RATING, bIsUserRating);

    return executeSQL(szSql);
}

MLRESULT CMediaLibrary::updatePlayedTime(IMedia *pMedia)
{
    if (!isOK())
        return m_nInitResult;

    if (!pMedia || pMedia->getID() == MEDIA_ID_INVALID)
        return ERR_OK;

    int                value;
    char            szSql[256];

    MutexAutolock autolock(m_mutexDataAccess);

    pMedia->getAttribute(MA_TIME_PLAYED, &value);
    snprintf(szSql, CountOf(szSql), "update medialib set time_played=%d where id=%d", value, pMedia->getID());

    return executeSQL(szSql);
}

MLRESULT CMediaLibrary::markPlayFinished(IMedia *pMedia)
{
    if (!isOK())
        return m_nInitResult;

    if (!pMedia || pMedia->getID() == MEDIA_ID_INVALID)
        return ERR_OK;

    char            szSql[256];
    int                nRet;
    int                nTimesPlayed;
    int                bIsUserRating;

    MutexAutolock autolock(m_mutexDataAccess);

    pMedia->getAttribute(MA_TIMES_PLAYED, &nTimesPlayed);
    pMedia->setAttribute(MA_TIMES_PLAYED, nTimesPlayed + 1);
    pMedia->getAttribute(MA_IS_USER_RATING, &bIsUserRating);
    if (bIsUserRating)
        snprintf(szSql, CountOf(szSql), "update medialib set times_played=times_played+1 where id=%d", pMedia->getID());
    else
    {
        int        nRating = getAutoRating(pMedia);
        snprintf(szSql, CountOf(szSql), "update medialib set times_played=times_played+1, rating=%d where id=%d",
            nRating, pMedia->getID());
        pMedia->setAttribute(MA_RATING, nRating / 100);
    }

    nRet = executeSQL(szSql);

    updatePlayedTime(pMedia);

    return nRet;
}

MLRESULT CMediaLibrary::markPlaySkipped(IMedia *pMedia)
{
    if (!isOK())
        return m_nInitResult;

    if (!pMedia || pMedia->getID() == MEDIA_ID_INVALID)
        return ERR_OK;

    char            szSql[256];
    int                nRet;
    int                nTimesPlaySkipped;
    int                bIsUserRating;

    MutexAutolock autolock(m_mutexDataAccess);

    pMedia->getAttribute(MA_TIMES_PLAY_SKIPPED, &nTimesPlaySkipped);
    pMedia->setAttribute(MA_TIMES_PLAY_SKIPPED, nTimesPlaySkipped + 1);
    pMedia->getAttribute(MA_IS_USER_RATING, &bIsUserRating);
    if (bIsUserRating)
        snprintf(szSql, CountOf(szSql), "update medialib set times_play_skipped=times_play_skipped+1 where id=%d", pMedia->getID());
    else
    {
        int        nRating = getAutoRating(pMedia);
        snprintf(szSql, CountOf(szSql), "update medialib set times_play_skipped=times_play_skipped+1, rating=%d where id=%d", 
            nRating, pMedia->getID());
        pMedia->setAttribute(MA_RATING, nRating / 100);
    }

    nRet = executeSQL(szSql);

    return nRet;
}

int CMediaLibrary::init(CMPlayer *pPlayer)
{
    string strFile = getAppDataDir();
    strFile += "medialib.db";

    m_nInitResult = m_db.open(strFile.c_str());
    if (m_nInitResult != ERR_OK)
        goto INIT_FAILED;

    // create version table.
    m_nInitResult = m_db.exec(SQL_CREATE_LIB_VER);
    if (m_nInitResult != ERR_OK)
    {
        // Do convert old lib to current.
        m_nInitResult = convertLib();
    }
    else
    {
        // new version table created, insert version numb.
        char        szSql[MAX_PATH];

        snprintf(szSql, CountOf(szSql), SQL_INSERT_VER_NUMB, MEDIA_LIB_VER_CUR);
        m_nInitResult = m_db.exec(szSql);
        if (m_nInitResult != ERR_OK)
            return m_nInitResult;
    }

    // create medialib talbe
    m_nInitResult = m_db.exec(SQL_CREATE_MEDIALIB);
    if (m_nInitResult != ERR_OK)
        return m_nInitResult;

    m_nInitResult = m_sqlAdd.prepare(&m_db, SQL_ADD_MEDIA);
    if (m_nInitResult != ERR_OK)
        goto INIT_FAILED;

    m_nInitResult = m_sqlAddFast.prepare(&m_db, SQL_ADD_MEDIA_FAST);
    if (m_nInitResult != ERR_OK)
        goto INIT_FAILED;

    m_nInitResult = m_sqlQueryByUrl.prepare(&m_db, SQL_QUERY_MEDIA_BY_URL);
    if (m_nInitResult != ERR_OK)
        goto INIT_FAILED;

    m_player = pPlayer;

    return m_nInitResult;

INIT_FAILED:
    close();
    return m_nInitResult;
}

void CMediaLibrary::close()
{
    m_sqlAdd.finalize();
    m_sqlAddFast.finalize();
    m_sqlQueryByUrl.finalize();
    m_db.close();
    m_player.release();
}

MLRESULT CMediaLibrary::queryPlaylist(cstr_t szSQL, LPCXSTR szClause, IPlaylist **ppPlaylist)
{
    if (!isOK())
        return m_nInitResult;

    MutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt    sqlQuery;
    int                nRet;
    CPlaylist        *playlist = nullptr;

    nRet = sqlQuery.prepare(&m_db, szSQL);
    if (nRet != ERR_OK)
        return nRet;

    nRet = sqlQuery.bindText(1, szClause);
    if (nRet != ERR_OK)
        return nRet;

    playlist = newPlaylist();

    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW)
    {
        CMedia        *media;

        media = newMedia();
        sqliteQueryMedia(&sqlQuery, media);
        playlist->insertItem(-1, media);
        media->release();

        nRet = sqlQuery.step();
    }
    if (playlist->getCount() == 0)
    {
        playlist->release();
        playlist = nullptr;
        if (nRet == ERR_OK)
            nRet = ERR_NOT_FOUND;
    }
    else
        nRet = ERR_OK;

    *ppPlaylist = playlist;

    return nRet;
}

// 
MLRESULT CMediaLibrary::queryVStr(cstr_t szSql, IVXStr **ppvStr)
{
    if (!isOK())
        return m_nInitResult;

    *ppvStr = nullptr;

    MutexAutolock autolock(m_mutexDataAccess);
    CSqlite3Stmt    sqlQuery;
    int                nRet;
    string        strUCS2;
    CVXStr            *pvStr;
    cstr_t            text;

    sqlQuery.prepare(&m_db, szSql);

    pvStr = new CVXStr();
    pvStr->addRef();

    nRet = sqlQuery.step();
    while (nRet == ERR_SL_OK_ROW)
    {
        text = (const char *)sqlQuery.columnText(0);
        if (text) {
            pvStr->push_back(text);
        }

        nRet = sqlQuery.step();
    }
    if (pvStr->size() == 0)
    {
        pvStr->release();
        pvStr = nullptr;
        if (nRet == ERR_OK)
            nRet = ERR_NOT_FOUND;
    }
    else
        nRet = ERR_OK;

    *ppvStr = pvStr;

    return nRet;
}

MLRESULT CMediaLibrary::executeSQL(cstr_t szSql)
{
    return m_db.exec(szSql);
}

#define  MediaGetSqlite3ColumText(mediaAttri)    \
    text = (const char *)sqlStmt->columnText(n++);    \
    if (text)\
        mediaAttri = text;\
    else\
        mediaAttri.resize(0);

void CMediaLibrary::sqliteQueryMedia(CSqlite3Stmt *sqlStmt, CMedia *pMedia)
{
    int            n = 0;
    const char    *text;
    string    str;

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
    pMedia->m_nTimeAdded.m_time = sqlStmt->columnInt(n++);
    pMedia->m_nTimePlayed.m_time = sqlStmt->columnInt(n++);
    pMedia->m_nRating = sqlStmt->columnInt(n++) / 100;
    pMedia->m_nPlayed = sqlStmt->columnInt(n++);
    pMedia->m_nPlaySkipped = sqlStmt->columnInt(n++);
    MediaGetSqlite3ColumText(pMedia->m_strLyricsFile);
}

void CMediaLibrary::getMediaCallback(CMedia *pMedia, int numCols, char **results)
{
    int            n = 0;

#define FILED_GET_INT(feild)    \
    if (results[n])\
        feild = atoi(results[n]);\
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
    FILED_GET_INT(pMedia->m_nTimeAdded.m_time);
    FILED_GET_INT(pMedia->m_nTimePlayed.m_time);
    FILED_GET_INT(pMedia->m_nRating);
    pMedia->m_nRating /= 100;
    FILED_GET_INT(pMedia->m_nPlayed);
    FILED_GET_INT(pMedia->m_nPlaySkipped);
    FILED_GET_TEXT(pMedia->m_strLyricsFile);

    assert(n <= numCols);
}

MLRESULT CMediaLibrary::undeleteMedia(long nMediaID)
{
    char        szSql[256];

    snprintf(szSql, CountOf(szSql), "update medialib set file_deleted=0 where id=%d", nMediaID);

    return executeSQL(szSql);
}

MLRESULT CMediaLibrary::add(IMedia *pMedia)
{
    if (!isOK())
        return m_nInitResult;

    CMPAutoPtr<IMedia>    pMediaExist;
    int                nRet;
    CXStr            str;

    pMedia->getSourceUrl(&str);
    nRet = getMediaByUrl(str.c_str(), &pMediaExist);
    if (nRet != ERR_NOT_FOUND)
        return nRet;

    MutexAutolock autolock(m_mutexDataAccess);

    return doAddMedia((CMedia*)pMedia);
}

MLRESULT CMediaLibrary::doAddMedia(CMedia *pMedia)
{
    int                nRet;
    int                n = 1;

    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strUrl);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strArtist);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strAlbum);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strTitle);
    m_sqlAdd.bindInt(n++, pMedia->m_nTrackNumb);
    m_sqlAdd.bindInt(n++, pMedia->m_nYear);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strGenre);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strComment);
    m_sqlAdd.bindInt(n++, pMedia->m_nLength);
    m_sqlAdd.bindInt(n++, pMedia->m_nFileSize);
    m_sqlAdd.bindInt(n++, pMedia->m_nTimeAdded.m_time);
    SQLITE3_BIND_TEXT(m_sqlAdd, pMedia->m_strLyricsFile);

    nRet = m_sqlAdd.step();
    if (nRet != ERR_OK)
        pMedia->release();
    else
    {
        pMedia->m_nID =  m_db.LastInsertRowId();
        pMedia->setInfoUpdatedToMediaLib(true);
    }

    m_sqlAdd.reset();

RET_FAILED:

    return nRet;
}

CPlaylist *CMediaLibrary::newPlaylist()
{
    CPlaylist        *pPlaylist;

    pPlaylist = new CPlaylist(m_player);
    pPlaylist->addRef();

    return pPlaylist;
}

int CMediaLibrary::convertLib()
{
    return ERR_OK;
}
