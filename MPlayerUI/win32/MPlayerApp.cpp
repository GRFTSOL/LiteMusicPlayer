// MPlayerApp.cpp: implementation of the CMPlayerApp class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerAppBase.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "OnlineSearch.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "MPFloatingLyrWnd.h"
#include "LyricShowObj.h"
#include "../../GfxLite/win32/GdiplusGraphicsLite.h"

#include "../base/win32/ProcessEnum.h"
#ifndef _MPLAYER
#include "../MiniLyrics/win32/MLPlayerMgr.h"
#include "../MiniLyrics/win32/PlayerPluginInstaller.h"
#include "../MiniLyrics/win32/DlgSetupPlayerPlugins.h"
#endif

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

bool terminateAnotherInstance()
{
    CProcessEnum    processEnum;
    uint32_t            vProcessID[1024];
    uint32_t            nSizeReturned;

    if (!processEnum.enum(vProcessID, sizeof(vProcessID), &nSizeReturned))
        return false;

    char    szSelfName[MAX_PATH] = "";
    GetModuleFileName(nullptr, szSelfName, CountOf(szSelfName));
    uint32_t dwCurrentPID = GetCurrentProcessId();

    nSizeReturned /= sizeof(uint32_t);
    for (int i = 0; i < nSizeReturned; i++)
    {
        if (vProcessID[i] == dwCurrentPID)
            continue;
        string strFile = processEnum.getProcessFileName(vProcessID[i]);
        if (strcmp(strFile.c_str(), szSelfName) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, vProcessID[i]);
            if (hProcess != nullptr)
            {
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
                     int       nCmdShow)
{
    setAppInstance(hInstance);

    initMiniDumper();

    DBG_LOG1("MiniLyrics command line: %s", lpCmdLine);

    // Only allow one copy of MiniLyrics.exe running, iPod Lyrics Downloader and MiniLyrics can't be running at same time.

    CMPlayerApp *pApp = CMPlayerAppBase::getInstance();
    if (pApp->isAnotherInstanceRunning())
    {
        HWND        hWndMsg = findWindow(MSG_WND_CLASS_NAME, nullptr);
        if (hWndMsg)
        {
            sendActivateMainWnd(hWndMsg);

            if (!isEmptyString(lpCmdLine))
            {
                DBG_LOG0("Another Instance is running, send command line to it.");
                sendCommandLine(hWndMsg, lpCmdLine);
            }
            return 0;
        }
        else
            terminateAnotherInstance();
    }

    if (!CMPlayerAppBase::getInstance()->init())
        return 1;

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CMPlayerAppBase::getInstance()->quit();

    return msg.wParam;
}


CMPlayerApp::CMPlayerApp()
{
    m_hMutexRuning = nullptr;
#ifndef _MPLAYER
    m_bReloadEmbeddedTheme = false;
#endif
}

CMPlayerApp::~CMPlayerApp()
{
}

bool CMPlayerApp::isAnotherInstanceRunning()
{
    assert(!m_hMutexRuning);
    HANDLE hMutexRuning = CreateMutex(nullptr, true, SZ_MUTEX_RUNNING);
    int nLastErr = getLastError();

    CloseHandle(hMutexRuning);

    return nLastErr == ERROR_ALREADY_EXISTS;
}

bool CMPlayerApp::setRunningFlag()
{
    // Is Product already running?
    assert(!m_hMutexRuning);
    m_hMutexRuning = CreateMutex(nullptr, true, SZ_MUTEX_RUNNING);
    if (getLastError() == ERROR_ALREADY_EXISTS )
    {
        CloseHandle(m_hMutexRuning);
        m_hMutexRuning = nullptr;
        return false;
    }

    return true;
}

cstr_t getAppIniFile()
{
    //
    // init Base frame, save log and settings in app dir
    return SZ_PROFILE_NAME;
}

bool CMPlayerApp::init()
{
    cstr_t lpCmdLine = GetCommandLine();

    if (isAnotherInstanceRunning())
    {
        HWND hWndMsg = findWindow(MSG_WND_CLASS_NAME, nullptr);
        if (hWndMsg)
        {
            sendActivateMainWnd(hWndMsg);

            if (!isEmptyString(lpCmdLine))
            {
                DBG_LOG0("Another Instance is running, send command line to it.");
                sendCommandLine(hWndMsg, lpCmdLine);
            }
            return false;
        }
        else
            terminateAnotherInstance();
    }

#if defined(_DEBUG) && defined(_MEM_LEAK_CHECK)
    InitAllocCheck(ACOutput_XML);
#endif

    g_log.setSrcRootDir(__FILE__, 2);

    char        szWorkingFolder[MAX_PATH] = {0};
    getModulePath(szWorkingFolder, getAppInstance());
    setWorkingFolder(szWorkingFolder);

    initBaseFrameWork(getAppInstance(), "log.txt", getAppIniFile(), SZ_SECT_UI);
    setFileNoReadOnly(g_profile.getFile());    

    g_LangTool.setMacro(SZ_MACRO_PRODUCT_NAME, SZ_APP_NAME);
    g_LangTool.setMacro(SZ_MACRO_COMPANY_NAME, SZ_COMPANY_NAME);

    LOG1(LOG_LVL_INFO, "start %s.", getAppNameLong().c_str());

    // init Winsock, Ole, Common Controls, gdiplus
    WSADATA        WsaData;
    if (WSAStartup(0x0101, &WsaData) != 0)
        ERR_LOG1("WSAStartup FAILED: %s", OSError().Description());

     INITCOMMONCONTROLSEX    initData;
     initData.dwSize = sizeof(initData);
     initData.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
     InitCommonControlsEx(&initData);

    // init gdiplus
    OleInitialize(nullptr);

    CGdiplusGraphicsLite::Startup();

#ifndef _MPLAYER
    m_appModeQuitToStart = SA_UNKNOWN;

    if (!processCmdLine(lpCmdLine, m_appMode))
        return false;

    char szFile[MAX_PATH];
    GetModuleFileName(nullptr, szFile, MAX_PATH);
    if (strcasecmp(fileGetName(szFile), "iPodLyricsDownloader") == 0)
        m_appMode = SA_IPOD_LYRICS_DOWNLOADER;
#endif

    return CMPlayerAppBase::init();
}

void CMPlayerApp::quit()
{
    if (!m_hMutexRuning)
        return;

    //
    // 先设置此标志表示退出，其他地方会用到根据此标志判断
    CloseHandle(m_hMutexRuning);
    m_hMutexRuning = nullptr;

    CMPlayerAppBase::quit();

#ifdef _WIN32_DESKTOP
    CGdiplusGraphicsLite::Shutdown();

    OleUninitialize();
#endif

#ifndef _MPLAYER
    if (m_appModeQuitToStart != SA_UNKNOWN)
    {
        char    szAppExe[MAX_PATH];

        GetModuleFileName(nullptr, szAppExe, CountOf(szAppExe));

        if (m_appModeQuitToStart == SA_IPOD_LYRICS_DOWNLOADER)
            execute(nullptr, szAppExe, "/iPodLyricsDownloader");
        else if (m_appModeQuitToStart == SA_LYRICS_PLAYER)
            execute(nullptr, szAppExe, nullptr);
    }
#endif
}

#ifndef _MPLAYER

bool CMPlayerApp::isSupportEmbedded()
{
    return g_Player.isSupportEmbedded();
}

bool CMPlayerApp::isEmbeddedImmovable()
{
    return g_Player.isEmbeddedImmovable();
}

void CMPlayerApp::endEmbeddedSkin()
{
    if (!m_bMPSkinMode)
    {
        CSkinWnd    *pWnd = getMainWnd();

        m_bMPSkinMode = true;
        if (::getParent(pWnd->getHandle()))
        {
            pWnd->closeSkin();
            pWnd->setParent(nullptr);
            g_Player.endEmbeddedSkin();

            pWnd->m_WndDrag.enableDrag(true);
            pWnd->m_wndResizer.fixedWidth(false);
            pWnd->m_wndResizer.fixedHeight(false);
        }

        getMainWnd()->getTrayIcon().forceShow(false);
    }
}

int CMPlayerApp::onSwitchToEmbeddedSkinMode()
{
    // to embedded mode
    assert(isSupportEmbedded());

    g_profile.writeInt("MPSkinMode", false);

    int nRet;
    if (g_Player.getPlayerType() == P_WINAMP2)
        nRet = loadWinamp2Skin(m_bReloadEmbeddedTheme);
    else
        nRet = loadGeneralEmbeddedSkin(m_bReloadEmbeddedTheme);

    m_bReloadEmbeddedTheme = false;

    if (nRet == ERR_OK)
        m_bMPSkinMode = false;

    return nRet;
}

#ifdef _DEBUG
void verySkinDirOK(string &strDir, cstr_t szName)
{
    if (!isDirExist(strDir.c_str()))
    {
        strDir = getInstallShareDir();
        strDir += szName;
    }
}
#endif

int CMPlayerApp::loadGeneralEmbeddedSkin(bool bLoadSkinColorTheme)
{
    //
    // General Embedded Skin
    //
    string        strSkinDir;
    int            nRet;
#define GEN_EMBEDDED_SKIN    "ml-gen-embedded-skin"

    const PlayerSkinInfo &skinInfo = g_Player.getPlayerSkinInfo();

    if (!skinInfo.hWndToAttach)
        return ERR_FALSE;

    strSkinDir = getAppResourceDir();

    strSkinDir += GEN_EMBEDDED_SKIN SZ_DIR_SLASH;
#ifdef _DEBUG
    verySkinDirOK(strSkinDir, GEN_EMBEDDED_SKIN SZ_DIR_SLASH);
#endif

    nRet = m_pSkinFactory->changeSkin(GEN_EMBEDDED_SKIN, strSkinDir.c_str(), "");
    if (nRet != ERR_OK)
        return nRet;

    CSkinWnd    *pWnd = getMainWnd();

    if (isLayeredWndSupported())
        ::unSetLayeredWindow(pWnd->getHandle());

    if (pWnd->isIconic())
        pWnd->showWindow(SW_RESTORE);

    ::setParentByForce(pWnd->getHandle(), skinInfo.hWndToAttach);

    if (bLoadSkinColorTheme && skinInfo.bClrThemeValid)
    {
        char        szColor[128];

        stringFromColor(szColor, skinInfo.clrFontHighlight);
        CMPlayerSettings::setLyricsDisplaySettings("FgColor", szColor);

        stringFromColor(szColor, skinInfo.clrFontLowlight);
        CMPlayerSettings::setLyricsDisplaySettings("FgLowColor", szColor);

        stringFromColor(szColor, skinInfo.clrBackground);
        CMPlayerSettings::setLyricsDisplaySettings("BgColor", szColor);

        stringFromColor(szColor, skinInfo.clrCurLineBg);
        CMPlayerSettings::setLyricsDisplaySettings("FocusLineBgColor", szColor);
    }

    HWND        hWndParent = ::getParent(pWnd->getHandle());
    if (hWndParent)
    {
        pWnd->m_WndDrag.enableDrag(false);
        pWnd->m_wndResizer.fixedWidth(true);
        pWnd->m_wndResizer.fixedHeight(true);

        if (pWnd->isIconic())
            pWnd->showWindow(SW_RESTORE);

        CRect    rc;
        getClientRect(skinInfo.hWndToAttach, &rc);
        ::moveWindow(pWnd->getHandle(), 0, 0, rc.width(), rc.height(), false);
    }

    getMainWnd()->getTrayIcon().forceShow(true);

    return ERR_OK;
}

uint32_t pleditGetPrivateProfileInt(cstr_t lpAppName, cstr_t lpKeyName, INT nDefault, cstr_t lpFileName)
{
    char    szBuffer[256];

    if (getPrivateProfileString(lpAppName, lpKeyName, "", szBuffer, 256, lpFileName) == 0)
        return nDefault;

    return stringToColor(szBuffer, nDefault);
}

int CMPlayerApp::loadWinamp2Skin(bool bLoadSkinColorTheme)
{
    //
    // 使用 Winamp2 的配套SKIN
    //
    // 主要是设置MiniLyrics的资源搜索路径：
    //   先搜索winamp skin目录中是否有skin，
    //   再搜索迷你歌词中对应的ml-winamp2-skin目录
    char    szWaSkinPath[MAX_PATH] = "";
    string    strSkinDir;
    int        nRet = ERR_OK;

    // 取得winamp的skin目录
    g_Player.getCurSkinPath(szWaSkinPath, MAX_PATH);
    if (isEmptyString(szWaSkinPath))
    {
        // 使用 minilyrics 的基本skin
        return m_pSkinFactory->changeSkin(SZ_DEFSKIN, "", "", true);
    }
    else
    {
        char    szTestFile[MAX_PATH];

        dirStringAddSlash(szWaSkinPath);

        strcpy_safe(szTestFile, CountOf(szTestFile), szWaSkinPath);
        strcat_safe(szTestFile, CountOf(szTestFile), "titlebar.bmp");
        if (!isFileExist(szTestFile))
        {
            return m_pSkinFactory->changeSkin(SZ_DEFSKIN, "", "", true);
        }
        else
        {
            // 取得缺省的skin的目录
            strSkinDir = getAppResourceDir();
            strSkinDir += "ml-winamp2-skin\\";
#ifdef _DEBUG
            verySkinDirOK(strSkinDir, "ml-winamp2-skin\\");
#endif

            nRet = m_pSkinFactory->changeSkin("ml-winamp2-skin", strSkinDir.c_str(), szWaSkinPath);
            if (nRet != ERR_OK)
                return nRet;
        }
    }

    if (bLoadSkinColorTheme)
    {
        //
        // WINAMP 的skin的配色方案
        //
        char    szPledit[MAX_PATH];
        COLORREF    clrHigh = 0xFF000000, clrLow = 0xFF000000, clrBg = 0xFF000000, clrFocusLineBg = 0xFF000000;
        char        szColor[128];


        if (m_pSkinFactory->getResourceMgr()->getResourcePathName(szPledit, MAX_PATH, "pledit.txt"))
        {
            clrHigh = pleditGetPrivateProfileInt("Text", "Current", 
                clrHigh, szPledit);

            clrLow = pleditGetPrivateProfileInt("Text", "Normal", 
                clrLow, szPledit);

            clrBg = pleditGetPrivateProfileInt("Text", "NormalBG", 
                clrBg, szPledit);

            clrFocusLineBg = pleditGetPrivateProfileInt("Text", "SelectedBG", 
                clrFocusLineBg, szPledit);

            if (clrHigh != 0xFF000000)
            {
                stringFromColor(szColor, clrHigh);
                CMPlayerSettings::setLyricsDisplaySettings("FgColor", szColor);
            }

            if (clrLow != 0xFF000000)
            {
                stringFromColor(szColor, clrLow);
                CMPlayerSettings::setLyricsDisplaySettings("FgLowColor", szColor);
            }

            if (clrBg != 0xFF000000)
            {
                stringFromColor(szColor, clrBg);
                CMPlayerSettings::setLyricsDisplaySettings("BgColor", szColor);
            }

            if (clrFocusLineBg != 0xFF000000)
            {
                stringFromColor(szColor, clrFocusLineBg);
                CMPlayerSettings::setLyricsDisplaySettings("FocusLineBgColor", szColor);
            }
        }
    }

    return nRet;
}

void CMPlayerApp::restartToAppMode(AppMode appMode)
{
    m_appModeQuitToStart = appMode;

    postQuitMessage();
}

#endif
