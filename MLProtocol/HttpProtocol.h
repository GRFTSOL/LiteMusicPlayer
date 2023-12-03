#pragma once

#ifndef MLProtocol_HttpProtocol_h
#define MLProtocol_HttpProtocol_h


#include "ClientCom.h"


#define __HTTP_PORT         80

class CHttpProtocol {
public:
    enum HTTP_VER {
        HTTP_V_1_0,
        HTTP_V_1_1,
        HTTP_V_UK,
    };

public:
    CHttpProtocol();
    virtual ~CHttpProtocol();

#ifdef WIN32
    static bool httpTimeStrToSysTime(cstr_t szHttpTime, SYSTEMTIME &sysTime);
#endif

public:
    virtual int readHead();
    virtual int writeHead() = 0;

    virtual int readData(string *pBuff, int nLenMax) = 0;
    virtual int writeData(const void *pData, size_t nLen) = 0;

    virtual int getHttpVersion(HTTP_VER &ver) { ver = m_httpVer; return ERR_OK; }
    virtual int setHttpVersion(HTTP_VER ver) { m_httpVer = ver; return ERR_OK; }

    int setNetFile(CNetFile *pFile) { m_pNetFile = pFile; return ERR_OK; }

    void setCacheData(const void *lpData, int nLen);

protected:
    virtual int parseHead() = 0;

    //
    //protected:
    //    virtual int getPropValue(const char * szName, string &strValue);
    //    virtual int setPropValue(const char * szName, const char * szValue);

protected:
    bool                        m_bHeadParsed;
    HTTP_VER                    m_httpVer;
    string                      m_buffHead;
    CNetFile                    *m_pNetFile;
    int                         m_nHeadEndPos;
    uint32_t                    m_dwContentLen;

    string                      m_strCookie;
    //char            m_szCookie[512];

};

// GET /a.rar HTTP/1.1
// Host: localhost
// Accept: */*
// Referer: http://localhost
// Range: bytes=%d-
// User-Agent: Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)
// Pragma: no-cache
// Cache-Control: no-cache
// Connection: close

// 使用HTTP代理
// HTTP 代理连接的服务器为代理服务器，端口为代理端口，
// URL 为包含域名的全路径
// Host: %http-host%
// Proxy-Authorization: Basic dGVzdDoxMjM=
// Proxy-Authorization: Basic User:Pass
// Proxy-Connection: close
class CHttpRequestProtocol : public CHttpProtocol {
public:
    enum REQUEST_TYPE {
        RQ_UNKNOWN,
        RQ_GET,
        RQ_POST,
    };

    enum REQUEST_PROPERTY {
        RP_HOST                     = 1 << 0,
        RP_ACCEPT                   = 1 << 1,
        RP_REFERER                  = 1 << 2,
        RP_CONTENT_LENGTH           = 1 << 3,
        RP_USER_PWD                 = 1 << 4,
        RP_CONTENT_TYPE             = 1 << 5,
        RP_USER_AGENT               = 1 << 6,
        RP_RANGE                    = 1 << 7,
        RP_CONNECTION               = 1 << 8,
        RP_PRAGMA                   = 1 << 9,
        RP_CACHE_CONTROL            = 1 << 10,
        RP_COOKIE                   = 1 << 11,
        RP_PROXY_AUTHORIZATION      = 1 << 12,

        RP_BASE_GET                 = RP_HOST | RP_ACCEPT | RP_REFERER | RP_USER_AGENT,
        RP_BASE_POST                = RP_HOST | RP_ACCEPT | RP_REFERER | RP_USER_AGENT | RP_CONTENT_LENGTH,
        RP_POST_MIN                 = RP_HOST | RP_USER_AGENT | RP_CONTENT_LENGTH,
    };

public:
    CHttpRequestProtocol();
    virtual ~CHttpRequestProtocol();

public:
    void setProxyBase64Pass(bool bEnableProxy, const char * szBase64ProxyUserPass = nullptr);
    bool isUseProxy() { return m_bUseProxy; }
    // const char * GetProxyServer() { return m_strProxyServer.c_str(); }

    virtual int writeHead();

    virtual int readData(string *pBuff, int nLenMax);
    virtual int writeData(const void *pData, size_t nLen);

    virtual int parseHead();

    void setAvailProp(uint32_t dwProp = RP_BASE_GET);
    uint32_t getAvailProp() { return m_dwAvailProp; }

    REQUEST_TYPE getRequestType();
    void setRequestType(REQUEST_TYPE rqType);

    int getContentLength(long &nLength);
    void setContentLength(long nLength);
    void setRange(uint32_t dwStart, uint32_t dwEnd);

    virtual cstr_t getPropValue(REQUEST_PROPERTY requestProperty);
    virtual void setPropValue(REQUEST_PROPERTY requestProperty, const char * szValue);

    cstr_t getUrl() { return m_strUrl.c_str(); }
    void setUrl(const char * szUrl);
    void setHost(const char * szHost, int nPort = __HTTP_PORT);

    bool isCookieAvail() { return (m_dwAvailProp & RP_COOKIE) == RP_COOKIE; }
    cstr_t getCookie() { if (isCookieAvail()) return m_strCookie.c_str(); else return ""; }
    void setCookie(const char *szCookie) {
        m_strCookie = szCookie;
        m_dwAvailProp |= RP_COOKIE;
    }

protected:
    uint32_t                    m_dwAvailProp;
    REQUEST_TYPE                m_RequestType;

