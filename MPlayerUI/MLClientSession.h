#pragma once

#include "../MLProtocol/MLProtocol.h"


struct ServerCfg {
    char                        szServer[32];
    int                         nPort;
};

#define SERVER_COUNT        1

extern ServerCfg g_LyrServers[SERVER_COUNT];
extern int g_nCurLyrServer;

extern int g_nPortSearchServer;
extern int g_nPortDownloadServer;

/*
***** To prevent DDos attach: *****
1. open as many port as possible on server, If any connection error occurs,
   so MusicPlayer client tries connect port from 80 to 180, about 100 ports.

*/

class CMLClientSession {
public:
    CMLClientSession();
    virtual ~CMLClientSession();

    void init(cstr_t szAppName);

    void setUploader(cstr_t szUser, cstr_t szPwd, bool bSavePwd);
    cstr_t getLoginName() const { return m_strLoginName.c_str(); }
    cstr_t getLoginPwd() const { return m_strPwdMask.c_str(); }

    void setProxy(bool bEnableProxy, cstr_t szProxyServer, int nPort, cstr_t szBase64ProxyUserPass = nullptr);

    int connect();

    int close() { m_bClosing = true; return m_ClientCom.close(); }

    bool isLogined() { return m_bLogined; }

    int login(string &strMsgRet);

    // search lyrics
    int search(bool bOnlyMatched, cstr_t szArtist, cstr_t szTitle, int nRequestPage, MLMsgRetSearch &retSearch);

    // search lyrics by batch
    int batchSearch(MLListSearchItems &listToSearch, MLMsgRetBatchSearch &retSearch);

    // upload lyrics
    int upload(const string &lyricsContent, string &strLyricsId, string &strMsg);

protected:
    void prepareClientInfo(CXMLMsgCmd &cmd);

    int sendXmlCommand(CXMLMsgCmd &cmd);

    // process common return message
    int processCommonRetMsg(MLRetMsg *pretMsg, MLMsgCmd cmdOrgSend);

public:
    static cstr_t getDefaultServer() { return g_LyrServers[g_nCurLyrServer].szServer; }
    static int getDefaultServerPort() { return g_nPortSearchServer; }

protected:
    CClientCom                  m_ClientCom;
    CMLProtocol                 m_MsgProtocol;
    CNetFile                    m_NetFile;
    bool                        m_bInitialzed;

    string                      m_strLoginName;
    string                      m_strPwdMask;
    string                      m_strClient;

    bool                        m_bLogined;

    bool                        m_bClosing;

    //
    // proxy
    string                      m_strBase64ProxyUserPass;
    string                      m_proxyServer;
    int                         m_nProxyPort;
    bool                        m_bEnableProxy;

};
