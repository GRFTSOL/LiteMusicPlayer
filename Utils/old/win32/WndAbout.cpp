///////////////////////////////////////////////////
// the implementation of mylib

#include <tchar.h>
#include <Shlobj.h>
#include "WndAbout.h"
#include "safestr.h"


//////////////////////////////////////////////////
// functions


bool isTopmostWindow(HWND hWnd) {
    uint32_t dwStyleEx;

    dwStyleEx = (uint32_t)::GetWindowLong(hWnd, GWL_EXSTYLE);

    return ((dwStyleEx & WS_EX_TOPMOST) == WS_EX_TOPMOST);
}

void topmostWindow(HWND hwnd, bool bTopmost) {
    uint32_t dwStyleEx;
    if (bTopmost) {
        dwStyleEx = (uint32_t)::GetWindowLong(hwnd, GWL_EXSTYLE);
        dwStyleEx |= WS_EX_TOPMOST;
        ::SetWindowLong(hwnd, GWL_EXSTYLE, dwStyleEx);
        ::setWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
    } else {
        dwStyleEx = (uint32_t)::GetWindowLong(hwnd, GWL_EXSTYLE);
        dwStyleEx &= ~WS_EX_TOPMOST;
        ::SetWindowLong(hwnd, GWL_EXSTYLE, dwStyleEx);
        ::setWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
    }
}

// PURPOSE:
//      activate the windows
void activateWindow(HWND hWnd) {
    uint32_t dwTimeOutPrev = 0;

    sendMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    showWindow(hWnd, SW_SHOW);

    if (!isTopmostWindow(hWnd)) {
        setWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        setWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    }
    // save old
    SystemParametersInfo(/*SPI_GETFOREGROUNDLOCKTIMEOUT*/0x2000, 0, (void *)&dwTimeOutPrev, 0/* SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE*/);
    // set new
    SystemParametersInfo(/*SPI_SETFOREGROUNDLOCKTIMEOUT*/0x2001, 0, (void *)0, SPIF_SENDWININICHANGE/* | SPIF_UPDATEINIFILE*/);

    if (::GetForegroundWindow() != hWnd) {
        SetForegroundWindow(hWnd);
    }
    if (IsWindowEnabled(hWnd)) {
        SetActiveWindow(hWnd);
    }

    // restore old
    SystemParametersInfo(/*SPI_SETFOREGROUNDLOCKTIMEOUT*/0x2001, dwTimeOutPrev, (void *)0, SPIF_SENDWININICHANGE/* | SPIF_UPDATEINIFILE*/);
#endif
    //    SetActiveWindow(hWnd);
}

//
// remove it's WS_EX_APPWINDOW AND ADD WS_EX_TOOLWINDOW
bool showAsToolWindow(HWND hWnd) {
    uint32_t dwStyleEx;
    dwStyleEx = (uint32_t)GetWindowLong(hWnd, GWL_EXSTYLE);
    if (!isFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) && !isFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW)) {
        // not showed in task bar, so show it
        showWindow(hWnd, SW_HIDE);
        dwStyleEx |= WS_EX_TOOLWINDOW;// | WS_EX_PALETTEWINDOW;
        dwStyleEx &= ~WS_EX_APPWINDOW;
        SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
        //        setWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        showWindow(hWnd, SW_SHOW);
        return true;
    }
    return false;
}

// add it's WS_EX_APPWINDOW AND remove WS_EX_TOOLWINDOW
bool showAsAppWindow(HWND hWnd) {
    uint32_t dwStyleEx;
    dwStyleEx = (uint32_t)GetWindowLong(hWnd, GWL_EXSTYLE);
    if (isFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) || isFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW)) {
        // not showed in task bar, so show it
        showWindow(hWnd, SW_HIDE);
        dwStyleEx &= ~(WS_EX_TOOLWINDOW | WS_EX_PALETTEWINDOW);
        dwStyleEx |= WS_EX_APPWINDOW;
        SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
        //        setWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        showWindow(hWnd, SW_SHOW);
        return true;
    }
    return false;
}

