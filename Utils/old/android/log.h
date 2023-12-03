/********************************************************************
    Created  :    2002/02/01    20:08
    FileName :    log.h
    Author   :    xhy
*********************************************************************/
#pragma once

#include <android/log.h>


// log level
enum {
    LOG_LVL_ERROR               = ANDROID_LOG_ERROR,
    LOG_LVL_WARNING             = ANDROID_LOG_WARN,
    LOG_LVL_INFO                = ANDROID_LOG_INFO,
    LOG_LVL_DEBUG               = ANDROID_LOG_DEBUG,
};

class CLog {
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
    char                        m_szTag[32];
    char                        m_szSrcRootDir[128];

};
