#include "MPlayerAppBase.h"
#include "MLCmd.h"
#include "DownloadMgr.h"
#include "MPEventsDispatcher.h"
#include "VersionUpdate.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "MPCommonCmdHandler.h"
#include "OnlineSearch.h"
#include "ErrorResolveDlg.h"
#include "LyricShowObj.h"
#include "../version.h"
#include "../LyricsLib/LyricsKeywordFilter.h"
#include "../LocalServer/LocalServer.hpp"
#include "../MediaTags/LrcParser.h"
#include "utils/unittest.h"

#ifdef _LINUX_GTK2
#include "../Skin/gtk2/SplashScreenWnd.h"
#include "gtk2/LyricsOutPluginMgr.h"
#endif

#ifdef _WIN32
#include "../Skin/win32/SplashScreenWnd.h"
#include "win32/LyricsOutPluginMgr.h"
#include "win32/LyricsDownloader.h"
#include "win32/DlgSaveLyrPrompt.h"
#endif

#include "MPFloatingLyrWnd.h"

#ifdef _MAC_OS
#include "mac/DlgSaveLyrPrompt.h"
#include "mac/LyricsOutPluginMgr.h"


#endif

CurrentLyrics g_currentLyrics;

CDownloadMgr g_LyricsDownloader;

CLyricsLocalSearch g_LyricSearch;

CharEncodingType getDefaultLyricsEncodingSettings();


string getAppNameLong() {
    return string(SZ_APP_NAME) + " " + VERSION_STR;
}

cstr_t getStrName(STR_NAME nameId) {
    switch (nameId) {
        case SN_HTTP_SIGNUP: return "http://www.viewlyrics.com/user/signup.aspx";
        case SN_HTTP_LOGIN: return "http://www.viewlyrics.com/user/login.aspx";
        case SN_HTTP_RATE_LRC: return "http://viewlyrics.com/lyrics/rate.aspx?lid=";
        default: break;
    }

    switch (nameId) {
        case SN_HTTP_DOMAIN: return "http://www.crintsoft.com";
        case SN_HTTP_REGISTER: return "http://www.crintsoft.com/mlbuy.htm";
        case SN_HTTP_DLSKIN: return "http://www.crintsoft.com/mlskin.htm";
        case SN_HTTP_FAQ_INET: return "http://www.crintsoft.com/mlfaq-ineterror.htm";
        case SN_HTTP_BBS: return "http://forum.viewlyrics.com";
        case SN_HTTP_ML_VER: return "http://viewlyrics.com/mlver.xml";
        case SN_HTTP_HELP: return "http://www.crintsoft.com/help.htm";
        case SN_HTTP_HELP_EMBEDDED_LYR: return "http://www.crintsoft.com/mlfaq-embedlyrics.htm";
        case SN_HTTP_HELP_UPLOAD_LYR: return "http://www.crintsoft.com/mlfaq-upload.htm";
        case SN_HTTP_HELP_EDIT_LYR: return "http://www.crintsoft.com/mlfaq-editlrc.htm";
        case SN_HTTP_HELP_SEARCH_LYR_SUGGESTIONS: return "http://www.crintsoft.com/mlfaq-search-lyrics-suggestions.htm";
        case SN_HTTP_DLPLUGIN: return "http://www.crintsoft.com/mlplugin.htm";
        case SN_HTTP_HELP_G15_LCD: return "http://www.crintsoft.com/help-logitech-lcd.htm";
        case SN_HTTP_FEEDBACK: return "http://www.crintsoft.com/contactus.htm";
        case SN_EMAIL: return "mailto:support@crintsoft.com";
        case SN_SUPPORT_MAIL: return "support@crintsoft.com";
        case SN_WEBHOME: return "www.crintsoft.com";
        default: assert(0); return "";
    }
}

void dispatchPlayPosEvent(int nPlayPos) {
    static int nPlayPosLast = 0;
    nPlayPos %= 500;
    if (nPlayPos != nPlayPosLast) {
        nPlayPosLast = nPlayPos;
        CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(ET_PLAYER_POS_UPDATE);
    }
}

//////////////////////////////////////////////////////////////////////////


