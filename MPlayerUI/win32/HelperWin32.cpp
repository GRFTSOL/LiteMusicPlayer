// Helper.cpp: implementation of the Helper class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "Helper.h"
#include <time.h>
#include <olectl.h>

void analyseProxySetting(cstr_t szProxySetting, char szServer[], int nMaxSize, int &nPort);

void execute(Window *pWnd, cstr_t szExe, cstr_t szParam)
{
    shellExecute(pWnd != nullptr ? pWnd->getHandle() : nullptr, "open", 
        szExe,
        szParam, nullptr, SW_SHOWNORMAL);
}

bool setClipboardText(Window *pWnd, cstr_t szText)
{
    return tobool(copyTextToClipboard(szText));
}

bool SHDeleteFile(cstr_t szFile, Window *pWndParent)
{
    if (isWin9xSystem())
    {
        SHFILEOPSTRUCTA sop;
        string strFile = szFile;
        strFile += '\0';

        sop.hwnd = pWndParent->getHandle();
        sop.wFunc = FO_DELETE;
        sop.pFrom = strFile.c_str();
        sop.pTo = nullptr;
        sop.fFlags = FOF_ALLOWUNDO;
        sop.fAnyOperationsAborted = false;
        sop.hNameMappings = nullptr;
        sop.lpszProgressTitle = "";

        return (SHFileOperationA(&sop) == 0 && !sop.fAnyOperationsAborted);
    }
    else
    {
        SHFILEOPSTRUCT    sop;
        string            strFile;

        strFile = szFile;
        strFile += '\0';

        sop.hwnd = pWndParent->getHandle();
        sop.wFunc = FO_DELETE;
        sop.pFrom = strFile.c_str();
        sop.pTo = nullptr;
        sop.fFlags = FOF_ALLOWUNDO;
        sop.fAnyOperationsAborted = false;
        sop.hNameMappings = nullptr;
        sop.lpszProgressTitle = "";

        return (SHFileOperation(&sop) == 0 && !sop.fAnyOperationsAborted);
    }
}

bool SHCopyFile(cstr_t szSrcFile, cstr_t szTargFile, Window *pWndParent)
{
    if (isWin9xSystem())
    {
        SHFILEOPSTRUCTA    sop;
        string            strSrc, strTarg;

        strSrc = szSrcFile;
        strTarg = szTargFile;
        strSrc += '\0';
        strTarg += '\0';

        sop.hwnd = pWndParent->getHandle();
        sop.wFunc = FO_COPY;
        sop.pFrom = strSrc.c_str();
        sop.pTo = strTarg.c_str();
        sop.fFlags = FOF_ALLOWUNDO;
        sop.fAnyOperationsAborted = false;
        sop.hNameMappings = nullptr;
        sop.lpszProgressTitle = "";

        return (SHFileOperationA(&sop) == 0 && !sop.fAnyOperationsAborted);
    }
    else
    {
        SHFILEOPSTRUCT    sop;
        string            strSrc, strTarg;

        strSrc = szSrcFile;
        strTarg = szTargFile;
        szSrcFile += '\0';
        szTargFile += '\0';

        sop.hwnd = pWndParent->getHandle();
        sop.wFunc = FO_COPY;
        sop.pFrom = strSrc.c_str();
        sop.pTo = strTarg.c_str();
        sop.fFlags = FOF_ALLOWUNDO;
        sop.fAnyOperationsAborted = false;
        sop.hNameMappings = nullptr;
        sop.lpszProgressTitle = "";

        return (SHFileOperation(&sop) == 0 && !sop.fAnyOperationsAborted);
    }
}

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES    0xFFFFFFFF
#endif

bool setFileNoReadOnly(cstr_t szFile)
{
    uint32_t    dwAttr;
    dwAttr = GetFileAttributes(szFile);

    if (dwAttr != INVALID_FILE_ATTRIBUTES)
    {
        if (isFlagSet(dwAttr, FILE_ATTRIBUTE_READONLY))
            return setFileAttributes(szFile, dwAttr & ~FILE_ATTRIBUTE_READONLY);
        else
            return true;
    }
    else
        return false;
}

