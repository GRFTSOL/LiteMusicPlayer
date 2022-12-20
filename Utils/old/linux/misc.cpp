#include "../stdafx.h"
#include "../stringex_t.h"
#include "../CharEncoding.h"
#include "../misc.h"

int getOperationSystemType()
{
#ifdef WIN32
    OSVERSIONINFO        version;

    memset(&version, 0, sizeof(version));
    version.dwOSVersionInfoSize = sizeof(version);

    if (!GetVersionEx(&version))
    {
        // LOG1(LOG_LVL_ERROR, "GetVersionEx FAILED! Error Id: %d", getLastError);
        return OPS_UNKNOWN;
    }

    // Major version: 5, windows 2000
    if (version.dwMajorVersion == 3)
    {
        if (version.dwMinorVersion == 51)
            return OPS_WINNT351;
    }
    else if (version.dwMajorVersion == 4)
    {
        if (version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            if (version.dwMinorVersion == 0)
                return OPS_WIN95;
            else if (version.dwMinorVersion == 10)
                return OPS_WIN98;
            else if (version.dwMinorVersion == 90)
                return OPS_WINME;
            else
                return OPS_WIN9XMORE;
        }
        else if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if (version.dwMinorVersion == 0)
                return OPS_WINNT4;
        }
    }
    else if (version.dwMajorVersion == 5)
    {
        if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if (version.dwMinorVersion == 0)
                return OPS_WIN2000;
            else if (version.dwMinorVersion == 1)
                return OPS_WINXP;
            else
                return OPS_WINXPMORE;
        }
    }
    else
        return OPS_WINXPMORE;
#endif

#ifdef _LINUX
    return OPS_LINUX;
#endif

#ifdef _MAC_OS
    return OPS_MACOSX;
#endif

    return OPS_UNKNOWN;
}

bool isWin9xSystem()
{
    return (getOperationSystemType() <= OPS_WIN9XMORE);
}

#ifndef _WIN32

#if defined(_MAC_OS)

#include <sys/time.h>

uint32_t getTickCount()
{
    timeval tim;
    gettimeofday(&tim, nullptr);
    return (uint32_t)(tim.tv_sec * 1000 + tim.tv_usec / 1000);
}

#else // defined(_MAC_OS)

uint32_t getTickCount()
{
    tms tm;
    return times(&tm);
}

bool copyTextToClipboard(cstr_t szText)
{
    return false;
}

bool getClipBoardText(string &str)
{
    return false;
}

#endif

bool getMonitorRestrictRect(const CRect &rcIn, CRect &rcRestrict)
{
    return false;
}

void sleep(uint32_t dwMilliseconds)
{
    usleep((unsigned int)dwMilliseconds);
}

#endif

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

#ifdef WIN32

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
        while (nLen >= 0 && szPath[nLen] != PATH_SEP_CHAR)
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
                return;
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
#ifdef _UNICODE
        str = (const WCHAR *)GlobalLock(handle);
#else
        ucs2ToTString((const WCHAR *)GlobalLock(handle), -1, str);
#endif
        GlobalUnlock(handle);
    }
    else
    {
        handle = GetClipboardData(CF_TEXT);
        if (handle)
        {
            str = (cstr_t)GlobalLock(handle);
            GlobalUnlock(handle);
        }
    }
    CloseClipboard();

    return handle != nullptr;
}

#elif (defined(_LINUX))// _WIN32

bool copyTextToClipboard(cstr_t szText)
{
    GtkClipboard    *clipBoard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    if (!clipBoard)
        return false;

    gtk_clipboard_set_text(clipBoard, szText, strlen(szText));

    return true;
}

bool getClipBoardText(string &str)
{
    return false;
}

#endif

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

#define BMPWIDTHBYTES(dwWidth, dwBitCount)    ( ( dwWidth * dwBitCount + 7 ) / 8 + 3 ) & ~3

