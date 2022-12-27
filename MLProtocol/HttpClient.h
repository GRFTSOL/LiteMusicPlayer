#pragma once

#include "ClientCom.h"
#include "MLProtocol.h"


int downloadUrl(cstr_t szUrl, cstr_t szRefer, int &nHttpCode, string &buffRet, cstr_t szUserAgent = nullptr);

class CHttpClient {
public:
    CHttpClient();
    virtual ~CHttpClient();

    int init(cstr_t szUrl, cstr_t szReferUrl);

    void setHostPort(int nPort);
    int getHostPort() const { return m_nHostPort; }

    int connect();

    int sendRequest();
    int sendPostRequest(const void *lpPostData, size_t len);

    int getRespond(int &nRetCode, string &buffRet);

    int close() { return m_ClientCom.close(); }

    cstr_t getServer() { return m_strServer.c_str(); }
    int getServerPort() { return m_nServerPort; }

    void setProxy(bool bEnableProxy, cstr_t szProxyServer, int nPort, cstr_t szBase64ProxyUserPass = nullptr);
    void setUserAgent(cstr_t szUserAgent);

    CHttpRequestProtocol &getHttpRequestProtocol() { return m_httpRequest; }

protected:
    CClientCom                  m_ClientCom;
    CHttpRequestProtocol        m_httpRequest;
    CHttpReturnProtocol         m_httpReturn;
    CNetFile                    m_NetFile;

    string                      m_strHost;
    int                         m_nHostPort;
    string                      m_strServer;
    int                         m_nServerPort;
    string                      m_strUserAgent;

    //
    // proxy
    bool                        m_bUseProxy;
    string                      m_strProxyServer;
    string                      m_strBase64ProxyUserPass;

};
