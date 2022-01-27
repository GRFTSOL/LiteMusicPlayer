/********************************************************************
    Created  :    2002/02/01    20:08
    FileName :    log.h
    Author   :    xhy
*********************************************************************/
#pragma once

#include <mutex>


// log level
enum
{
    LOG_LVL_ERROR                = 2,
    LOG_LVL_WARNING                = 3,
    LOG_LVL_INFO                = 4,
    LOG_LVL_DEBUG                = 5,
};

class CLog  
{
protected:
    FILE        *m_fpLog;
    string      m_logFile;
    string      m_srcRootDir;

    std::mutex  m_mutex;

public:
    CLog();
    virtual ~CLog();

public:
    void init(cstr_t szLogFileName);
    void close();

    void setSrcRootDir(cstr_t szSrcFile, int nCurFileDeep = 1);

    void writeLog(uint8_t byLevel, cstr_t szFile, int nLine, cstr_t szFormat, ...);
    void logDumpStr(uint8_t byLevel, cstr_t szFile, int nLine, cstr_t szStr, int nStrLen);

protected:
    bool openLogFile();

};