bool isToolWindow(HWND hWnd) {
    uint32_t dwStyleEx;
    dwStyleEx = (uint32_t)GetWindowLong(hWnd, GWL_EXSTYLE);

    if ((dwStyleEx & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW/* | WS_EX_PALETTEWINDOW*/) {
        return true;
    }

    return false;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd,uint32_t uMsg,LPARAM lp, LPARAM pData) {
    char szDir[MAX_PATH];

    switch (uMsg) {
    case BFFM_INITIALIZED:
        {
            cstr_t szInitFolder;

            szInitFolder = (cstr_t)pData;

            if (szInitFolder && !isEmptyString(szInitFolder)) {
                // WParam is true since you are passing a path.
                // It would be false if you were passing a pidl.
                sendMessage(hwnd,BFFM_SETSELECTION,true,(LPARAM)szInitFolder);
            }
            break;
        }
    case BFFM_SELCHANGED:
        {
            // set the status window to the currently selected path.
            if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir)) {
                sendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
            }
            break;
        }
    default:
        break;
    }
    return 0;
}

bool browserForFolder(HWND hWnd, cstr_t szTitle, char * szPath, cstr_t szRootFoler) {
    LPMALLOC pMalloc;

    if (SHGetMalloc(&pMalloc) != NOERROR) {
        return false;
    }

    BROWSEINFO bInfo;
    LPITEMIDLIST pidl;

    ZeroMemory((PVOID)&bInfo, sizeof(bInfo));

    if (szRootFoler != nullptr && !isEmptyString(szRootFoler)) {
        OLECHAR olePath[MAX_PATH];
        ULONG chEaten;
        ULONG dwAttributes;
        HRESULT hr;
        LPSHELLFOLDER pDesktopFolder;

        // get a pointer to the Desktop's IShellFolder interface.
        if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder))) {
#ifndef UNICODE
            // IShellFolder::ParseDisplayName requires the file name be in Unicode
            MultiByteToWideChar(CP_ACP,
                MB_PRECOMPOSED,
                szRootFoler,
                -1,
                olePath,
                MAX_PATH
                );
#else
            strcpy_safe(olePath, CountOf(olePath), szRootFoler);
#endif

            // Convert the path to ITEMIDLIST
            hr = pDesktopFolder->ParseDisplayName(nullptr,
                nullptr,
                olePath,
                &chEaten,
                &pidl,
                &dwAttributes
                );
            if (FAILED(hr)) {
                goto BR_FAILED;
            }
            bInfo.pidlRoot = pidl;
        }
    }

    bInfo.hwndOwner = hWnd;
    bInfo.pszDisplayName = szPath;
    bInfo.lpszTitle = szTitle;
#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE  0x0040
#endif
    bInfo.ulFlags = /*BIF_EDITBOX | */BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;//BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;

    bInfo.lpfn = BrowseCallbackProc; // address of callback function.
    bInfo.lParam = (LPARAM)szPath; // pass address of object to callback function

    pidl = ::SHBrowseForFolder(&bInfo);
    if (pidl == nullptr) {
        goto BR_FAILED;
    }

    //    m_iImageIndex = bInfo.iImage;

    if (::SHGetPathFromIDList(pidl, szPath) == false) {
        goto BR_FAILED;
    }

    pMalloc->free(pidl);
    pMalloc->release();
    return true;

BR_FAILED:
    if (pMalloc) {
        pMalloc->free(pidl);
        pMalloc->release();
    }
    return false;
}

// COMMENT:
//        判断系统是否支持半透明窗口
//        只有Windows2000或以上才支持
bool isLayeredWndSupported() {
    OSVERSIONINFO version;

    memset(&version, 0, sizeof(version));
    version.dwOSVersionInfoSize = sizeof(version);

    if (!GetVersionEx(&version)) {
        //        LOGOUT1(LOG_LVL_ERROR, "GetVersionEx FAILED! Error Id: %d", getLastError);
        return false;
    }

    // Major version: 5, windows 2000 系列
    if (version.dwMajorVersion < 5) {
        return false;
    }

    return true;
}

//    COMMENT:
//        sets the opacity and transparency color key of a layered window
//    INPUT:
//        crKey,        specifies the color key
//        bAlpha,        value for the blend function, When bAlpha is 0,
//                    the window is completely transparent.
//                    When bAlpha is 255, the window is opaque.
//        dwFlags        Specifies an action to take. This parameter can be one or
//                    more of the following values. Value Meaning
//                    LWA_COLORKEY Use crKey as the transparency color.
//                    LWA_ALPHA Use bAlpha to determine the opacity of the layered window
bool setLayeredWindow(HWND hWnd, COLORREF crKey, uint8_t bAlpha, uint32_t dwFlags) {
    uint32_t dwStyle;

    // 首先设置WS_EX_LAYERED属性
    dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    dwStyle |= WS_EX_LAYERED;
    SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);

    return setLayeredWindowAttributes(hWnd, crKey, bAlpha, dwFlags);
}

