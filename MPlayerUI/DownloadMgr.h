/********************************************************************
    Created  :    2002/02/21    2:09
    FileName :    DownloadMgr.h
    Author   :    xhy
    
    Purpose  :    下载任务的关理类
*********************************************************************/

#if !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
#define AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_

#pragma once

#include "MLProfile.h"


enum DownloadTaskType
{
    DTT_LYRICS,
    DTT_CHECK_VERSION,
    DTT_CHECK_VERSION_NOUI,
    DTT_CHECK_AD,
};

class CDownloadTask
{
public:
    CDownloadTask(DownloadTaskType type, cstr_t szUrl)
    {
        taskType = type;
        m_strURL = szUrl;
        m_errResult = ERR_OK;
        m_nHttpRetCode = 0;
    }

    DownloadTaskType    taskType;
    string                m_strURL;
    string                strMediaKey;
    string                m_strLyrFileName;
    string            buffContent;

    MLRESULT            m_errResult;
    int                    m_nHttpRetCode;

};

class CDownloadMgr
{
public:
    CDownloadMgr();
    virtual ~CDownloadMgr();

    typedef    list<CDownloadTask *>        LIST_TASK;

public:
    int init();
    void quit();

    bool searchInCacheResult(bool bShowInfoText);

    void onSongChanged();

    void downloadLyrics(cstr_t szMediaKey, cstr_t szSongFile, cstr_t szUrl, cstr_t szFileName);
    void downloadVersionFile(cstr_t szUrl, bool bNoUI);

    void setDefSavePath(cstr_t szDefSavePath) { m_strDefSavePath = szDefSavePath; }
    cstr_t getDefSavePath() const { return m_strDefSavePath.c_str(); }

    bool autoDownloadEnabled() const { return m_bAutoDownload; }

    string getSaveLyricsFile(cstr_t szSongFile, cstr_t szFileName);
    string getSaveLyricsFile(cstr_t szSongFile, cstr_t szFileName, DOWN_SAVE_DIR DownSaveDir, DOWN_SAVE_NAME DownSaveName);

    int saveDownloadedLyrics(cstr_t szMediaFile, cstr_t szLyrFileName, const void *lpData, int nSize, uint32_t nLyrSaveFlag = LST_NONE);
    int saveDownloadedLyrAsFile(string &strFile, const void *lpData, int nSize);

protected:

    void addTask(CDownloadTask *pTask);

    bool isTaskExist(cstr_t szURL);

    void createDefaultLyricsDir(string &strDefSavePath);

    void onEndDownload(CDownloadTask *pTask);

    static void downloadThreadProc(void *lpParam);

    void downloadThread();

    int runHttpTask(CDownloadTask *pTask);

    int tryToConnect(class CHttpClient &httpClient);

    bool isQuiting() { return m_eventShutDown.acquire(0); }

protected:
    CThread                    m_threadDownload;
    std::mutex                    m_mutexAccess;
    LIST_TASK                m_listTasks, m_listRunningTasks;
    Event                    m_eventShutDown;
    string                    m_strDefSavePath;

public:
    bool                    m_bAutoDownload;

};


#endif // !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