    // char                m_szUrl[512];
    string                      m_strUrl;

    int                         m_nHostPort;
    char                        m_szHost[128];
    char                        m_szAccept[128];
    char                        m_szRefer[512];
    // uint32_t                m_dwContentLen;
    char                        m_szUserPwd[128];
    char                        m_szContentType[128];
    char                        m_szUserAgent[128];
    uint32_t                    m_dwRangeBeg;
    uint32_t                    m_dwRangeEnd;
    char                        m_szConnection[128];
    char                        m_szPragma[128];
    char                        m_szCacheControl[128];

    //
    // proxy
    bool                        m_bUseProxy;
    string                      m_strProxyServer;
    string                      m_strBase64ProxyUserPass;

};

/*
HTTP/1.1 200 OK
Server: Microsoft-IIS/5.1
Connection: close
Date: Wed, 14 Jul 2004 08:45:03 GMT
Location: localstart.asp
Content-length: 1075926
Content-Type: application/octet-stream
Last-Modified: Wed, 21 Apr 2004 01:52:19 GMT
ETag: "d62c11484327c41:826"
Content-Range: bytes 1843889-2950222/2950223
set-Cookie: ASPSESSIONIDAQCCTAQC=PMFLKGLAAJMGGNHCGADKMEAC; path=/
Content-Disposition: attachment;filename=l黎明 - That's Life.lrc
Cache-control: private
WWW-Authenticate: Negotiate
WWW-Authenticate: NTLM
WWW-Authenticate: Basic realm="localhost"
*/
class CHttpReturnProtocol : public CHttpProtocol {
public:
    enum RET_PROPERTY {
        RP_SERVER                   = 1 << 0,
        RP_CONNECTION               = 1 << 1,
        RP_DATE                     = 1 << 2,
        RP_LOCATION                 = 1 << 3, // for redirect...
        RP_CONTENT_TYPE             = 1 << 4,
        RP_CONTENT_RANGE            = 1 << 5,
        RP_LAST_MODIFIED            = 1 << 6,
        RP_ETAG                     = 1 << 7,
        RP_CONTENT_LENGTH           = 1 << 8,
        RP_SET_COOKIE               = 1 << 9,
        RP_CONTENT_DISP_FILE        = 1 << 10,
        RP_CACHE_CONTROL            = 1 << 11,

        RP_BASE_RET                 = RP_SERVER | RP_CONNECTION | RP_DATE | RP_CONTENT_TYPE | RP_CONTENT_LENGTH,
        RP_MIN_RET                  = RP_SERVER | RP_CONNECTION | RP_CONTENT_LENGTH,
    };

public:
    CHttpReturnProtocol();
    virtual ~CHttpReturnProtocol();

    virtual int writeHead();

    virtual int readData(string *pBuff, int nLenMax);
    virtual int writeData(const void *pData, size_t nLen);

    int parseHead();

    uint32_t getAvailProp() { return m_dwAvailProp; }
    void setAvailProp(uint32_t dwProp = RP_BASE_RET);

    int getContentLength(long &nLength);
    void setContentLength(long nLength);
    void setRange(uint32_t dwStart, uint32_t dwEnd);

    virtual const char * getPropValue(RET_PROPERTY retProperty);
    virtual void setPropValue(RET_PROPERTY retProperty, const char * szValue);

    const char * getResult(int &nCode);
    int getResultCode() { return m_dwResultCode; }

    void setServer(const char * szServer);
    void setConnection(const char * szConnection);

    static int resultCodeToErrorCode(int nResultCode);

    bool isCookieAvail() { return (m_dwAvailProp & RP_SET_COOKIE) == RP_SET_COOKIE; }
    cstr_t getCookie() { if (isCookieAvail()) return m_strCookie.c_str(); else return ""; }
    void setCookie(const char *szCookie) {
        m_strCookie = szCookie;
        m_dwAvailProp |= RP_SET_COOKIE;
    }

    void setContentType(const char *szContentType) {
        strcpy_safe(m_szContentType, CountOf(m_szContentType), szContentType);
        m_dwAvailProp |= RP_CONTENT_TYPE;
    }

    void setDate(const char *szDate) {
        strcpy_safe(m_szDate, CountOf(m_szDate), szDate);
        m_dwAvailProp |= RP_DATE;
    }

protected:
    uint32_t                    m_dwAvailProp;

    uint32_t                    m_dwResultCode;
    char                        m_szResult[128];

    char                        m_szServer[128];
    char                        m_szConnection[128];
    char                        m_szDate[128];
    char                        m_szContentType[128];
    char                        m_szLastModified[128];
    char                        m_szEtag[128];
    char                        m_szLocation[512];
    char                        m_szCacheControl[128];
    char                        m_szContentFileName[128]; // Content-Disposition: attachment;filename=l黎明 - That's Life.lrc
    bool                        m_bTransfEncChunked;
    // char        m_szContentLen
    uint32_t                    m_dwContentLen;
    uint32_t                    m_dwRangeBeg, m_dwRangeEnd;

};

int getMonthByStr(cstr_t szMonth);
int getDayOfWeekByStr(cstr_t szDayOfWeek);

#endif // !defined(MLProtocol_HttpProtocol_h)