bool g_bAutoMinimized = false;

CMPlayerAppBase::CMPlayerAppBase() {
}

CMPlayerAppBase::~CMPlayerAppBase() {

}

CMPSkinFactory *CMPlayerAppBase::getMPSkinFactory() {
    assert(m_pInstance);
    return (CMPSkinFactory *)m_pInstance->getSkinFactory();
}

CEventsDispatcherBase *CMPlayerAppBase::getEventsDispatcher() {
    assert(m_pInstance);
    return m_pInstance->getEventPatcher();
}

bool CMPlayerAppBase::init() {
    setDefaultSettings();

    if (!CSkinApp::init()) {
        return false;
    }

    CLyricsKeywordFilter::init();

    runAllUnittest();

    g_LyricSearch.init();

    g_LyricsDownloader.init();

    setDefaultLyricsEncoding(getDefaultLyricsEncodingSettings());

    g_autoProcessEmbeddedLyrics.init();

    g_OnlineSearch.init();

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_LYRICS_SEARCH_END, ET_DOWNLOAD_END, ET_LYRICS_RESEARCH);

    g_player.onInit();

    // create main skin window.
    int nRet = loadDefaultSkin(SZ_DEFSKIN);
    if (nRet != ERR_OK) {
        g_profile.writeString(m_pSkinFactory->getSkinFileName(), "DefaultSkinWnd", "");
        nRet = changeSkinByUserCmd(SZ_DEFSKIN);
        if (nRet != ERR_OK) {
            messageOut("Failed to open skin, please perform full uninstallation to solve this issue.");
        }
    }

    m_hotKey.init();

    m_hotKey.setEventWnd(getMainWnd());

    // Restore iconic status
    //    if (g_profile.getBool("Minimized", false))
    //    {
    //        if (getMainWnd()->isToolWindow())
    //            getMainWnd()->showWindow(SW_HIDE);
    //        getMainWnd()->showWindow(SW_SHOWMINNOACTIVE);
    //    }

    if (g_profile.getBool(SZ_SECT_UI, "ClickThrough", false)) {
        getMPSkinFactory()->setClickThrough(true);
    }

#ifdef _MINILYRICS_WIN32
    if (g_profile.getBool("FloatingLyr", false)) {
        g_wndFloatingLyr.create();
    }
#endif

    g_lyrOutPlguinMgr.init();

    // onMediaChanged(true);

    LocalServer::getInstance()->start();

    getInstance()->setRunningFlag();

    // Checker for new version
    if (g_profile.getBool(SZ_SECT_UI, "CheckNewVer", true)) {
        CVersionUpdate VerUp;
        VerUp.checkNewVersion();
    }

    return true;
}

void CMPlayerAppBase::quit() {
    // 因为网络原因会导致 线程卡住，不主动调用 quit.
    // g_LyricSearch.quit();
    // g_LyricsDownloader.quit();

    g_lyrOutPlguinMgr.quit();
    g_OnlineSearch.quit();

    IEventHandler::unregisterHandler();
    m_pEventDispatcher->quit();

    getHotkey().quit();

    g_wndFloatingLyr.destroy();
    m_pSkinFactory->quit();
    g_player.onQuit();

    g_currentLyrics.close();

    g_profile.close();
    g_log.close();
    g_LangTool.close();

#if defined(_DEBUG) && defined(_MEM_LEAK_CHECK)
    DeInitAllocCheck();
#endif
}

void CMPlayerAppBase::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED) {
        onMediaChanged(true);
    } else if (pEvent->eventType == ET_LYRICS_RESEARCH) {
        onMediaChanged(false);
    } else if (pEvent->eventType == ET_LYRICS_SEARCH_END) {
        onOnlineSearchEnd();
    } else if (pEvent->eventType == ET_DOWNLOAD_END) {
        CEventDownloadEnd *pDlEvt = (CEventDownloadEnd *)pEvent;
        if (pDlEvt->downloadType == CEventDownloadEnd::DT_DL_LRC_FAILED) {
            onDownloadLyricsFailed(pDlEvt->pTask);
        } else if (pDlEvt->downloadType == CEventDownloadEnd::DT_DL_CHECK_NEW_VERSION_OK) {
            CVersionUpdate verCheck;
            verCheck.onDownloadOK(pDlEvt->pTask);
        }
    }
}