void unSetLayeredWindow(HWND hWnd) {
    uint32_t dwStyle;

    // 首先设置WS_EX_LAYERED属性
    dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    dwStyle &= ~WS_EX_LAYERED;
    SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);
}

bool isChildWnd(HWND hWnd) {
    uint32_t dwStyleEx;

    dwStyleEx = (uint32_t)::GetWindowLong(hWnd, GWL_STYLE);
    return (dwStyleEx & WS_CHILD) == WS_CHILD;
}

void setParentByForce(HWND hWndChild, HWND hWndParent) {
    uint32_t dwStyle;
    dwStyle = GetWindowLong(hWndChild, GWL_STYLE);

    if (hWndParent) {
        dwStyle |= WS_CHILD;
    } else {
        dwStyle &= ~WS_CHILD;
    }

    SetWindowLong(hWndChild, GWL_STYLE, dwStyle);
    ::setParent(hWndChild, hWndParent);
}

bool hasCaption(HWND hWnd) {
    uint32_t dwStyleEx;

    dwStyleEx = (uint32_t)::GetWindowLong(hWnd, GWL_STYLE);
    return (dwStyleEx & WS_CAPTION) == WS_CAPTION;
}

void removeCaption(HWND    hWnd) {
    uint32_t dwStyleEx;

    dwStyleEx = (uint32_t)::GetWindowLong(hWnd, GWL_STYLE);
    dwStyleEx &= ~WS_CAPTION;
    ::SetWindowLong(hWnd, GWL_STYLE, dwStyleEx);
    ::setWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void addCaption(HWND    hWnd) {
    uint32_t dwStyleEx;

    dwStyleEx = (uint32_t)::GetWindowLong(hWnd, GWL_STYLE);
    if ((dwStyleEx & WS_CAPTION) != WS_CAPTION) {
        dwStyleEx |= WS_CAPTION;
        ::SetWindowLong(hWnd, GWL_STYLE, dwStyleEx);
        ::setWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

bool moveWindowSafely(HWND hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint) {
    if (isChildWnd(hWnd)) {
        CPoint pt;
        HWND hWndParent;
        hWndParent = getParent(hWnd);
        if (hWndParent) {
            pt.x = X;
            pt.y = Y;
            screenToClient(hWndParent, &pt);
            X = pt.x;
            Y = pt.y;
        }
    }

    bool bHasCap;
    bool bRet;

    bHasCap = hasCaption(hWnd);
    if (bHasCap) {
        removeCaption(hWnd);
    }

    bRet = moveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);

    if (bHasCap) {
        addCaption(hWnd);
    }

    return bRet;
}

bool enableDlgItem(HWND hWnd, int nIDItem, bool bEnable) {
    return tobool(::enableWindow(::getDlgItem(hWnd, nIDItem), bEnable));
}

bool showDlgItem(HWND hWnd, int nIDItem, int nCmdShow) {
    return tobool(::showWindow(::getDlgItem(hWnd, nIDItem), nCmdShow));
}

/*

// "All supported files (*.lrc; *.txt)\0*.lrc;*.txt\0Lyric File (*.lrc)\0*.LRC\0Text File (*.txt)\0*.TXT\0\0"
bool openFileDialog(HWND hWnd, HINSTANCE hInst, cstr_t extFilter, int nFilterIndex, char * szFile, int nFileNameBuffSize, cstr_t szTitle, uint32_t dwFlag)
{
    OPENFILENAME openfile;

    if (!isFileExist(szFile))
        emptyStr(szFile);

    openfile.lStructSize = sizeof(openfile);
    openfile.hwndOwner = hWnd;
    openfile.hInstance = hInst;
    openfile.lpstrFilter = extFilter;
    openfile.lpstrCustomFilter = nullptr;
    openfile.lpstrFile = szFile;
    openfile.nMaxFile = nFileNameBuffSize;
    openfile.lpstrFileTitle = nullptr;
    openfile.nMaxFileTitle = 0;
    openfile.lpstrInitialDir = nullptr;// g_szLyricInitDir;
    openfile.lpstrTitle = szTitle;
    openfile.Flags = dwFlag;
    openfile.nFileOffset = 0;
    openfile.nFileExtension = 0;
    openfile.lpstrDefExt = nullptr;
    openfile.nFilterIndex = nFilterIndex;
    return getOpenFileName(&openfile);
}
*/
