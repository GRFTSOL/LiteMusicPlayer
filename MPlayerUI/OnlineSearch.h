// OnlineSearch.h: interface for the COnlineSearch class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../third-parties/sqlite/Sqlite3.hpp"


class ILyrSearchResultCacheDB
{
public:
    virtual bool open() = 0;

    virtual void close() = 0;

    virtual bool isOpened() = 0;

    virtual bool searchCache(cstr_t szFileAssociateKeyword, cstr_t szArtist, cstr_t szTitle, V_LRCSEARCHRESULT *pvResult, uint32_t *pdwSearchTime) = 0;
    virtual bool updateSearchLrcInfo(cstr_t szFileAssociateKeyword, MLMsgRetSearch &retSearch) = 0;

protected:
    void resultToXML(MLMsgRetSearch &retSearch, CXMLWriter &xmlWriter);
    void resultFromXML(MLMsgRetSearch &retSearch, cstr_t szXML);

};

class CLyrSearchResultCacheDB : public ILyrSearchResultCacheDB
{
public:
    CLyrSearchResultCacheDB();
    ~CLyrSearchResultCacheDB();

    virtual bool open();

    virtual void close();

    virtual bool isOpened() { return m_db.m_db != nullptr; }

    virtual bool searchCache(cstr_t szFileAssociateKeyword, cstr_t szArtist, cstr_t szTitle, V_LRCSEARCHRESULT *pvResult, uint32_t *pdwSearchTime);
    virtual bool updateSearchLrcInfo(cstr_t szFileAssociateKeyword, MLMsgRetSearch &retSearch);

protected:
    CSqlite3        m_db;

};

class CLyrSearchResultCacheMemDB : public ILyrSearchResultCacheDB
{
public:
    virtual bool open() { return true; }

    virtual void close() { m_listResults.clear(); }

    virtual bool isOpened() { return true; }

    virtual bool searchCache(cstr_t szFileAssociateKeyword, cstr_t szArtist, cstr_t szTitle, V_LRCSEARCHRESULT *pvResult, uint32_t *pdwSearchTime);
    virtual bool updateSearchLrcInfo(cstr_t szFileAssociateKeyword, MLMsgRetSearch &retSearch);

    static void free();

protected:
    struct ITEM
    {
        string        strKeyword;
        string        strXml;
    };
    typedef list<ITEM>        LIST_ITEMS;
    static LIST_ITEMS        m_listResults;

};

class COnlineSearch  
{
public:
    class INotifyEvent
    {
    public:
        virtual void onErrorSaveLyrics(int nIndex, cstr_t szMessage) = 0;
        virtual void onLyricsReturned(int nIndex, string &bufLyrics) = 0;
        virtual void onNoLyricsReturned(int nIndex) = 0;

    };

    struct COnlineSearchThreadParam
    {
        string        strTitle;
        string        strArtist;
        string        strAssociateMediaKey;
        bool        bOnlyMatched;

        COnlineSearch    *pOnlineSearch;
        COnlineSearchThreadParam()
        {
            pOnlineSearch = nullptr;
            bOnlyMatched = false;
        }
    };

    COnlineSearch();
    virtual ~COnlineSearch();

public:
    bool init();
    void quit();

    bool searchCacheForCur(V_LRCSEARCHRESULT *pvResult = nullptr, uint32_t *pdwSearchTime = nullptr);

    bool autoSearch();

    int searchOnline(CMLClientSession &session, cstr_t szArtist, cstr_t szTitle, cstr_t szMediaKey, V_LRCSEARCHRESULT &vResult, string &strMsg, int &nCurPage, int &nPageCount);
    int batchSearch(CMLClientSession &session, MLListSearchItems &listSearch, uint32_t nLyrSaveFlag, string &strMsg, INotifyEvent *pNotify);

    int batchSearch(CMLClientSession &session, MLListSearchItems &listSearch, MLMsgRetBatchSearch &retLyrics);

protected:
    int updateSearchLrcInfo(cstr_t szFileAssociateKeyword, MLMsgRetSearch &retSearch);
    bool addSearchJob(bool bOnlyMatched, cstr_t szMediaKey, cstr_t szArtist, cstr_t szTitle);

    static void threadSearchOnline(void *lpParam);

    std::mutex            m_mutexDB;
    bool            m_bRunning;
    CThread            m_threadSearch;

    CLyrSearchResultCacheDB        m_dbSearchResult;
    CLyrSearchResultCacheMemDB    m_dbSearchResultMem;
    //CLyricsInetSearch            m_lyricsInetSearch;

};

bool filterSpamInfo(string &artist, string &title);

extern COnlineSearch        g_OnlineSearch;