void CMPlayerAppBase::postQuitMessage() {
    onLyricsChangingSavePrompt();

    g_autoProcessEmbeddedLyrics.onPostQuit();

    g_player.saveCurrentPlaylist();

    CSkinApp::postQuitMessage();
}

CSkinFactory *CMPlayerAppBase::newSkinFactory() {
    return new CMPSkinFactory(this, g_uidDefinition);
}

void CMPlayerAppBase::onMediaChanged(bool bAutoDownloadIfNotExist) {
    // 判断是否要保存歌词
    if (!onLyricsChangingSavePrompt()) {
        return;
    }

    // auto save embedded lyrics
    g_autoProcessEmbeddedLyrics.onSongChanged();

    g_currentLyrics.newLyrics(g_player.getMediaKey().c_str(), g_player.getMediaLength());

    //
    // 没有歌曲信息
    //
    if (!g_player.isMediaOpened()) {
        dispatchLyricsChangedSyncEvent();
        return;
    }

    bool bAssociateNone = false;
    int nRet;

    string strAssociateMediaKey = g_player.getMediaKey();
    if (g_LyricSearch.isAssociatedWithNoneLyrics(strAssociateMediaKey.c_str())) {
        bAssociateNone = true;
    } else {
        string strLyricsFile;
        if (g_LyricSearch.getBestMatchLyrics(g_player.getSrcMedia(), g_player.getArtist(), g_player.getTitle(), strLyricsFile)) {
            nRet = openLyrics(strAssociateMediaKey.c_str(), strLyricsFile.c_str());
            if (nRet != ERR_OK) {
                dispatchLongErrorText(ERROR2STR_LOCAL(nRet));
            }
        }
    }

    if (g_currentLyrics.hasLyricsOpened()) {
        if (g_bAutoMinimized) {
            CMPlayerAppBase::getMPSkinFactory()->restoreAll();
            g_bAutoMinimized = false;
        }
    } else {
        // add Media Info:
        string mediaInfo;
        if (!isEmptyString(g_player.getArtist())) {
            mediaInfo.append(string(_TLT("Artist:")) + " " + g_player.getArtist() + SZ_NEW_LINE);
        }
        if (!isEmptyString(g_player.getTitle())) {
            mediaInfo.append(string(_TLT("Title:")) + " " + g_player.getTitle() + SZ_NEW_LINE);
        }
        if (!isEmptyString(g_player.getAlbum())) {
            mediaInfo.append(string(_TLT("Album:")) + " " + g_player.getAlbum() + SZ_NEW_LINE);
        }
        g_currentLyrics.fromString(mediaInfo.c_str());
        g_currentLyrics.properties().lyrContentType = LCT_UNKNOWN;

        dispatchLyricsChangedSyncEvent();
    }

    if (bAutoDownloadIfNotExist && !bAssociateNone) {
        g_LyricsDownloader.onSongChanged();
    }
}

void CMPlayerAppBase::onLanguageChanged() {
    g_LangTool.onLanguageChanged();

    m_pSkinFactory->onLanguageChanged();

    if (g_wndFloatingLyr.isValid()) {
        g_wndFloatingLyr.onLanguageChanged();
    }
}

void CMPlayerAppBase::onOnlineSearchEnd() {
    // If lyrics already associated, do not download auto.
    if (g_LyricSearch.isAssociatedLyrics(g_player.getMediaKey().c_str())) {
        return;
    }

    // 在线自动搜索的通知消息，表示该歌词的结果已经搜索到了
    g_LyricsDownloader.searchInCacheResult(true);
}

CMPSkinMainWnd *CMPlayerAppBase::getMainWnd() {
    assert(m_pInstance);

    CSkinWnd *pWnd = m_pInstance->getSkinFactory()->getMainWnd();
    if (pWnd) {
        return (CMPSkinMainWnd *)pWnd;
    } else {
        ERR_LOG0("Main window should have been created already.");
        static CMPSkinMainWnd wnd;
        wnd.m_WndDrag.init(&wnd, nullptr);
        return &wnd;
    }
}

