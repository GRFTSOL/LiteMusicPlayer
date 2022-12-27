/********************************************************************
    Created  :    2003年11月2日 23:24:29
    FileName :    BaseFrmWork.cpp
    Author   :    xhy

    Purpose  :    应用程序的基本框架结构定义
*********************************************************************/

#include "../stdafx.h"
#include "../Utils.h"
#include "../App.h"


// 配置文件
class CProfile    g_profile;

// 日志
class CLog        g_log;

char g_szWorkingFolder[MAX_PATH] = "";
char g_szAppDataDir[MAX_PATH] = "";

void setWorkingFolder(cstr_t szWorkingFolder) {
    // assert(szWorkingFolder && !isEmptyString(szWorkingFolder));
    strcpy_safe(g_szWorkingFolder, MAX_PATH, szWorkingFolder);
    dirStringAddSep(g_szWorkingFolder);
}

void getAppResourceDir(char * szWorkingFolder) {
    if (isEmptyString(g_szWorkingFolder)) {
        getAppResourceDir();
    }

    _tcscpy(szWorkingFolder, g_szWorkingFolder);
}

cstr_t getAppResourceDir() {
    return g_szWorkingFolder;
}

cstr_t getAppDataDir() {
    if (isEmptyString(g_szAppDataDir)) {
        getAppResourceDir(g_szAppDataDir);
    }

    return g_szAppDataDir;
}

void getAppDataDir(char *szAppDir) {
    strcpy_safe(szAppDir, MAX_PATH, getAppDataDir());
}

void initSetDefaultCharEncoding();

#ifdef _ANDROID

bool initBaseFrameWork(cstr_t szWorkingFolder, cstr_t szLogTag, cstr_t szProfileName, cstr_t szDefAppName) {
    assert(sizeof(uint8_t) == 1);
    assert(sizeof(uint16_t) == 2);
    assert(sizeof(uint32_t) == 4);
    assert(sizeof(int16_t) == 2);
    assert(sizeof(int32_t) == 4);
    assert(sizeof(int64_t) == 8);
    assert(sizeof(WCHAR) == 2);
    assert(sizeof(WCHAR) == 2);

    strcpy_safe(g_szAppDataDir, CountOf(g_szAppDataDir), szWorkingFolder);
    strcpy_safe(g_szWorkingFolder, CountOf(g_szWorkingFolder), szWorkingFolder);
    dirStringAddSep(g_szWorkingFolder);
    dirStringAddSep(g_szAppDataDir);

    g_profile.init(szProfileName, szDefAppName);

    g_log.init(szLogTag);

    return true;
}

#else

bool initBaseFrameWork(int argc, char *argv[], cstr_t szLogFile, cstr_t szProfileName, cstr_t szDefAppName) {
    assert(sizeof(uint8_t) == 1);
    assert(sizeof(uint16_t) == 2);
    assert(sizeof(uint32_t) == 4);
    assert(sizeof(int16_t) == 2);
    assert(sizeof(int32_t) == 4);
    assert(sizeof(int64_t) == 8);
    assert(sizeof(WCHAR) == 2);
    assert(sizeof(WCHAR) == 2);

    {
        int nLen;

        _tcscpy(g_szWorkingFolder, argv[0]);
        nLen = strlen(g_szWorkingFolder);
        nLen --;
        while (nLen >= 0 && g_szWorkingFolder[nLen] != PATH_SEP_CHAR) {
            nLen --;
        }
        if (nLen < 0) {
            return false;
        }

        g_szWorkingFolder[nLen + 1] = '\0';
    }

    g_profile.init(szProfileName, szDefAppName);

    g_log.init(szLogFile);

    return true;
}

#endif
