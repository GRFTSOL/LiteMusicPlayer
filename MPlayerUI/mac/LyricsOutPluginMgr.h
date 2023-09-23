#pragma once

#ifndef MPlayerUI_mac_LyricsOutPluginMgr_h
#define MPlayerUI_mac_LyricsOutPluginMgr_h

#include "../LyricsOutPlugin.h"


class CLyricsOutPluginMgr : public IMLyrOutHost {
public:
    struct Plugins {
        string                      strPluginFile;
        ILyricsOut                  *plyricsOut;
        bool                        bInitOK;
        unsigned int                uNotifyFlag;
    };

    CLyricsOutPluginMgr();
    virtual ~CLyricsOutPluginMgr();

    int init();
    void quit();

    bool isActive() { return m_bActive; }

    void getLoadedPlugins(vector<string> &vPlugins);
    void configurePlugin(int nPluginIdx, Window *pWndParent);
    void aboutPlugin(int nPluginIdx, Window *pWndParent);
    void uninstPlugin(int nPluginIdx, Window *pWndParent);

    void onPlayPos(int uPos);
    void onLyricsChanged();
    void onSongChanged();

    virtual int getLineCount();
    virtual bool getLyricsOfLine(int nLine, char szLyrics[], int nBuffLen, int &beginTime, int &endTime);
    virtual int getCurLine();
    virtual int getPlayPos();
    virtual bool getMediaFile(char szFile[], int nBuffLen);
#ifdef _WIN32
    virtual HWND getMainWnd();
#endif

protected:
    typedef vector<Plugins>        V_PLUGINS;
    V_PLUGINS                   m_vPlugins;
    bool                        m_bActive;
    uint32_t                    m_dwNotifyFlag;

    int                         m_nCurLine;
    bool                        m_bHalfOfCurLine;
    int                         m_nCLBegTime, m_nCLHalftime, m_nCLendtime; // CL = cur line
    int                         m_nNLBegTime, m_nNLEndtime;

};

extern CLyricsOutPluginMgr g_lyrOutPlguinMgr;

#endif // !defined(MPlayerUI_mac_LyricsOutPluginMgr_h)