CMPHotkey &CMPlayerAppBase::getHotkey() {
    assert(m_pInstance);

    return ((CMPlayerAppBase*)m_pInstance)->m_hotKey;
}

CMPlayerApp *CMPlayerAppBase::getInstance() {
    if (m_pInstance) {
        return (CMPlayerApp *)m_pInstance;
    }

    if (!m_pInstance) {
        m_pInstance = new CMPlayerApp();
    }

    return (CMPlayerApp*)m_pInstance;
}

void CMPlayerAppBase::dispatchInfoText(cstr_t szInfo, cstr_t szInfoName) {
    IEvent *pEvent = new IEvent;
    pEvent->eventType = ET_UI_INFO_TEXT;
    pEvent->strValue = szInfo;
    if (szInfoName) {
        pEvent->name = szInfoName;
    }

    getEventsDispatcher()->dispatchUnsyncEvent(pEvent);
}

void CMPlayerAppBase::dispatchLongErrorText(cstr_t szError, int cmd) {
    dispatchLongErrorText(szError, ("cmd://" + getSkinFactory()->getStringOfID(cmd)).c_str());
}

void CMPlayerAppBase::dispatchLongErrorText(cstr_t szError, cstr_t szCmd) {
    IEvent *pEvent = new IEvent;
    pEvent->eventType = ET_UI_LONG_ERROR_TEXT;
    pEvent->strValue = szError;
    if (szCmd != nullptr) {
        pEvent->name = szCmd;
    }

    getEventsDispatcher()->dispatchUnsyncEvent(pEvent);
}

void CMPlayerAppBase::dispatchResearchLyrics() {
    IEvent *pEvent = new IEvent();
    pEvent->eventType = ET_LYRICS_RESEARCH;
    getEventsDispatcher()->dispatchUnsyncEvent(pEvent);
}

void CMPlayerAppBase::dispatchLyricsChangedSyncEvent() {
    IEvent *pEvent = new IEvent();

    pEvent->eventType = ET_LYRICS_CHANGED;
    getEventsDispatcher()->dispatchSyncEvent(pEvent);
}

void CMPlayerAppBase::onDownloadLyricsFailed(CDownloadTask *pTask) {
    if (pTask->m_errResult != ERR_OK) {
        showInetErrorDlg(getMainWnd(), pTask->m_errResult);
    } else if (pTask->m_nHttpRetCode != 200) {
        auto str = stringPrintf("URL:  %s\r\nCode: %d, %s", pTask->m_strURL.c_str(),
            pTask->m_nHttpRetCode, httpErrorCodeToStr(pTask->m_nHttpRetCode));

        getMainWnd()->messageOut(str.c_str());
    }
}

void CMPlayerAppBase::newLyrics() {
    // 判断是否要保存歌词
    if (!onLyricsChangingSavePrompt()) {
        return;
    }

    g_currentLyrics.newLyrics(g_player.getMediaKey().c_str(), g_player.getMediaLength());

    dispatchLyricsChangedSyncEvent();
}

int CMPlayerAppBase::openLyrics(cstr_t szAssociateKeyword, cstr_t szLrcSource) {
    int nRet;

    nRet = g_currentLyrics.openLyrics(szAssociateKeyword, g_player.getMediaLength(), szLrcSource);
    if (nRet != ERR_OK) {
        return nRet;
    }

    dispatchLyricsChangedSyncEvent();

    return nRet;
}

// Before lyrics changing, show save Prompt dialog,
// if return true, cancel changes.
bool CMPlayerAppBase::onLyricsChangingSavePrompt() {
    // 判断是否要保存歌词
    m_pEventDispatcher->dispatchSyncEvent(ET_LYRICS_ON_SAVE_EDIT);

    if (g_currentLyrics.hasLyricsOpened() && g_currentLyrics.isModified()) {
        int nRet = saveLyrDialogBox(getMainWnd());
        if (nRet == IDYES) {
            return CMPCommonCmdHandler::saveCurrentLyrics(getMainWnd(), false);
        } else if (nRet == IDCANCEL) {
            return false;
        }
    }

    return true;
}


