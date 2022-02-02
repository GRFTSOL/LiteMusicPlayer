/********************************************************************
    Created  :    2003年11月2日 23:24:29
    FileName :    BaseFrmWork.cpp
    Author   :    xhy
    
    Purpose  :    应用程序的基本框架结构定义
*********************************************************************/

#include "Utils.h"
#include "App.h"

// 配置文件
class CProfile        g_profile;

// 日志
class CLog        g_log;

HINSTANCE        g_hInst = nullptr;            // Instance handle of the application

char            g_szWorkingFolder[MAX_PATH] = "";

void setWorkingFolder(cstr_t szWorkingFolder)
{
    // assert(szWorkingFolder && !isEmptyString(szWorkingFolder));
    strcpy_safe(g_szWorkingFolder, MAX_PATH, szWorkingFolder);
    dirStringAddSlash(g_szWorkingFolder);
}

void getAppResourceDir(char * szWorkingFolder)
{
    if (isEmptyString(g_szWorkingFolder))
    {
        getAppResourceDir();
    }

    strcpy_safe(szWorkingFolder, MAX_PATH, g_szWorkingFolder);
}

cstr_t getAppResourceDir()
{
    if (isEmptyString(g_szWorkingFolder))
    {
        assert(g_hInst);

        getModulePath(g_szWorkingFolder, g_hInst);

        dirStringAddSlash(g_szWorkingFolder);
    }

    return g_szWorkingFolder;
}

char            g_szAppDataDir[MAX_PATH] = "";

void getAppDataDir(char * szAppDataDir, cstr_t szDefAppName)
{
    OSVERSIONINFO    osvi;
    bool            bInUserDir = false;

    char szAppExeName[MAX_PATH];
    if (szDefAppName == nullptr)
    {
        char szAppExe[MAX_PATH];
        GetModuleFileName(nullptr, szAppExe, CountOf(szAppExe));
        strcpy_safe(szAppExeName, CountOf(szAppExeName), fileGetTitle(szAppExe).c_str());
        szDefAppName = szAppExeName;
    }

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (GetVersionEx(&osvi))
    {
        if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 5)
            bInUserDir = true;
    }
    if (bInUserDir)
    {
        string        strFile;
        
        //
        // PerUserSettings?
        //
        strFile = getAppResourceDir();
        strFile += "install.ini";
        if (isFileExist(strFile.c_str()))
        {
            CProfile    file;
            file.init(strFile.c_str(), "install");
            bInUserDir = file.getBool("PerUserSettings", true);
            if (!bInUserDir)
            {
                // To verify that it is writable.
                if (!canWriteInDir(getAppResourceDir()))
                    bInUserDir = true;
            }
        }
    }
    
    if (!bInUserDir)
    {
        getAppResourceDir(szAppDataDir);
    }
    else
    {
        if (SUCCEEDED(SHGetSpecialFolderPath(nullptr, szAppDataDir, CSIDL_APPDATA, false)))
        {
            dirStringAddSlash(szAppDataDir);
            strcat_safe(szAppDataDir, MAX_PATH, szDefAppName);
            if (!isDirExist(szAppDataDir))
                createDirectory(szAppDataDir, nullptr);
            dirStringAddSlash(szAppDataDir);
        }
        else
            getAppResourceDir(szAppDataDir);
    }
}

cstr_t getAppDataDir()
{
    if (isEmptyString(g_szAppDataDir))
    {
        getAppDataDir(g_szAppDataDir, nullptr);
    }

    return g_szAppDataDir;
}

void getAppDataDir(char *szAppDir)
{
    strcpy_safe(szAppDir, MAX_PATH, getAppDataDir());
}

#if defined (_DEBUG) && defined (_WIN32)
string getInstallShareDir()
{
    string strShareDir = getAppResourceDir();

    char        szFile[MAX_PATH];
    if (GetModuleFileName(getAppInstance(), szFile, CountOf(szFile)))
    {
        if (strcasecmp(fileGetName(szFile), "MiniLyrics.exe") == 0)
            strShareDir += "..\\Install_ML\\Share\\";
        else if (strcasecmp(fileGetName(szFile), "iPodLyricsDownloader.exe") == 0)
            strShareDir += "..\\Install_iPodLyricsDownloader\\Share\\";
        else if (strcasecmp(fileGetName(szFile), "WPOnline.exe") == 0)
            strShareDir += "..\\Install_WP\\Share\\";
        else
            strShareDir += "..\\Install_ZP\\Share\\";
    }

    return strShareDir;
}
#endif

HINSTANCE getAppInstance()
{
    return g_hInst;
}

void setAppInstance(HINSTANCE hInst)
{
    g_hInst = hInst;
}
/*
HINSTANCE getResourceHandle()
{
}

HINSTANCE setResourceHandle(HINSTANCE hResHandle)
{
}*/

static void _invalid_parameter(
                        const WCHAR * expression,
                        const WCHAR * function, 
                        const WCHAR * file, 
                        unsigned int line,
                        uintptr_t pReserved
                        )
{
}

bool initBaseFrameWork(HINSTANCE hInst, cstr_t szLogFile, cstr_t szProfileName, cstr_t szDefAppName)
{
    g_hInst = hInst;

    getAppDataDir(g_szAppDataDir, szDefAppName);

    g_profile.init(szProfileName, szDefAppName);
    g_log.init(szLogFile);

     _set_invalid_parameter_handler(_invalid_parameter);

    return true;
}
