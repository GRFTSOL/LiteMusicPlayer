#pragma once


class CAutoProcessEmbeddedLyrics  
{
public:
    struct Item
    {
        string                m_strSongFile;
        VecStrings            m_vLyrNames;    // SZ_SONG_ID3V2_USLT, etc.
        bool                m_bRemove;        // true: remove, false: add.
        uint8_t                m_nTryFailed;
        bool                m_bDealOK;
        int                    m_nSucceededCount;

        // save embedded lyrics
        string            m_strLyrics;

        Item()
        {
            m_bDealOK = false;
            m_nTryFailed = 0;
            m_nSucceededCount = 0;
        }

        bool fromXML(SXNode *pNode);
        void toXML(CXMLWriter &xmlWriter);
    };
    typedef list<Item>        LIST_ITEMS;

    CAutoProcessEmbeddedLyrics();
    virtual ~CAutoProcessEmbeddedLyrics();

    int saveEmbeddedLyrics(cstr_t szSongFile, cstr_t szLyricsFile, string *pBufLyrics, VecStrings &vEmbeddedLyrNames, bool bAddToQueueOnFailed = true, int *succeededCount = nullptr);
    int removeEmbeddedLyrics(cstr_t szSongFile, VecStrings &vEmbeddedLyrNames, bool bAddToQueueOnFailed = true);
    void removeJob(cstr_t szSongFile);

    void init();

    bool onPostQuit();
    void onSongChanged();

protected:
    friend class CDlgBatchProcessEmbeddedLyrics;
    friend class CTestCaseCAutoProcessEmbeddedLyrics;

    int dealEmbeddedLyrics(Item &item);
    int dealSaveEmbeddedLyrics(Item &item);
    int dealRemoveEmbeddedLyrics(Item &item);

    void saveJobs();
    void loadJobs(bool bDeleteFile);

    LIST_ITEMS                m_listJobs;
    std::mutex                    m_mutex;

};

extern CAutoProcessEmbeddedLyrics        g_autoProcessEmbeddedLyrics;
