#include "MPlayerApp.h"
#include "LyricsOutPluginMgr.h"
#include "Helper.h"


typedef int (*FunInitMLyricsOutPlugin)(HINSTANCE hDllInstance, IMLyrOutHost *pMLyrOutHost, ILyricsOut **ppLyricsOut);

CLyricsOutPluginMgr g_lyrOutPlguinMgr;



CLyricsOutPluginMgr::CLyricsOutPluginMgr() {
    m_bActive = false;
    m_bHalfOfCurLine = 0;
    m_dwNotifyFlag = 0;
    m_nCLBegTime = m_nCLHalftime = m_nCLendtime = 0;
    m_nNLBegTime = m_nNLEndtime = 0;
    m_nCurLine = 0;
}

CLyricsOutPluginMgr::~CLyricsOutPluginMgr() {

}

void promptForLogitechLCD(cstr_t szModuleName) {
    if (strcasecmp(fileGetName(szModuleName), "mlp_G15.dll") != 0) {
        return;
    }

    if (g_profile.getBool("G15LCDPrompted", false)) {
        return;
    }

    g_profile.writeInt("G15LCDPrompted", true);

    string strMsg = _TL("You have Logitech LCD device installed, lyrics can be displayed on it.");
    strMsg += "\r\n";
    strMsg += _TLT("Do you want to visit our website for more information?");
    int nRet = CMPlayerAppBase::getInstance()->messageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_OKCANCEL);
    if (nRet == IDOK) {
        openUrl(CMPlayerAppBase::getMainWnd(), getStrName(SN_HTTP_HELP_G15_LCD));
    }
}

int CLyricsOutPluginMgr::init() {
    int nRet;
    FileFind find;
    HMODULE hModule;
    string strPluginDir;
    string strPluginFile;
    FunInitMLyricsOutPlugin funInitMLyricsOutPlugin;

    // uninstall the plug-in which was selected last time.
    strPluginFile = g_profile.getString("UninstLyrOutPluginFile", "");
    if (!strPluginFile.empty()) {
        setFileNoReadOnly(strPluginFile.c_str());
        if (deleteFile(strPluginFile.c_str())) {
            g_profile.writeString("UninstLyrOutPluginFile", "");
        }
    }

    m_dwNotifyFlag = 0;

    strPluginDir = getAppResourceDir();
    strPluginDir += "Plugins\\";

    DBG_LOG0("Begin to load lyrics output plugins.");

    if (!find.openDir(strPluginDir.c_str(), "*.dll")) {
        DBG_LOG1("Can't open plugin dir: %s", strPluginDir.c_str());
        return ERR_OK;
    }

    while (find.findNext()) {
        if (find.isCurDir()) {
            continue;
        }

        if (strncasecmp(find.getCurName(), "mlp_", 4) == 0) {
            strPluginFile = strPluginDir;
            strPluginFile += find.getCurName();
            hModule = LoadLibrary(strPluginFile.c_str());
            if (!hModule) {
                ERR_LOG2("Failed to load plugin: %s, %s", strPluginFile.c_str(), OSError().Description());
                continue;
            }

            funInitMLyricsOutPlugin = (FunInitMLyricsOutPlugin)GetProcAddress(hModule, "InitMLyricsOutPlugin");
            if (funInitMLyricsOutPlugin) {
                Plugins plugin;
                nRet = (*funInitMLyricsOutPlugin)(hModule, this, &plugin.plyricsOut);
                if (nRet != ML_LYR_OUT_VERSION || plugin.plyricsOut == nullptr) {
                    FreeLibrary(hModule);
                    CMPlayerAppBase::getInstance()->messageOut(stringPrintf("init lyrics output plugin: %s FAILED!\r\nMaybe incorrect plugin version!", strPluginFile.c_str()).c_str());
                } else {
                    plugin.strPluginFile = strPluginFile;
                    plugin.hModule = hModule;
                    plugin.uNotifyFlag = 0;
                    plugin.bInitOK = plugin.plyricsOut->onInit(&plugin.uNotifyFlag);
                    if (plugin.bInitOK) {
                        m_dwNotifyFlag |= plugin.uNotifyFlag;
                        m_vPlugins.push_back(plugin);
                        promptForLogitechLCD(strPluginFile.c_str());
                    }
                }
            } else {
                FreeLibrary(hModule);
                DBG_LOG1("GetProAddress FAILED: %s, InitMLyricsOutPlugin", strPluginFile.c_str());
            }
        }
    }

    m_bActive = !m_vPlugins.empty();
    if (m_bActive) {
        registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_LYRICS_CHANGED, ET_LYRICS_DRAW_UPDATE, ET_PLAYER_CUR_MEDIA_CHANGED);
    }

    return ERR_OK;
}

