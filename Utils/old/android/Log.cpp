

#include "../stdafx.h"

#include "log.h"
#include "../App.h"


#define MAX_MSG_LEN         1024

//////////////////////////////////////////////////////////////////////

static cstr_t ignoreRootDir(cstr_t szFile, cstr_t szRootDir) {
    cstr_t szRet = szFile;

    while (*szRet && *szRet == *szRootDir) {
        szRootDir++;
        szRet++;
    }

    if (*szRootDir == '\0') {
        return szRet;
    } else {
        return szFile;
    }
}


CLog::CLog() {
    _tcscpy(m_szTag, "log");
    emptyStr(m_szSrcRootDir);
}

CLog::~CLog() {
}

void CLog::writeLog(uint8_t byLevel, cstr_t szFile, int nLine, cstr_t szFormat, ...) {
#ifndef _DEBUG
    // 将发行版本中的调试信息去掉
    if (byLevel == LOG_LVL_DEBUG) {
        return;
    }
#endif    // _DEBUG

    szFile = ignoreRootDir(szFile, m_szSrcRootDir);

    va_list args;

    char szBuffer[MAX_MSG_LEN] = "";
    int nLen = 0;

    nLen = snprintf(szBuffer, CountOf(szBuffer) - 1, "%s %d ",
        szFile, nLine);

    // 格式化日志
    char *szMsg = nullptr;
    va_start(args, szFormat);
    szMsg = szBuffer + nLen;
    int nBuf = _vstprintf_s(szMsg, CountOf(szBuffer) - nLen, szFormat, args);
    va_end(args);

    // was there an error? was the expanded string too long?
    if (nBuf < 0 || nBuf >= CountOf(szBuffer) - nLen) {
        strcpy_safe(szMsg, CountOf(szBuffer) - nLen, "Insufficient log buffer space.");
    } else {
        szMsg[nBuf] = '\0';
    }

    nLen = (int)strlen(szBuffer) + 1;

    __android_log_write(byLevel, "Tag", szBuffer);
}

void CLog::logDumpStr(uint8_t byLevel, cstr_t szFile, int nLine, cstr_t szStr, int nStrLen) {
}

void CLog::init(cstr_t szLogFileName) {
    if (!szLogFileName) {
        return;
    }

    strcpy_safe(m_szTag, CountOf(m_szTag), fileGetTitle(szLogFileName).c_str());
}

void CLog::close() {
}

void CLog::setSrcRootDir(cstr_t szSrcFile, int nCurFileDeep) {
    cstr_t szDirEndPos = nullptr;

    szDirEndPos = szSrcFile + strlen(szSrcFile);

    while (szDirEndPos > szSrcFile) {
        if (*szDirEndPos == PATH_SEP_CHAR) {
            if (nCurFileDeep > 0) {
                nCurFileDeep--;
            } else {
                break;
            }
        }
        szDirEndPos--;
    }

    strncpysz_safe(m_szSrcRootDir, CountOf(m_szSrcRootDir), szSrcFile, int(szDirEndPos - szSrcFile));
}
