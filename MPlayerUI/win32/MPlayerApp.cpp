

#include "../MPlayerApp.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "OnlineSearch.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "MPFloatingLyrWnd.h"
#include "LyricShowObj.h"
#include "../../GfxLite/win32/GdiplusGraphicsLite.h"
#include "CrashRptDlg.h"
#include "MPMsg.h"

// #define _MEM_LEAK_CHECK

#if defined(_DEBUG) && defined(_MEM_LEAK_CHECK)
//
// Go link below to check, how to use this
// http://www.codeproject.com/KB/applications/leakfinder.aspx
//
#include "Stackwalker.h"
#endif


bool terminateAnotherInstance() {
    CProcessEnum processEnum;
    uint32_t vProcessID[1024];
    uint32_t nSizeReturned;

    if (!processEnum.enum(vProcessID, sizeof(vProcessID), &nSizeReturned)) {
        return false;
    }

    char szSelfName[MAX_PATH] = "";
    GetModuleFileName(nullptr, szSelfName, CountOf(szSelfName));
    uint32_t dwCurrentPID = GetCurrentProcessId();

    nSizeReturned /= sizeof(uint32_t);
    for (int i = 0; i < nSizeReturned; i++) {
        if (vProcessID[i] == dwCurrentPID) {
            continue;
        }
        string strFile = processEnum.getProcessFileName(vProcessID[i]);
        if (strcmp(strFile.c_str(), szSelfName) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, vProcessID[i]);
            if (hProcess != nullptr) {
                ERR_LOG0("Terminate another running instance.");
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
                return true;
            }
        }
    }

    return false;
}
/*
    cstr_t        szToMoveFiles[] = { "MiniLyric.ini", "MLLyrics.S2L", "FontColorCustom.the",
        "dbSearchCache.db", "dbLyrUpload.db" };
    string        strMLIniOld, strMLIniNew;

    for (int i = 0; i < CountOf(szToMoveFiles); i++)
    {
        strMLIniOld = getAppResourceDir();
        strMLIniOld += szToMoveFiles[i];
        strMLIniNew = szAppDataDir;
        strMLIniNew += szToMoveFiles[i];
        if (isFileExist(strMLIniOld.c_str()))
            moveFile(strMLIniOld.c_str(), strMLIniNew.c_str());
    }
*/

int WINAPI _tWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    char *    lpCmdLine,
    int       nCmdShow) {
    setAppInstance(hInstance);

    initMiniDumper();

    DBG_LOG1("MiniLyrics command line: %s", lpCmdLine);

    // Only allow one copy of MiniLyrics.exe running, iPod Lyrics Downloader and MiniLyrics can't be running at same time.

    CMPlayerApp *pApp = CMPlayerAppBase::getInstance();
    if (pApp->isAnotherInstanceRunning()) {
        HWND hWndMsg = findWindow(MSG_WND_CLASS_NAME, nullptr);
        if (hWndMsg) {
            sendActivateMainWnd(hWndMsg);

            if (!isEmptyString(lpCmdLine)) {
                DBG_LOG0("Another Instance is running, send command line to it.");
                sendCommandLine(hWndMsg, lpCmdLine);
            }
            return 0;
        } else {
            terminateAnotherInstance();
        }
    }

    if (!CMPlayerAppBase::getInstance()->init()) {
        return 1;
    }

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CMPlayerAppBase::getInstance()->quit();

    return msg.wParam;
}


CMPlayerApp::CMPlayerApp() {
    m_hMutexRuning = nullptr;
#ifndef _MPLAYER
    m_bReloadEmbeddedTheme = false;
#endif
}

CMPlayerApp::~CMPlayerApp() {
}

bool CMPlayerApp::isAnotherInstanceRunning() {
    assert(!m_hMutexRuning);
    HANDLE hMutexRuning = CreateMutex(nullptr, true, SZ_MUTEX_RUNNING);
    int nLastErr = getLastError();

    CloseHandle(hMutexRuning);

    return nLastErr == ERROR_ALREADY_EXISTS;
}

bool CMPlayerApp::setRunningFlag() {
    // Is Product already running?
    assert(!m_hMutexRuning);
    m_hMutexRuning = CreateMutex(nullptr, true, SZ_MUTEX_RUNNING);
    if (getLastError() == ERROR_ALREADY_EXISTS ) {
        CloseHandle(m_hMutexRuning);
        m_hMutexRuning = nullptr;
        return false;
    }

    return true;
}

cstr_t getAppIniFile() {
    //
    // init Base frame, save log and settings in app dir
    return SZ_PROFILE_NAME;
}

bool CMPlayerApp::init() {
    cstr_t lpCmdLine = GetCommandLine();

    if (isAnotherInstanceRunning()) {
        HWND hWndMsg = findWindow(MSG_WND_CLASS_NAME, nullptr);
        if (hWndMsg) {
            sendActivateMainWnd(hWndMsg);

            if (!isEmptyString(lpCmdLine)) {
                DBG_LOG0("Another Instance is running, send command line to it.");
                sendCommandLine(hWndMsg, lpCmdLine);
            }
            return false;
        } else {
            terminateAnotherInstance();
        }
    }

#if defined(_DEBUG) && defined(_MEM_LEAK_CHECK)
    InitAllocCheck(ACOutput_XML);
#endif

    g_log.setSrcRootDir(__FILE__, 2);

    char szWorkingFolder[MAX_PATH] = {0};
    getModulePath(szWorkingFolder, getAppInstance());
    setWorkingFolder(szWorkingFolder);

    initBaseFrameWork(getAppInstance(), "log.txt", getAppIniFile(), SZ_SECT_UI);
    setFileNoReadOnly(g_profile.getFile());

    g_LangTool.setMacro(SZ_MACRO_PRODUCT_NAME, SZ_APP_NAME);
    g_LangTool.setMacro(SZ_MACRO_COMPANY_NAME, SZ_COMPANY_NAME);

    LOG1(LOG_LVL_INFO, "start %s.", getAppNameLong().c_str());

    // init Winsock, Ole, Common Controls, gdiplus
    WSADATA WsaData;
    if (WSAStartup(0x0101, &WsaData) != 0) {
        ERR_LOG1("WSAStartup FAILED: %s", OSError().Description());
    }

    INITCOMMONCONTROLSEX initData;
    initData.dwSize = sizeof(initData);
    initData.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
    InitCommonControlsEx(&initData);

    // init gdiplus
    OleInitialize(nullptr);

    CGdiplusGraphicsLite::Startup();

    return CMPlayerAppBase::init();
}

void CMPlayerApp::quit() {
    if (!m_hMutexRuning) {
        return;
    }

    //
    // 先设置此标志表示退出，其他地方会用到根据此标志判断
    CloseHandle(m_hMutexRuning);
    m_hMutexRuning = nullptr;

    CMPlayerAppBase::quit();

#ifdef _WIN32
    CGdiplusGraphicsLite::Shutdown();

    OleUninitialize();
#endif
}
