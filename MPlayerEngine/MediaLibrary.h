// MediaLibrary.h: interface for the CMediaLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEDIALIBRARY_H__12D1F10C_DB4B_4E0A_AE95_30251D39B91B__INCLUDED_)
#define AFX_MEDIALIBRARY_H__12D1F10C_DB4B_4E0A_AE95_30251D39B91B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../third-parties/sqlite/Sqlite3.hpp"

#include "IMPlayer.h"
#include "Playlist.h"


class CMPlayer;

class CMediaLibrary : public IMediaLibrary  
{
    OBJ_REFERENCE_DECL
public:
    CMediaLibrary();
    virtual ~CMediaLibrary();

    virtual MLRESULT getAllArtist(IVString **ppvArtist);

    virtual MLRESULT getAllAlbum(IVString **ppvAlbum);

    virtual MLRESULT getAllGenre(IVString **ppvAlbum);

    virtual MLRESULT getAllYear(IVInt **ppvYear);

    virtual MLRESULT getAlbumOfArtist(cstr_t szArtist, IVString **ppvAlbum);

    virtual uint32_t getMediaCount();

    virtual MLRESULT getMediaByUrl(cstr_t szUrl, IMedia **pMedia);

    virtual MLRESULT add(cstr_t szMediaUrl, IMedia **ppMedia);

    // add media to media library fast, didn't update media info.
    virtual MLRESULT addFast(cstr_t szMediaUrl, cstr_t szArtist, cstr_t szTitle, IMedia **ppMedia);

    virtual MLRESULT updateMediaInfo(IMedia *pMedia);

    // removes the specified item from the media library
    virtual MLRESULT remove(IMedia **pMedia, bool bDeleteFile);

    // If the media file was removed temporarily, set this flag on.
    virtual MLRESULT setDeleted(IMedia **pMedia);

    virtual MLRESULT getAll(IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

    virtual MLRESULT getByArtist(cstr_t szArtist, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

    virtual MLRESULT getByAlbum(cstr_t szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

    virtual MLRESULT getByAlbum(cstr_t szArtist, cstr_t szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

    virtual MLRESULT getByTitle(cstr_t szTitle, IPlaylist **ppPlaylist);

    virtual MLRESULT getByGenre(cstr_t szGenre, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

    virtual MLRESULT getByYear(int nYear, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

    virtual MLRESULT getByRating(int nRating, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

    virtual MLRESULT getRecentPlayed(uint32_t nCount, IPlaylist **ppPlaylist);

    virtual MLRESULT getRecentPlayed(int nDayAgoBegin, int nDayAgoEnd, IPlaylist **ppPlaylist);

    virtual MLRESULT getTopPlayed(uint32_t nCount, IPlaylist **ppPlaylist);

    virtual MLRESULT getTopRating(uint32_t nCount, IPlaylist **ppPlaylist);

    virtual MLRESULT getRecentAdded(uint32_t nCount, IPlaylist **ppPlaylist);

    virtual MLRESULT getRandom(uint32_t nCount, IPlaylist **ppPlaylist);

    virtual MLRESULT getRandomByTime(uint32_t nDurationInMin, IPlaylist **ppPlaylist);

    // 0-5, 0 for unrate.
    virtual MLRESULT rate(IMedia *pMedia, uint32_t nRating);

    virtual MLRESULT updatePlayedTime(IMedia *pMedia);
    virtual MLRESULT markPlayFinished(IMedia *pMedia);
    virtual MLRESULT markPlaySkipped(IMedia *pMedia);

public:
    int init(CMPlayer *pPlayer);
    void close();

    bool isOK() { return m_nInitResult == ERR_OK; }

    MLRESULT queryPlaylist(cstr_t szSQL, cstr_t szClause, IPlaylist **ppPlaylist);

    MLRESULT queryVStr(cstr_t szSql, IVString **ppvStr);

    MLRESULT executeSQL(cstr_t szSql);

    void sqliteQueryMedia(CSqlite3Stmt *sqlStmt, CMedia *pMedia);

    void getMediaCallback(CMedia *pMedia, int numCols, char **results);

    MLRESULT undeleteMedia(long nMediaID);

    virtual MLRESULT add(IMedia *pMedia);

protected:
    MLRESULT doAddMedia(CMedia *pMedia);

    CPlaylist *newPlaylist();

    int convertLib();

protected:
    friend class CMLQueryPlaylist;

    CMPAutoPtr<CMPlayer>        m_player;

    CSqlite3                    m_db;
    std::mutex                  m_mutexDataAccess;

    CSqlite3Stmt        m_sqlAdd, m_sqlAddFast, m_sqlQueryByUrl;
    int                    m_nInitResult;

};

#endif // !defined(AFX_MEDIALIBRARY_H__12D1F10C_DB4B_4E0A_AE95_30251D39B91B__INCLUDED_)
