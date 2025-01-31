#include "HttpClient.h"


int downloadUrl(cstr_t szUrl, cstr_t szRefer, int &nHttpCode, string &buffRet, cstr_t szUserAgent) {
    CHttpClient httpClient;
    int nRet;

    nRet = httpClient.init(szUrl, szRefer);
    if (nRet != ERR_OK) {
        DBG_LOG1("init Error: %s", (cstr_t)Error2Str(nRet));
        return nRet;
    }

    nRet = httpClient.connect();
    if (nRet != ERR_OK) {
        DBG_LOG1("connect Error: %s", (cstr_t)Error2Str(nRet));
        return nRet;
    }

    if (szUserAgent != nullptr) {
        httpClient.setUserAgent(szUserAgent);
    }

    nRet = httpClient.sendRequest();
    if (nRet != ERR_OK) {
        DBG_LOG1("sendRequest Error=%s", (cstr_t)Error2Str(nRet));
        httpClient.close();
        return nRet;
    }

    nRet = httpClient.getRespond(nHttpCode, buffRet);
    if (nRet != ERR_OK) {
        DBG_LOG1("getRespond Error=%s", (cstr_t)Error2Str(nRet));
        httpClient.close();
        return nRet;
    }
    httpClient.close();

    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////

CHttpClient::CHttpClient() {
    m_strUserAgent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1309.0 Safari/537.17";
    m_bUseProxy = false;
    m_nHostPort = 80;
    m_nServerPort = 80;
}

CHttpClient::~CHttpClient() {

}

int CHttpClient::init(cstr_t szUrl, cstr_t szReferUrl) {
    string scheme, path;

    if (!urlParse(szUrl, scheme, m_strHost, m_nHostPort, path)) {
        return ERR_HTTP_BAD_URL;
    }

    if (strcasecmp(scheme.c_str(), "http") != 0) {
        return ERR_HTTP_BAD_URL;
    }

    if (m_nHostPort == -1) {
        m_nHostPort = __HTTP_PORT;
    }

    m_httpRequest.setNetFile(&m_NetFile);
    m_httpRequest.setHost(m_strHost.c_str(), m_nHostPort);
    m_httpRequest.setRequestType(CHttpRequestProtocol::RQ_GET);

    m_httpRequest.setUrl(path.c_str());

    m_httpRequest.setAvailProp(CHttpRequestProtocol::RP_BASE_GET);
    m_httpRequest.setPropValue(CHttpRequestProtocol::RP_HOST, m_strHost.c_str());
    m_httpRequest.setPropValue(CHttpRequestProtocol::RP_ACCEPT, "*/*");
    if (szReferUrl) {
        m_httpRequest.setPropValue(CHttpRequestProtocol::RP_REFERER, szReferUrl);
    }
    m_httpRequest.setPropValue(CHttpRequestProtocol::RP_USER_AGENT, m_strUserAgent.c_str());

    m_httpReturn.setNetFile(&m_NetFile);
    m_httpReturn.setAvailProp(CHttpReturnProtocol::RP_MIN_RET);
    m_httpReturn.setServer("MusicPlayer Server/1.1");
    m_httpReturn.setConnection("close");

    setProxy(false, nullptr, 80);

    return ERR_OK;
}

void CHttpClient::setHostPort(int nPort) {
    m_nHostPort = nPort;
    if (!m_bUseProxy) {
        m_nServerPort = nPort;
    }
}

int CHttpClient::connect() {
    if (m_ClientCom.isValid()) {
        m_ClientCom.close();
    }

    int nRet;
    nRet = m_ClientCom.connect(m_strServer.c_str(), m_nServerPort);
    if (nRet != ERR_OK) {
        return nRet;
    }

    m_NetFile.attach(m_ClientCom.getSocket());
    m_NetFile.setTimeOut(20000); // 20 second

    return ERR_OK;
}

void CHttpClient::setProxy(bool bEnableProxy, cstr_t szProxyServer, int nPort, cstr_t szBase64ProxyUserPass) {
    if (bEnableProxy && !isEmptyString(szProxyServer)) {
        m_strServer = szProxyServer;
        m_nServerPort = nPort;

        m_httpRequest.setProxyBase64Pass(true, szBase64ProxyUserPass);
    } else {
        m_strServer = m_strHost;
        m_nServerPort = m_nHostPort;
    }
}

int CHttpClient::sendRequest() {
    m_httpRequest.setNetFile(&m_NetFile);
    //    m_httpRequest.setContentLength();
    return m_httpRequest.writeHead();
    //    return m_httpRequest.writeData(pMsgBuff->getBuffer(), pMsgBuff->getLength());
}

int CHttpClient::sendPostRequest(const void *lpPostData, size_t len) {
    m_httpRequest.setRequestType(CHttpRequestProtocol::RQ_POST);
    m_httpRequest.setContentLength(len);

    m_httpRequest.setNetFile(&m_NetFile);
    //    m_httpRequest.setContentLength();
    int nRet = m_httpRequest.writeHead();
    if (nRet != ERR_OK) {
        return nRet;
    }

    return m_httpRequest.writeData((void *)lpPostData, len);
}

int CHttpClient::getRespond(int &nRetCode, string &buffRet) {
    int nRet;

    buffRet.clear();

    // HTTP 消息
    m_httpReturn.setNetFile(&m_NetFile);
    nRet = m_httpReturn.readHead();
    if (nRet != ERR_OK) {
        return nRet;
    }

    nRet = m_httpReturn.readData(&buffRet, 1024 * 1000);
    if (nRet == ERR_OK) {
        nRetCode = m_httpReturn.getResultCode();
        nRet = CHttpReturnProtocol::resultCodeToErrorCode(nRetCode);
    }

    return nRet;
}

void CHttpClient::setUserAgent(cstr_t szUserAgent) {
    m_strUserAgent = szUserAgent;
}
