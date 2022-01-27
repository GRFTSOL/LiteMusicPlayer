#include "stringex_t.h"
#include "CharEncoding.h"
#include "misc.h"

/*

bool enumIPAddr(vector<string> &vIps)
{
    //////////////////
    // get host name.
    //
    char hostname[256];
    int res = gethostname(hostname, sizeof(hostname));
    if (res != 0)
    {
        ERR_LOG1("gethostname FAILED! error: %d", WSAGetLastError());
        return false;
    }

    ////////////////
    // get host info for hostname. 
    //
    hostent* pHostent = gethostbyname(hostname);
    if (pHostent==nullptr)
    {
        ERR_LOG1("gethostbyname FAILED! error: %d", WSAGetLastError());
        return false;
    }

    //////////////////
    // parse the hostent information returned
    //
    hostent& he = *pHostent;
    //printf("name=%s\naliases=%s\naddrtype=%d\nlength=%d\n",
    //    he.h_name, he.h_aliases, he.h_addrtype, he.h_length);
    
    sockaddr_in sa;
    for (int nAdapter=0; he.h_addr_list[nAdapter]; nAdapter++) {
        memcpy ( &sa.sin_addr.s_addr, he.h_addr_list[nAdapter],he.h_length);
        // printf("Address: %s\n", inet_ntoa(sa.sin_addr)); // display as string
        USES_CONVERSION;
        vIps.push_back(A2T(inet_ntoa(sa.sin_addr)));
        DBG_LOG1("Ip of the host: %s", vIps.back().c_str());
    }
    
    return true;
}
*/
/*

void drawBlueCtrl(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    char        szString[256];

    getWindowText(lpDrawItemStruct->hwndItem, szString, 256);

    setTextColor(lpDrawItemStruct->hDC, RGB(0, 0, 255));
    SetBkMode(lpDrawItemStruct->hDC, TRANSPARENT);
    DrawText(lpDrawItemStruct->hDC, szString, strlen(szString), &lpDrawItemStruct->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}
*/

bool executeCmdAndWait(cstr_t szCmd, uint32_t dwTimeOut, uint32_t *pExitCode)
{
    STARTUPINFO            startInfo;
    PROCESS_INFORMATION    procInfo;
    bool                bRet = true;

    memset(&startInfo, 0, sizeof(startInfo));
    memset(&procInfo, 0, sizeof(procInfo));

    if (!CreateProcess(nullptr, (char *)szCmd, nullptr, nullptr, false, 0, nullptr, nullptr, &startInfo, &procInfo))
        return false;

    if (dwTimeOut != 0 && WaitForSingleObject(procInfo.hProcess, dwTimeOut) == WAIT_TIMEOUT)
    {
        TerminateProcess(procInfo.hProcess, 0);
        bRet = false;
    }

    if (pExitCode && bRet)
        GetExitCodeProcess(procInfo.hProcess, pExitCode);

    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

    return bRet;
}

