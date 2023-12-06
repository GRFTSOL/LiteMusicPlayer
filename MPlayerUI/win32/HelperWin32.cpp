

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT	0x0500
#include <windows.h>
#include <time.h>
#include <olectl.h>
#include "../MPlayerApp.h"


void analyseProxySetting(cstr_t szProxySetting, char szServer[], int nMaxSize, int &nPort);

void execute(Window *pWnd, cstr_t szExe, cstr_t szParam) {
    ShellExecute(pWnd != nullptr ? pWnd->getWndHandle() : nullptr, "open",
        szExe,
        szParam, nullptr, SW_SHOWNORMAL);
}

bool setClipboardText(Window *pWnd, cstr_t szText) {
    return tobool(copyTextToClipboard(szText));
}

bool SHDeleteFile(cstr_t szFile, Window *pWndParent) {
    SHFILEOPSTRUCT sop;
    string strFile;

    strFile = szFile;
    strFile += '\0';

    sop.hwnd = pWndParent->getWndHandle();
    sop.wFunc = FO_DELETE;
    sop.pFrom = strFile.c_str();
    sop.pTo = nullptr;
    sop.fFlags = FOF_ALLOWUNDO;
    sop.fAnyOperationsAborted = false;
    sop.hNameMappings = nullptr;
    sop.lpszProgressTitle = "";

    return (SHFileOperation(&sop) == 0 && !sop.fAnyOperationsAborted);
}

bool SHCopyFile(cstr_t szSrcFile, cstr_t szTargFile, Window *pWndParent) {
    SHFILEOPSTRUCT sop;
    string strSrc, strTarg;

    strSrc = szSrcFile;
    strTarg = szTargFile;
    szSrcFile += '\0';
    szTargFile += '\0';

    sop.hwnd = pWndParent->getWndHandle();
    sop.wFunc = FO_COPY;
    sop.pFrom = strSrc.c_str();
    sop.pTo = strTarg.c_str();
    sop.fFlags = FOF_ALLOWUNDO;
    sop.fAnyOperationsAborted = false;
    sop.hNameMappings = nullptr;
    sop.lpszProgressTitle = "";

    return (SHFileOperation(&sop) == 0 && !sop.fAnyOperationsAborted);
}

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif

bool setFileNoReadOnly(cstr_t szFile) {
    uint32_t dwAttr;
    dwAttr = GetFileAttributes(szFile);

    if (dwAttr != INVALID_FILE_ATTRIBUTES) {
        if (isFlagSet(dwAttr, FILE_ATTRIBUTE_READONLY)) {
            return SetFileAttributes(szFile, dwAttr & ~FILE_ATTRIBUTE_READONLY);
        } else {
            return true;
        }
    } else {
        return false;
    }
}

bool loadProxySvrFromIE(bool &bUseProxy, string &strSvr, int &nPort) {
    // 取得IE 的代理设置
    // [HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings]
    // REG_SZ: "ProxyServer"="127.1.1.1:80"
    // ftp=192.168.1.12:80;gopher=192.168.1.13:80;http=192.168.1.1:80;https=192.168.1.11:80
    HKEY hKeyInet;
    bool bRet = true;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        0, KEY_QUERY_VALUE, &hKeyInet) == ERROR_SUCCESS) {
        char szBuff[256];

        // ProxyEnable?
        DWORD dwType = REG_DWORD, dwEnable, nSize = sizeof(szBuff);
        if (RegQueryValueEx(hKeyInet, "ProxyEnable", nullptr, &dwType, (uint8_t *)&dwEnable, &nSize) != ERROR_SUCCESS) {
            bRet = false;
            bUseProxy = false;
        } else {
            bUseProxy = dwEnable != 0;
        }

        // ProxyServer ?
        dwType = REG_SZ;
        nSize = sizeof(szBuff);
        if (RegQueryValueEx(hKeyInet, "ProxyServer", nullptr, &dwType, (uint8_t *)szBuff, &nSize) != ERROR_SUCCESS) {
            bRet = false;
        } else {
            string http;
            cstr_t szBeg = strstr(szBuff, "http=");
            if (szBeg) {
                szBeg += 5;
                cstr_t szEnd = strchr(szBeg, ';');
                if (szEnd) {
                    http.assign(szBeg, (int)(szEnd - szBeg));
                } else {
                    http = szBeg;
                }
            } else {
                http = szBuff;
            }

            char szServer[256] = { 0 };
            analyseProxySetting(http.c_str(), szServer, CountOf(szServer), nPort);
            strSvr = szServer;
        }

        RegCloseKey(hKeyInet);
    } else {
        bRet = false;
    }

    return bRet;
}

void getNotepadEditor(string &strEditor) {
    char szBuffer[MAX_PATH] = { 0 };

    GetWindowsDirectoryA(szBuffer, MAX_PATH);
    
    strEditor = dirStringJoin(szBuffer, "notepad.exe");
    if (isFileExist(strEditor.c_str())) {
        return;
    }

    strEditor = dirStringJoin(szBuffer, "system32\\notepad.exe");
    if (isFileExist(strEditor.c_str())) {
        return;
    }

    strEditor = dirStringJoin(szBuffer, "system\\notepad.exe");
    if (isFileExist(strEditor.c_str())) {
        return;
    }
}
