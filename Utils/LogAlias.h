#pragma once

#include "Log.h"

extern class CLog        g_log;

#define LOG0(byLevel, szMessage)            \
    g_log.writeLog(byLevel, __FILE__, __LINE__, szMessage)

#define LOG1(byLevel, sz, p1)            \
    g_log.writeLog(byLevel, __FILE__, __LINE__, sz, p1)

#define LOG2(byLevel, sz, p1, p2)            \
    g_log.writeLog(byLevel, __FILE__, __LINE__, sz, p1, p2)

#define LOG3(byLevel, sz, p1, p2, p3)            \
    g_log.writeLog(byLevel, __FILE__, __LINE__, sz, p1, p2, p3)

#define ERR_LOG0(szMessage)            \
    g_log.writeLog(LOG_LVL_ERROR, __FILE__, __LINE__, szMessage)

#define ERR_LOG1(sz, p1)            \
    g_log.writeLog(LOG_LVL_ERROR, __FILE__, __LINE__, sz, p1)

#define ERR_LOG2(sz, p1, p2)            \
    g_log.writeLog(LOG_LVL_ERROR, __FILE__, __LINE__, sz, p1, p2)

#define ERR_LOG3(sz, p1, p2, p3)            \
    g_log.writeLog(LOG_LVL_ERROR, __FILE__, __LINE__, sz, p1, p2, p3)

#define ERR_LOGDUMPS(sz, len)            \
    g_log.logDumpStr(LOG_LVL_ERROR, __FILE__, __LINE__, sz, len)

#ifdef _DEBUG
#define DBG_LOG0(szMessage)            \
    g_log.writeLog(LOG_LVL_DEBUG, __FILE__, __LINE__, szMessage)

#define DBG_LOG1(sz, p1)            \
    g_log.writeLog(LOG_LVL_DEBUG, __FILE__, __LINE__, sz, p1)

#define DBG_LOG2(sz, p1, p2)            \
    g_log.writeLog(LOG_LVL_DEBUG, __FILE__, __LINE__, sz, p1, p2)

#define DBG_LOG3(sz, p1, p2, p3)            \
    g_log.writeLog(LOG_LVL_DEBUG, __FILE__, __LINE__, sz, p1, p2, p3)

#define DBG_LOGDUMPS(sz, len)            \
    g_log.logDumpStr(LOG_LVL_DEBUG, __FILE__, __LINE__, sz, len)

#else
#define DBG_LOG0(szMessage)            (0)

#define DBG_LOG1(sz, p1)                (0)

#define DBG_LOG2(sz, p1, p2)            (0)

#define DBG_LOG3(sz, p1, p2, p3)        (0)

#define DBG_LOGDUMPS(sz, len)        (0)

#endif