HBITMAP loadBmpFile(cstr_t szFile)
{
    uint32_t                dwQUADSize = 0;
    LPBITMAPINFO        lpbi = nullptr;                //BITMAPINFO (include infoheader and RGBQUAD)
    LPBITMAPINFOHEADER    lpbih = nullptr;
    HANDLE                hFile;
    uint32_t                dwFileLen, dwBytesRead;
    uint8_t                *lpbBmpData = nullptr;
    uint32_t                dwBiSize, dwDataSize;
    HDC                    hdc = nullptr;
    HBITMAP                hBmp = nullptr;

    //
    // first load bitmap from file to memory
    // 

    lpbi = (LPBITMAPINFO) new uint8_t[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
    lpbih = &lpbi->bmiHeader;

    hFile = CreateFile(szFile, GENERIC_READ, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, 
        OPEN_EXISTING, 0, nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        ERR_LOG1("open File:%s FAILED!", szFile);
        return nullptr;
    }

    dwFileLen = getFileSize(hFile, nullptr);

    if (dwFileLen <= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
    {
        ERR_LOG1("File:%s is not a Bitmap file. File size is less than BMP File Header size!", szFile);
        goto FAILED;
    }

    // read BITMAPFILEHEADER and judge whether it is a bmp file?
    BITMAPFILEHEADER bfh;

    if (!ReadFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwBytesRead, nullptr))
        goto READ_FILE_FAILED;

    uint16_t    wFlagBM;
    wFlagBM = MAKEWORD((uint8_t)'B',(uint8_t)'M');
    if (bfh.bfType != wFlagBM)
    {
        ERR_LOG1("File:%s is not a Bitmap file. File Header does not match TAG:BM!", szFile);
        goto FAILED;
    }

    if (! ReadFile(hFile, lpbih, sizeof(BITMAPINFOHEADER), &dwBytesRead, nullptr))
        goto READ_FILE_FAILED;

    switch (lpbih->biBitCount)
    {
    case 32:
    case 24:
    case 16:
        dwQUADSize = 0;//sizeof(RGBQUAD);
        break;
    case 8:                                
        dwQUADSize = sizeof(RGBQUAD) * (lpbih->biClrUsed ? lpbih->biClrUsed : 256);
        break;
    case 4:
        dwQUADSize = sizeof(RGBQUAD) * (lpbih->biClrUsed ? lpbih->biClrUsed : 16);
        break;
    case 2:
        dwQUADSize = sizeof(RGBQUAD) * (lpbih->biClrUsed ? lpbih->biClrUsed : 4);
        break;
    case 1:
        dwQUADSize = sizeof(RGBQUAD) * (lpbih->biClrUsed ? lpbih->biClrUsed : 2);
        break;
    }
    dwBiSize = sizeof(BITMAPINFOHEADER) + dwQUADSize;
    dwDataSize = dwFileLen - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER) - dwQUADSize;

    lpbBmpData = new uint8_t[dwDataSize];
    if (lpbBmpData == nullptr)
    {
        ERR_LOG1("alloc %d size of Memory FAILED!", dwDataSize);
        goto FAILED;
    }

    if (! ReadFile(hFile, lpbi->bmiColors, dwQUADSize, &dwBytesRead, nullptr))
        goto READ_FILE_FAILED;
    if (! ReadFile(hFile, lpbBmpData, dwDataSize, &dwBytesRead, nullptr))
        goto READ_FILE_FAILED;

    //
    // load bitmap from memory to HBITMAP
    //
    hdc = GetDC(nullptr);

    hBmp = CreateCompatibleBitmap(hdc, lpbih->biWidth, lpbih->biHeight);
    if (hBmp == nullptr)
    {
        ERR_LOG0("CreateCompatibleBitmap FAILED!");
        goto FAILED;
    }
    
    if (!SetDIBits(hdc, hBmp, 0, lpbih->biHeight, lpbBmpData, lpbi, DIB_RGB_COLORS))
    {
        ERR_LOG0("SetDIBits FAILED!");
        goto FAILED;
    }

    CloseHandle(hFile);

    ReleaseDC(nullptr, hdc);

    delete [](uint8_t *)lpbi;

    delete [] lpbBmpData;

    return hBmp;

READ_FILE_FAILED:
    ERR_LOG1("read File:%s FAILED!", szFile);
FAILED:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    if (lpbi)
        delete [](uint8_t *)lpbi;

    if (lpbBmpData)
        delete [] lpbBmpData;

    if (hdc)
        ReleaseDC(nullptr, hdc);

    if (hBmp)
        DeleteObject(hBmp);

    return nullptr;
}

*/