void CLyricsOutPluginMgr::quit() {
    V_PLUGINS::iterator it, itEnd;

    if (m_bActive) {
        IEventHandler::unregisterHandler();
    }

    itEnd = m_vPlugins.end();
    for (it = m_vPlugins.begin(); it != itEnd; ++it) {
        Plugins &plugin = *it;
        if (plugin.bInitOK) {
            plugin.plyricsOut->onQuit();
        }
        FreeLibrary(plugin.hModule);
    }
    m_vPlugins.clear();
}

void CLyricsOutPluginMgr::getLoadedPlugins(vector<string> &vPlugins) {
    V_PLUGINS::iterator it, itEnd;
    string str;

    itEnd = m_vPlugins.end();
    for (it = m_vPlugins.begin(); it != itEnd; ++it) {
        Plugins &plugin = *it;
        if (plugin.bInitOK) {
            if (plugin.bInitOK) {
                str = plugin.plyricsOut->getDescription();
            } else {
                str = "Failed to load.";
            }
            str += " (";
            str += fileGetName(plugin.strPluginFile.c_str());
            str += ")";
            vPlugins.push_back(str);
        }
    }
}

void CLyricsOutPluginMgr::configurePlugin(int nPluginIdx, Window *pWndParent) {
#ifdef _WIN32
    if (nPluginIdx < 0 || nPluginIdx >= (int)m_vPlugins.size()) {
        return;
    }

    Plugins &plugin = m_vPlugins[nPluginIdx];
    if (plugin.bInitOK) {
        plugin.plyricsOut->config(pWndParent->getHandle());
    }
#endif
}

void CLyricsOutPluginMgr::aboutPlugin(int nPluginIdx, Window *pWndParent) {
#ifdef _WIN32
    if (nPluginIdx < 0 || nPluginIdx >= (int)m_vPlugins.size()) {
        return;
    }

    Plugins &plugin = m_vPlugins[nPluginIdx];
    if (plugin.bInitOK) {
        plugin.plyricsOut->about(pWndParent->getHandle());
    }
#endif
}

void CLyricsOutPluginMgr::uninstPlugin(int nPluginIdx, Window *pWndParent) {
#ifdef _WIN32
    if (nPluginIdx < 0 || nPluginIdx >= (int)m_vPlugins.size()) {
        return;
    }

    Plugins &plugin = m_vPlugins[nPluginIdx];
    if (plugin.bInitOK) {
        int nRet;
        string str = _TLT("Are you sure you want to permanently uninstall this plug-in?");
        str += "\n";
        str += _TLT("$Product$ will uninstall it at next restart.");
        nRet = pWndParent->messageOut(str.c_str(), MB_YESNO);
        if (nRet == IDYES) {
            g_profile.writeString("UninstLyrOutPluginFile", plugin.strPluginFile.c_str());
        }
    }
#endif
}

// IEventHandler
void CLyricsOutPluginMgr::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_LYRICS_CHANGED) {
        onLyricsChanged();
    } else if (pEvent->eventType == ET_LYRICS_DRAW_UPDATE) {
        onPlayPos(g_currentLyrics.getPlayElapsedTime());
    } else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED) {
        onSongChanged();
    }
}