bool loadProxySvrFromIE(bool &bUseProxy, string &strSvr, int &nPort)
{
    // 取得IE 的代理设置
    // [HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings]
    // REG_SZ: "ProxyServer"="127.1.1.1:80"
    // ftp=192.168.1.12:80;gopher=192.168.1.13:80;http=192.168.1.1:80;https=192.168.1.11:80
    HKEY    hKeyInet;
    bool    bRet = true;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        0, KEY_QUERY_VALUE, &hKeyInet) == ERROR_SUCCESS)
    {
        uint32_t        nSize, dwType, dwEnable;
        char        szBuff[256];
        nSize = sizeof(szBuff);

        // ProxyEnable?
        dwType = REG_DWORD;
        nSize = sizeof(dwEnable);
        if (RegQueryValueEx(hKeyInet, "ProxyEnable", nullptr, &dwType, (uint8_t *)&dwEnable, &nSize) != ERROR_SUCCESS)
        {
            bRet = false;
            bUseProxy = false;
        }
        else
        {
            bUseProxy = dwEnable != 0;
        }

        // ProxyServer ?
        dwType = REG_SZ;
        nSize = sizeof(szBuff);
        if (RegQueryValueEx(hKeyInet, "ProxyServer", nullptr, &dwType, (uint8_t *)szBuff, &nSize) != ERROR_SUCCESS)
        {
            bRet = false;
        }
        else
        {
            char    szServer[256] = "";
            char    szHttp[256];
            cstr_t    szBeg, szEnd;
            szBeg = strstr(szBuff, "http=");
            if (szBeg)
            {
                szBeg += 5;
                szEnd = strchr(szBeg, ';');
                if (szEnd)
                    strncpysz_safe(szHttp, CountOf(szHttp), szBeg, (int)(szEnd - szBeg));
                else
                    strcpy_safe(szHttp, CountOf(szHttp), szBeg);
            }
            else
                strcpy_safe(szHttp, CountOf(szHttp), szBuff);

            analyseProxySetting(szHttp, szServer, CountOf(szServer), nPort);
            strSvr = szServer;
        }

        RegCloseKey(hKeyInet);
    }
    else
        bRet = false;

    return bRet;
}


static bool findStringValue(cstr_t szBuff, cstr_t szName, cstr_t szEndTag, string &strValue)
{
    cstr_t        szBeg, szEnd;

    strValue.resize(0);

    szBeg = strstr(szBuff, szName);
    if (!szBeg)
        return false;

    szBeg += strlen(szName);

    szEnd = strstr(szBeg, szEndTag);
    if (!szEnd)
        return false;

    strValue.append(szBeg, szEnd);

    return true;
}

bool loadProxySvrFromFireFox(bool &bUseProxy, string &strSvr, int &nPort)
{
    char        szFireFoxDir[MAX_PATH];

    if (!SUCCEEDED(SHGetSpecialFolderPath(nullptr, szFireFoxDir, CSIDL_APPDATA, false)))
        return false;

    dirStringAddSlash(szFireFoxDir);
    strcat_safe(szFireFoxDir, CountOf(szFireFoxDir), "Mozilla\\Firefox\\");
    // C:\Documents and Settings\username\Application Data\Mozilla\Firefox

    string        strProfileFile, strProfilePath;
    CProfile    profile;

    strProfileFile = szFireFoxDir;
    strProfileFile += "profiles.ini";
    if (!isFileExist(strProfileFile.c_str()))
        return false;

    profile.init(strProfileFile.c_str());

    {
        //
        // get current profile path.
        //

        int        nDefault = 0;
        // get current profile folder.
        for (int i = 0; i < 10; i++)
        {
            if (1 == profile.getInt(CStrPrintf("Profile%d", i).c_str(), "Default", 0))
            {
                // Find the default profile.
                nDefault = i;
                break;
            }
        }

        string        strPath = profile.getString(CStrPrintf("Profile%d", nDefault).c_str(), "Path", "");
        if (strPath.empty())
            return false;

        if (strPath.size() > 2 && strPath[1] == ':')
            strProfilePath = strPath;
        else
        {
            strProfilePath = szFireFoxDir;
            strProfilePath += strPath;
        }
        dirStringAddSlash(strProfilePath);
    }

    // get proxy settings.
    string        strPrefJSFile;
    string    strPrefJS;

    strPrefJSFile = strProfilePath;
    strPrefJSFile += "prefs.js";

    if (!readFileByBom(strPrefJSFile.c_str(), strPrefJS))
        return false;

    string        strProxy, strPort, strType;

    // user_pref("network.proxy.http", "192.168.1.112");
    // user_pref("network.proxy.http_port", 80);
    // user_pref("network.proxy.type", 1);
    findStringValue(strPrefJS.c_str(), "user_pref(\"network.proxy.http\", \"", "\";"), strSvr);
    findStringValue(strPrefJS.c_str(), "user_pref(\"network.proxy.http_port\", ", ");", strPort);
    findStringValue(strPrefJS.c_str(), "user_pref(\"network.proxy.type\", ", ");", strType);

    if (atoi(strType.c_str()) == 1)
    {
        nPort = atoi(strPort.c_str());
        return true;
    }

    return false;
}

void getNotepadEditor(string &strEditor)
{
    char        szBuffer[MAX_PATH];

    GetWindowsDirectory(szBuffer, MAX_PATH);
    dirStringAddSlash(szBuffer);

    strEditor = szBuffer;
    strEditor += "notepad.exe";
    if (isFileExist(strEditor.c_str()))
        return;

    strEditor = szBuffer;
    strEditor += "system32\\notepad.exe";
    if (isFileExist(strEditor.c_str()))
        return;

    strEditor = szBuffer;
    strEditor += "system\\notepad.exe";
    if (isFileExist(strEditor.c_str()))
        return;
}

uint32_t getSecCount()
{
    return getTickCount() / 1000;
}