int CMPlayerAppBase::messageOut(cstr_t lpText, uint32_t uType, cstr_t lpCaption) {
    Window *pWnd;

    pWnd = getMainWnd();
    if (!pWnd) {
        return IDCANCEL;
    }

    if (lpCaption == nullptr) {
        lpCaption = SZ_APP_NAME;
    }

    return pWnd->messageOut(lpText, uType, lpCaption);
}

int CMPlayerAppBase::changeSkinByUserCmd(cstr_t szSkin) {
    // set default page, when skin changes.
#ifdef _MINILYRICS_WIN32
    if (CMPlayerAppBase::getInstance()->isEmbeddedMode()) {
        g_profile.writeInt("MPSkinMode", true);
    }

    CMPlayerAppBase::getInstance()->endEmbeddedSkin();
#endif

    writeDefaultSkin(szSkin);

    return m_pSkinFactory->changeSkin(szSkin, "", "", true);
}

void CMPlayerAppBase::getCurLyrDisplaySettingName(bool bFloatingLyr, string &strSectionName, EventType &etDispSettings) {
    if (bFloatingLyr) {
        strSectionName = SZ_SECT_FLOATING_LYR;
        etDispSettings = ET_LYRICS_FLOATING_SETTINGS;
    } else {
        strSectionName = SZ_SECT_LYR_DISPLAY;
        etDispSettings = ET_LYRICS_DISPLAY_SETTINGS;
    }
}

cstr_t CMPlayerAppBase::getCurLyrDisplaySettingName(bool bFloatingLyr) {
    if (bFloatingLyr) {
        return SZ_SECT_FLOATING_LYR;
    } else {
        return SZ_SECT_LYR_DISPLAY;
    }
}

void CMPlayerAppBase::setDefaultSettings() {
    g_profile.doCache();

#ifdef _WIN32
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "Font", "Verdana, 16, bold, 0, 0, Tahoma");
#else
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "Font", ", 13, bold, 0, 0, ");
#endif
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "FgColor", "#FFFFFF");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "FgLowColor", "#B2B2B2");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "BgColor", "#576C91");

    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "Ed_HighColor", "#0000AE");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "Ed_LowColor", "#000000");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "Ed_BgColor", "#FFFFFF");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "Ed_TagColor", "#FF8000");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "Ed_FocusLineBgColor", "#E7F7FF");

    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "HilightBorderColor", "#000000");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "LowlightBorderColor", "#000000");

    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "HilightOBM", OBM_COLOR);
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "HilightGradient1", "#FFFF80");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "HilightGradient2", "#FF2D2D");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "HilightGradient3", "#AE0000");

    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "FgColor", "#FF0000");

    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "LowlightOBM", OBM_COLOR);
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "LowlightGradient1", "#0000A0");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "LowlightGradient2", "#0080FF");
    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "LowlightGradient3", "#0000FF");

    g_profile.setDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, "FgLowColor", "#0000FF");

    //
    // Floating lyrics display settings
    //
#ifdef _WIN32
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "Font", "Verdana, 35, bold, 0, 0, Tahoma");
#else
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "Font", ", 35, bold, 0, 0, ");
#endif
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "FgColor", "#0000FF");
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "FgLowColor", "#8080FF");
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "BgColor", "#576C91");

    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "LyrDrawOpt", "fadeout");
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "OutlineLyrText", true);

    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "HilightBorderColor", "#000000");
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "LowlightBorderColor", "#000000");

    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "HilightOBM", OBM_GRADIENT_COLOR);
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "HilightGradient1", "#FFFFA4");
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "HilightGradient2", "#FFFF00");
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "HilightGradient3", "#FE6001");

    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "FgColor", "#FFF900");

    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "LowlightOBM", OBM_GRADIENT_COLOR);
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "LowlightGradient1", "#912F00");
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "LowlightGradient2", "#FF8040");
    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "LowlightGradient3", "#973100");

    g_profile.setDefaultIfNotExist(SZ_SECT_FLOATING_LYR, "FgLowColor", "#FF8040");
}