void CLyricsOutPluginMgr::onPlayPos(int uPos) {
    V_PLUGINS::iterator it, itEnd;

    itEnd = m_vPlugins.end();

    if (m_dwNotifyFlag & ILyricsOut::NOTIF_PLAY_POS) {
        for (it = m_vPlugins.begin(); it != itEnd; ++it) {
            Plugins &plugin = *it;
            if (plugin.bInitOK) {
                plugin.plyricsOut->onPlayPos(uPos);
            }
        }
    }

    if (m_dwNotifyFlag & (ILyricsOut::NOTIF_CUR_LINE | ILyricsOut::NOTIF_HALF_CUR_LINE)) {
        LyricsLines &lyrLines = g_currentLyrics.getLyricsLines();
        LyricsLine *pLine;

        int nCurLineOld = m_nCurLine;
        bool bHalfOfCurLineOld = m_bHalfOfCurLine;

        if (uPos < m_nCLBegTime || uPos >= m_nNLBegTime || m_nCurLine == -1
            || m_nCurLine >= (int)lyrLines.size()) {
            m_nCurLine = g_currentLyrics.getCurPlayLine(lyrLines);
            if (m_nCurLine >= 0 && m_nCurLine < (int)lyrLines.size()) {
                pLine = lyrLines[m_nCurLine];

                m_nCLBegTime = pLine->beginTime;
                m_nCLendtime = pLine->endTime;
                m_nCLHalftime = (m_nCLBegTime + m_nCLendtime) / 2;

                if (m_nCurLine + 1 < (int)lyrLines.size()) {
                    pLine = lyrLines[m_nCurLine + 1];
                    m_nNLBegTime = pLine->beginTime;
                    m_nNLEndtime = pLine->endTime;
                } else {
                    m_nNLBegTime = m_nNLEndtime = pLine->endTime;
                }
                if (uPos >= m_nCLHalftime) {
                    m_bHalfOfCurLine = true;
                } else {
                    m_bHalfOfCurLine = false;
                }
            } else {
                m_bHalfOfCurLine = false;
            }
        }

        if (!m_bHalfOfCurLine && uPos >= m_nCLHalftime) {
            m_bHalfOfCurLine = true;
        }

        if (m_nCurLine != nCurLineOld) {
            string strLyrCurline;
            if (m_nCurLine != -1) {
                pLine = lyrLines[m_nCurLine];
                g_currentLyrics.lyricsLineToText(pLine, strLyrCurline);
            }

            for (it = m_vPlugins.begin(); it != itEnd; ++it) {
                Plugins &plugin = *it;
                if (plugin.bInitOK && (plugin.uNotifyFlag & ILyricsOut::NOTIF_CUR_LINE)) {
                    plugin.plyricsOut->onCurLineChanged(m_nCurLine, strLyrCurline.c_str(), m_nCLBegTime, m_nCLendtime);
                }
            }
        }

        if (m_bHalfOfCurLine && m_bHalfOfCurLine != bHalfOfCurLineOld) {
            for (it = m_vPlugins.begin(); it != itEnd; ++it) {
                Plugins &plugin = *it;
                if (plugin.bInitOK && (plugin.uNotifyFlag & ILyricsOut::NOTIF_HALF_CUR_LINE)) {
                    plugin.plyricsOut->onHalfOfCurLine(m_nCurLine);
                }
            }
        }
    }
}

void CLyricsOutPluginMgr::onSongChanged() {
    V_PLUGINS::iterator it, itEnd;

    if (m_vPlugins.empty()) {
        return;
    }

    m_nCurLine = -1;

    int nDuration;
    nDuration = g_player.getMediaLength();

    itEnd = m_vPlugins.end();
    for (it = m_vPlugins.begin(); it != itEnd; ++it) {
        Plugins &plugin = *it;
        if (plugin.bInitOK) {
            plugin.plyricsOut->onSongChanged(g_player.getArtist(), g_player.getTitle(), nDuration);
        }
    }
}

void CLyricsOutPluginMgr::onLyricsChanged() {
    V_PLUGINS::iterator it, itEnd;

    m_nCurLine = -1;

    itEnd = m_vPlugins.end();
    for (it = m_vPlugins.begin(); it != itEnd; ++it) {
        Plugins &plugin = *it;
        if (plugin.bInitOK) {
            plugin.plyricsOut->onLyricsChanged();
        }
    }
}

int CLyricsOutPluginMgr::getLineCount() {
    LyricsLines &lyrLines = g_currentLyrics.getLyricsLines();
    return lyrLines.size();
}

bool CLyricsOutPluginMgr::getLyricsOfLine(int nLine, char szLyrics[], int nBuffLen, int &beginTime, int &endTime) {
    LyricsLines &lyrLines = g_currentLyrics.getLyricsLines();
    if (nLine < 0 || nLine >= (int)lyrLines.size()) {
        return false;
    }

    string str;
    LyricsLine *pLine = lyrLines[nLine];
    g_currentLyrics.lyricsLineToText(pLine, str);
    strcpy_safe(szLyrics, nBuffLen, str.c_str());
    beginTime = pLine->beginTime;
    endTime = pLine->endTime;

    return true;
}

int CLyricsOutPluginMgr::getCurLine() {
    LyricsLines &lyrLines = g_currentLyrics.getLyricsLines();

    return g_currentLyrics.getCurPlayLine(lyrLines);
}

int CLyricsOutPluginMgr::getPlayPos() {
    return g_currentLyrics.getPlayElapsedTime();
}

bool CLyricsOutPluginMgr::getMediaFile(char szFile[], int nBuffLen) {
    emptyStr(szFile);
    strcpy_safe(szFile, nBuffLen, g_player.getSrcMedia());
    return !isEmptyString(szFile);
}

#ifdef _WIN32
HWND CLyricsOutPluginMgr::getMainWnd() {
    if (CMPlayerAppBase::getMainWnd()) {
        return CMPlayerAppBase::getMainWnd()->getHandle();
    } else {
        return nullptr;
    }
}
#endif