// PURPOSE   : get full task save file name by task name.
//
// PARAMETER: 
//      szPath is the path that returned: format is like : "c:\\antivirus\\"
bool getModulePath(char szPath[], HINSTANCE hInstance/* = nullptr*/)
{
    __try
    {
        int nLen;

        emptyStr(szPath);

        nLen = GetModuleFileName(hInstance, szPath, MAX_PATH);
        nLen --;
        while (nLen >= 0 && szPath[nLen] != DIR_SLASH)
            nLen --;
        if (nLen < 0)
            return false;
        
        szPath[nLen + 1] = '\0';
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
    
    return true;
}

bool getMonitorRestrictRect(const CRect &rcIn, CRect &rcRestrict)
{
    if (getOperationSystemType() >= OPS_WIN98)
    {
typedef struct tagMONITORINFO {
  uint32_t  cbSize; 
  CRect   rcMonitor; 
  CRect   rcWork; 
  uint32_t  dwFlags; 
} MONITORINFO, *LPMONITORINFO;
typedef bool (WINAPI *FUNGetMonitorInfo)(HANDLE hMonitor, LPMONITORINFO lpmi);
typedef HANDLE (WINAPI *FUNMonitorFromRect)(CRect *lprc, uint32_t dwFlags);
#ifndef MONITOR_DEFAULTTONULL
#define MONITOR_DEFAULTTONULL    0
#endif
        static FUNMonitorFromRect    funMonitorFromRect = nullptr;
        static FUNGetMonitorInfo    funGetMonitorInfo = nullptr;

        if (!funMonitorFromRect || !funGetMonitorInfo)
        {
            static HMODULE        hModule = nullptr;
            if (!hModule)
                hModule = LoadLibrary("user32.dll");
            if (hModule)
            {
                funMonitorFromRect = (FUNMonitorFromRect)GetProcAddress(hModule, "MonitorFromRect");
                funGetMonitorInfo = (FUNGetMonitorInfo)GetProcAddress(hModule, "GetMonitorInfoA");
            }
        }
        if (funMonitorFromRect && funGetMonitorInfo)
        {
            HANDLE        hMonitor;
            MONITORINFO    monitorInfo;
            monitorInfo.cbSize = sizeof(monitorInfo);
            hMonitor = funMonitorFromRect(&rcIn, MONITOR_DEFAULTTONULL);
            if (hMonitor)
            {
                funGetMonitorInfo(hMonitor, &monitorInfo);
                rcRestrict = monitorInfo.rcMonitor;
                return true;
            }
        }
    }

    rcRestrict.top = rcRestrict.left = 0;
    rcRestrict.bottom = GetSystemMetrics(SM_CYSCREEN);
    rcRestrict.right = GetSystemMetrics(SM_CXSCREEN);
    
    return true;
}

void centerWindowToMonitor(HWND hWnd)
{
    CRect    rc, rcRestrict;
    int        w, h;

    getWindowRect(hWnd, &rc);
    getMonitorRestrictRect(rc, rcRestrict);

    w = rc.right - rc.left;
    h = rc.bottom - rc.top;

    rc.left = rcRestrict.left + (rcRestrict.right - rcRestrict.left - w) / 2;
    rc.top = rcRestrict.top + (rcRestrict.bottom - rcRestrict.top - h) / 2;

    setWindowPos(hWnd, nullptr, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

bool copyTextToClipboard(cstr_t szText)
{
    int        nLen;
    HGLOBAL hglbCopy;
    char *    lptstrCopy;
    
    if (!OpenClipboard(nullptr))
        return false;
    EmptyClipboard();
    
    nLen = strlen(szText);
    
    hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (nLen + 1) * sizeof(char));
    
    if (hglbCopy == nullptr) 
    {
        CloseClipboard();
        return false;
    }
    
    lptstrCopy = (char *)GlobalLock(hglbCopy);
    memcpy(lptstrCopy, szText,
        nLen * sizeof(char));
    lptstrCopy[nLen] = (char) 0;    // null character
    GlobalUnlock(hglbCopy);
    
    // Place the handle on the clipboard. 
    
#ifdef _UNICODE
    SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
    SetClipboardData(CF_TEXT, hglbCopy);
#endif

    CloseClipboard();

    return true;
}

bool getClipBoardText(string &str)
{
    HANDLE        handle;
    OpenClipboard(nullptr);
    handle = GetClipboardData(CF_UNICODETEXT);
    if (handle)
    {
        ucs2ToUtf8((const WCHAR *)GlobalLock(handle), -1, str);
        GlobalUnlock(handle);
    }
    else
    {
        handle = GetClipboardData(CF_TEXT);
        if (handle)
        {
            convertStr((cstr_t)GlobalLock(handle), -1, str);
            GlobalUnlock(handle);
        }
    }
    CloseClipboard();

    return handle != nullptr;
}
