#pragma once


/**
 * When music is playing, the media file can NOT be modified, so try to save/modify/remove
 * file later in the background. CAutoProcessEmbeddedLyrics is responsible for this.
 */
class CAutoProcessEmbeddedLyrics {
public:
    struct Item {
        string                      m_strSongFile;
        VecStrings                  m_vLyrNames;        // SZ_SONG_ID3V2_USLT, etc.
        bool                        m_bRemove = false;  // true: remove, false: add.
        uint8_t                     m_nTryFailed = 0;
        bool                        m_bDealOK = false;
        int                         m_nSucceededCount = 0;

        // save embedded lyrics
        string                      m_strLyrics;

        bool fromXML(SXNode *pNode);
        void toXML(CXMLWriter &xmlWriter);
    };
    using ListItems = list<Item>;

    CAutoProcessEmbeddedLyrics();
    virtual ~CAutoProcessEmbeddedLyrics();

    int saveEmbeddedLyrics(cstr_t szSongFile, const string &strLyrics, const VecStrings &vEmbeddedLyrNames);
    int removeEmbeddedLyrics(cstr_t szSongFile, const VecStrings &vEmbeddedLyrNames, bool bAddToQueueOnFailed = true);
    void removeJob(cstr_t szSongFile);

    void init();

    bool onPostQuit();
    void onSongChanged();

protected:
    friend class AutoProcessEmbeddedLyrics_AutoProcessEmbeddedLyricsLoad_Test;

    int dealEmbeddedLyrics(Item &item);
    int dealSaveEmbeddedLyrics(Item &item);
    int dealRemoveEmbeddedLyrics(Item &item);

    void saveJobs();
    void loadJobs(bool bDeleteFile);

    ListItems                   m_listJobs;
    std::mutex                  m_mutex;

};

extern CAutoProcessEmbeddedLyrics g_autoProcessEmbeddedLyrics;
