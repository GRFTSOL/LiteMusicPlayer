#include "../MPlayerApp.h"
#include "../Helper.h"
#include "LyricsOutPluginMgr.h"


CLyricsOutPluginMgr g_lyrOutPlguinMgr;



CLyricsOutPluginMgr::CLyricsOutPluginMgr() {

}

CLyricsOutPluginMgr::~CLyricsOutPluginMgr() {

}

int CLyricsOutPluginMgr::init() {
    return ERR_OK;
}

void CLyricsOutPluginMgr::quit() {
}

void CLyricsOutPluginMgr::getLoadedPlugins(vector<string> &vPlugins) {
    /*    V_PLUGINS::iterator        it, itEnd;
    char        szFileName[MAX_PATH];
    string        str;

    itEnd = m_vPlugins.end();
    for (it = m_vPlugins.begin(); it != itEnd; ++it)
    {
        Plugins        &plugin = *it;
        if (plugin.bInitOK)
        {
            fileGetName(szFileName, CountOf(szFileName), plugin.strPluginFile.c_str());
            if (plugin.bInitOK)
            {
                str = plugin.plyricsOut->getDescription();
            }
            else
            {
                str = "Failed to load.";
            }
            str += " (";
            str += szFileName;
            str += ")";
            vPlugins.push_back(str);
        }
    }*/
}

void CLyricsOutPluginMgr::configurePlugin(int nPluginIdx, Window *pWndParent) {
}

void CLyricsOutPluginMgr::aboutPlugin(int nPluginIdx, Window *pWndParent) {
}

void CLyricsOutPluginMgr::uninstPlugin(int nPluginIdx, Window *pWndParent) {
}

void CLyricsOutPluginMgr::onPlayPos(int uPos) {
}

void CLyricsOutPluginMgr::onSongChanged() {
}

void CLyricsOutPluginMgr::onLyricsChanged() {
}

int CLyricsOutPluginMgr::getLineCount() {
}

bool CLyricsOutPluginMgr::getLyricsOfLine(int nLine, char szLyrics[], int nBuffLen, int &beginTime, int &endTime) {
    return false;
}

int CLyricsOutPluginMgr::getCurLine() {
    return 0;
}

int CLyricsOutPluginMgr::getPlayPos() {
    return g_currentLyrics.getPlayElapsedTime();
}

bool CLyricsOutPluginMgr::getMediaFile(char szFile[], int nBuffLen) {
    emptyStr(szFile);
    strcpy_safe(szFile, nBuffLen, g_player.getSrcMedia());
    return !emptyStr(szFile);
}

#ifdef _WIN32
HWND CLyricsOutPluginMgr::getMainWnd() {
    if (MPlayerApp::getMainWnd()) {
        return MPlayerApp::getMainWnd()->getHandle();
    } else {
        return nullptr;
    }
}
#endif
