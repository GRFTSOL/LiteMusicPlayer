#include "HttpProtocol.h"


#define SZ_RETURNA          "\r\n"

#define CONST_STRLEN(str)   (CountOf(str) - 1)

#define SZ_HH_SERVER        "Server: "
#define SZ_HH_CONNECTION    "Connection: "
#define SZ_HH_DATE          "Date: "
#define SZ_HH_LOCATION      "Location: "
#define SZ_HH_CONTENT_TYPE  "Content-Type: "
#define SZ_HH_LAST_MODIFIED "Last-Modified: "
#define SZ_HH_ETAG          "ETag: "
#define SZ_HH_CONTENT_LENGTH "Content-length: "
#define SZ_HH_CONTENT_RANGE "Content-Range: "
#define SZ_HH_SET_COOKIE    "set-Cookie: "
#define SZ_HH_CACHE_CONTROL "Cache-control: "

#define SZ_HH_CONTENT_DISP_FILENAME "Content-Disposition: attachment;filename="
#define SZ_TRANSF_ENC_CHUNKED       "chunked"
#define SZ_HH_TRANSF_ENCODING       "Transfer-Encoding:"

#define READ_BLOCK_SIZE     2048

cstr_t    _szStdMonth[12] = {
    "jan", "feb", "mar", "apr", "may", "jun",
    "jul", "aug", "sep", "oct", "nov", "dec"
};

int getMonthByStr(cstr_t szMonth) {
    for (int i = 0; i < 12; i++) {
        if (strncasecmp(_szStdMonth[i], szMonth, 3) == 0) {
            return i + 1;
        }
    }
    return -1;
}

cstr_t    _szStdDayOfWeek[7] = {
    "sun", "mon", "tue", "wed", "thu", "fri", "sat"
};

int getDayOfWeekByStr(cstr_t szDayOfWeek) {
    for (int i = 0; i < 7; i++) {
        if (strncasecmp(_szStdDayOfWeek[i], szDayOfWeek, 3) == 0) {
            return i + 1;
        }
    }
    return -1;
}

CHttpProtocol::CHttpProtocol() {
    m_bHeadParsed = false;
    m_httpVer = HTTP_V_1_1;
    m_pNetFile = nullptr;
    m_nHeadEndPos = 0;
    m_dwContentLen = 0;
}

CHttpProtocol::~CHttpProtocol() {

}

#ifdef WIN32
bool CHttpProtocol::httpTimeStrToSysTime(cstr_t szHttpTime, SYSTEMTIME &sysTime) {
    SYSTEMTIME SysTime;
    char szDayOfWeek[64];
    char szMonth[64];
    char szGMT[64];

    // Mon, 04 Nov 2002 15:41:19 GMT
    if (8 != sscanf(szHttpTime, "%3s, %02d %3s %d %d:%d:%d %s",
        szDayOfWeek, &SysTime.wDay, szMonth, &SysTime.wYear,
        &SysTime.wHour, &SysTime.wMinute, &SysTime.wSecond, szGMT)) {
        // GetSystemTime(&SysTime);
        return false;
    } else {
        // 转换星期几？
        SysTime.wDayOfWeek = getDayOfWeekByStr(szDayOfWeek);
        if (SysTime.wDayOfWeek == -1) {
            SysTime.wDayOfWeek = 1;
        }

        // 转换月
        SysTime.wMonth = getMonthByStr(szMonth);
        if (SysTime.wMonth == -1) {
            SysTime.wMonth = 1;
        }
    }

    return true;
}
#endif

int CHttpProtocol::readHead() {
    int nLastPos = 0;
    int i = 0;

    const int nHttpHeadEndTagLen = 4;

    while (1) {
        int nRead;
        int nRet;

        if (nLastPos > nHttpHeadEndTagLen) {
            nLastPos -= nHttpHeadEndTagLen;
        }

        StringView bufHead(m_buffHead);
        m_nHeadEndPos = bufHead.strstr( "\r\n\r\n", nLastPos);
        if (m_nHeadEndPos != -1) {
            m_nHeadEndPos += nHttpHeadEndTagLen;
            return parseHead();
        }
        nLastPos = (int)m_buffHead.size();

        nRet = m_pNetFile->read(m_buffHead, READ_BLOCK_SIZE, &nRead);
        if (nRet == ERR_OK) {
            if (nRead == 0) {
                // 接收完成，但是没有找到HTTP头结束标志
                ERR_LOG2("Can't find the end pos of Http header. Buffer: %d, %S", m_buffHead.size(), m_buffHead.c_str());
                return ERR_HTTP_HEAD_NOT_END;
            }
        } else {
            return nRet;
        }
        i++;
    }

    return ERR_FALSE;
}

void CHttpProtocol::setCacheData(const void *lpData, int nLen) {
    m_buffHead.clear();
    m_buffHead.append((const char *)lpData, nLen);
}


//////////////////////////////////////////////////////////////////////////
//
CHttpRequestProtocol::CHttpRequestProtocol() {
    m_bUseProxy = false;
    m_dwAvailProp = RP_BASE_GET;
    m_RequestType = RQ_GET;

    // emptyStr(m_szUrl);

    m_nHostPort = __HTTP_PORT;
    emptyStr(m_szHost);
    emptyStr(m_szAccept);
    emptyStr(m_szRefer);
    m_dwContentLen = 0;
    emptyStr(m_szUserPwd);
    emptyStr(m_szContentType);
    emptyStr(m_szUserAgent);
    m_dwRangeBeg = 0;
    m_dwRangeEnd = 0;
    emptyStr(m_szConnection);
    emptyStr(m_szPragma);
    emptyStr(m_szCacheControl);
}

CHttpRequestProtocol::~CHttpRequestProtocol() {
}

void CHttpRequestProtocol::setUrl(const char * szUrl) {
    m_strUrl.clear();

    if (szUrl[0] != '/') {
        m_strUrl += '/';
    }

    m_strUrl += szUrl;
}

void CHttpRequestProtocol::setHost(const char * szHost, int nPort) {
    m_dwAvailProp |= RP_HOST;
    strcpy_safe(m_szHost, CountOf(m_szHost), szHost);
    m_nHostPort = nPort;
}

void CHttpRequestProtocol::setProxyBase64Pass(bool bEnableProxy, const char * szBase64ProxyUserPass) {
    m_bUseProxy = bEnableProxy;
    if (bEnableProxy) {
        if (szBase64ProxyUserPass && !isEmptyString(szBase64ProxyUserPass)) {
            m_strBase64ProxyUserPass = szBase64ProxyUserPass;
            m_dwAvailProp |= RP_PROXY_AUTHORIZATION;
        } else {
            m_strBase64ProxyUserPass = "";
            m_dwAvailProp &= ~RP_PROXY_AUTHORIZATION;
        }
    }
}

int CHttpRequestProtocol::writeHead() {
    string urlNew;

    if (m_bUseProxy) {
        if (m_nHostPort != __HTTP_PORT) {
            urlNew = stringPrintf("http://%s:%d", m_szHost, m_nHostPort).c_str();
        } else {
            urlNew = stringPrintf("http://%s", m_szHost).c_str();
        }
        if (m_strUrl[0] != '/') {
            urlNew += "/";
        }
        urlNew += m_strUrl;
    } else {
        urlNew = m_strUrl;
    }

    // "GET %s HTTP/%s\r\n"
    if (m_RequestType == RQ_GET) {
        m_buffHead = "GET ";
    } else {
        m_buffHead = "POST ";
    }
    m_buffHead += urlNew;
    m_buffHead += " HTTP/";
    m_buffHead += m_httpVer == HTTP_V_1_1 ? "1.1" : "1.0";
    m_buffHead += "\r\n";

    if (isFlagSet(m_dwAvailProp, RP_HOST) && !isEmptyString(m_szHost)) {
        m_buffHead += "Host: ";
        m_buffHead += m_szHost;
        m_buffHead += "\r\n";
    }

    if (isFlagSet(m_dwAvailProp, RP_ACCEPT) && !isEmptyString(m_szAccept)) {
        m_buffHead += "Accept: ";
        m_buffHead += m_szAccept;
        m_buffHead += "\r\n";
    }

    if (isFlagSet(m_dwAvailProp, RP_REFERER) && !isEmptyString(m_szRefer)) {
        m_buffHead += "Referer: ";
        m_buffHead += m_szRefer;
        m_buffHead += "\r\n";
    }

    if (isFlagSet(m_dwAvailProp, RP_CONTENT_LENGTH) && m_dwContentLen > 0) {
        m_buffHead += SZ_HH_CONTENT_LENGTH;
        m_buffHead += itos(m_dwContentLen);
        m_buffHead += "\r\n";
    }
    //
    //    if (m_dwAvailProp & RP_USER_PWD)
    //    {
    //        szBuff = m_buffHead.allocMemory(nSize) + m_buffHead.size();
    //        nLen = sprintf(szBuff, "Proxy-Authorization: %s", m_szUs);
    //        m_buffHead.setLength(m_buffHead.size() + nLen);
    //        nSize += nLen;
    //    }

    if (isFlagSet(m_dwAvailProp, RP_USER_AGENT) && !isEmptyString(m_szUserAgent)) {
        m_buffHead += "User-Agent: ";
        m_buffHead += m_szUserAgent;
        m_buffHead += "\r\n";
    }

    if (isFlagSet(m_dwAvailProp, RP_CONTENT_TYPE) && !isEmptyString(m_szContentType)) {
        m_buffHead += SZ_HH_CONTENT_TYPE;
        m_buffHead += m_szContentType;
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_RANGE) {
        if (m_dwRangeEnd > m_dwRangeBeg) {
            m_buffHead += stringPrintf("Range: bytes=%d-%d\r\n", m_dwRangeBeg, m_dwRangeEnd);
        } else if (m_dwRangeEnd == 0) {
            m_buffHead += stringPrintf("Range: bytes=%d-\r\n", m_dwRangeBeg);
        }
    }

    if (isFlagSet(m_dwAvailProp, RP_CONNECTION) && !isEmptyString(m_szConnection)) {
        m_buffHead += SZ_HH_CONNECTION;
        m_buffHead += m_szConnection;
        m_buffHead += "\r\n";
    }

    if (isFlagSet(m_dwAvailProp, RP_PRAGMA) && !isEmptyString(m_szPragma)) {
        m_buffHead += "Pragma: ";
        m_buffHead += m_szPragma;
        m_buffHead += "\r\n";
    }

    if (isFlagSet(m_dwAvailProp, RP_CACHE_CONTROL) && !isEmptyString(m_szCacheControl)) {
        m_buffHead += SZ_HH_CACHE_CONTROL;
        m_buffHead += m_szCacheControl;
        m_buffHead += "\r\n";
    }

    if (isFlagSet(m_dwAvailProp, RP_COOKIE) && m_strCookie.size() > 0) {
        m_buffHead += "Cookie: ";
        m_buffHead += m_strCookie.c_str();
        m_buffHead += "\r\n";
    }

    if (isFlagSet(m_dwAvailProp, RP_PROXY_AUTHORIZATION) && m_bUseProxy && !m_strBase64ProxyUserPass.empty()) {
        m_buffHead += "Proxy-Authorization: Basic ";
        m_buffHead += m_strBase64ProxyUserPass.c_str();
        m_buffHead += "\r\n";
    }

    m_buffHead += SZ_RETURNA;

    // DBG_LOGDUMPS(m_buffHead.c_str(), m_buffHead.size());

    return m_pNetFile->write(m_buffHead.data(), (int)m_buffHead.size(), nullptr);
}

int CHttpRequestProtocol::readData(string *pBuff, int nLenMax) {
    if (m_RequestType == RQ_POST) {
        if ((int)m_buffHead.size() > m_nHeadEndPos) {
            pBuff->append(m_buffHead.data() + m_nHeadEndPos, m_buffHead.size() - m_nHeadEndPos);
        }

        int nRet = ERR_OK;
        if (m_dwContentLen == -1) {
            nRet = m_pNetFile->readTillDataStreamClosed(*pBuff);
        } else if (pBuff->size() < m_dwContentLen) {
            nRet = m_pNetFile->readExactly(*pBuff, int(m_dwContentLen - pBuff->size()));
        } else if (pBuff->size() > m_dwContentLen) {
            pBuff->resize(m_dwContentLen);
        }
        if (nRet != ERR_OK) {
            return nRet;
        }
    }

    return ERR_OK;
}

int CHttpRequestProtocol::writeData(const void *pData, size_t nLen) {
    //    DBG_LOGDUMPS((char *)pData, nLen);
    return m_pNetFile->write((const char *)pData, (int)nLen, nullptr);
}

int CHttpRequestProtocol::parseHead() {
    cstr_t szHead;
    cstr_t szPos;
    cstr_t szEnd;
    cstr_t szEnd2;
    szPos = szHead = m_buffHead.data();

    m_dwAvailProp = 0;

    if (strncasecmp(szHead, "GET", 3) == 0) {
        m_RequestType = RQ_GET;
        szPos += 4;
    } else if (strncasecmp(szHead, "POST", 4) == 0) {
        m_RequestType = RQ_POST;
        szPos += 5;
    } else {
        return ERR_HTTP_BAD_REQUEST;
    }

    //
    // /a.rar HTTP/1.1
    szEnd = strstr(szPos, SZ_RETURNA);
    if (szEnd == nullptr) {
        return ERR_HTTP_BAD_FORMAT;
    }
    szEnd2 = strchr(szPos, 0x20);
    if (szEnd2 == nullptr || szEnd2 >= szEnd) {
        return ERR_HTTP_BAD_FORMAT;
    }
    // strncpy_safe(m_szUrl, 512, szPos, (int)(szEnd2 - szPos));
    m_strUrl.erase();
    m_strUrl.append(szPos, szEnd2);

    szEnd2++;
    if (strncmp(szEnd2, "HTTP/", 5) != 0) {
        return ERR_HTTP_BAD_HTTP_PRO_TYPE;
    }
    szEnd2 += 5;

    if (strncmp(szEnd2, "1.1", 2) == 0) {
        m_httpVer = HTTP_V_1_1;
    } else if (strncmp(szEnd2, "1.0", 2) == 0) {
        m_httpVer = HTTP_V_1_0;
    } else {
        m_httpVer = HTTP_V_UK;
    }

    if (m_httpVer != HTTP_V_UK) {
        szEnd2+=3;
        if (strncmp(szEnd2, SZ_RETURNA, 2) != 0) {
            return ERR_HTTP_BAD_HTTP_PRO_TYPE;
        }
    }

    //
    //
    szPos = szEnd + 2;
    while (1) {
        if (strncmp(szPos, SZ_RETURNA, 2) == 0) {
            break;
        }
        szEnd = strstr(szPos, SZ_RETURNA);
        if (!szEnd) {
            return ERR_HTTP_BAD_FORMAT;
        }
        if (strncasecmp(szPos, "Host: ", 6) == 0) {
            szPos += 6;
            strncpy_safe(m_szHost, CountOf(m_szHost), szPos, size_t(szEnd - szPos));
            m_dwAvailProp |= RP_HOST;
        } else if (strncasecmp(szPos, "Accept: ", 8) == 0) {
            szPos += 8;
            strncpy_safe(m_szAccept, CountOf(m_szAccept), szPos, size_t(szEnd - szPos));
            m_dwAvailProp |= RP_ACCEPT;
        } else if (strncasecmp(szPos, "Referer: ", 9) == 0) {
            szPos += 9;
            strncpy_safe(m_szRefer, CountOf(m_szRefer), szPos, size_t(szEnd - szPos));
            m_dwAvailProp |= RP_REFERER;
        } else if (strncasecmp(szPos, "Range: ", 7) == 0) {
            //assert(0);
            szPos += 7;
            // strncpy_safe(m_szHost, CountOf(m_szHost), szPos, size_t(szEnd - szPos));
            m_dwAvailProp |= RP_RANGE;
        } else if (strncasecmp(szPos, SZ_HH_CONTENT_LENGTH, 16) == 0) {
            szPos += 16;
            m_dwContentLen = atoi(szPos);
            m_dwAvailProp |= RP_CONTENT_LENGTH;
        } else if (strncasecmp(szPos, "User-Agent: ", 12) == 0) {
            szPos += 12;
            strncpy_safe(m_szUserAgent, CountOf(m_szUserAgent), szPos, size_t(szEnd - szPos));
            m_dwAvailProp |= RP_USER_AGENT;
        } else if (strncasecmp(szPos, SZ_HH_CACHE_CONTROL, 15) == 0) {
            szPos += 15;
            strncpy_safe(m_szCacheControl, CountOf(m_szCacheControl), szPos, size_t(szEnd - szPos));
            m_dwAvailProp |= RP_CACHE_CONTROL;
        } else if (strncasecmp(szPos, SZ_HH_CONNECTION, 12) == 0) {
            szPos += 12;
            strncpy_safe(m_szConnection, CountOf(m_szConnection), szPos, size_t(szEnd - szPos));
            m_dwAvailProp |= RP_CONNECTION;
        } else if (strncasecmp(szPos, "Cookie: ", 8) == 0) {
            szPos += 8;
            m_strCookie = "";
            m_strCookie.append(szPos, szEnd);
            // strncpy_safe(m_szCookie, CountOf(m_szCookie), szPos, size_t(szEnd - szPos));
            m_dwAvailProp |= RP_COOKIE;
        } else {
            // char        szName[128];
            // strncpy_safe(szName, CountOf(szName), szPos, int(szEnd - szPos));
            // ERR_LOG1("unknown http request property: %s", szName);
        }
        szPos = szEnd + 2;
    }

    return ERR_OK;
}

void CHttpRequestProtocol::setAvailProp(uint32_t dwProp) {
    m_dwAvailProp = dwProp;
}

CHttpRequestProtocol::REQUEST_TYPE CHttpRequestProtocol::getRequestType() {
    return m_RequestType;
}

void CHttpRequestProtocol::setRequestType(REQUEST_TYPE rqType) {
    m_RequestType = rqType;
}

int CHttpRequestProtocol::getContentLength(long &nLength) {
    return m_dwContentLen;
}

void CHttpRequestProtocol::setContentLength(long nLength) {
    m_dwAvailProp |= RP_CONTENT_LENGTH;
    m_dwContentLen = (int)nLength;
}

void CHttpRequestProtocol::setRange(uint32_t dwStart, uint32_t dwEnd) {
    m_dwRangeBeg = dwStart;
    m_dwRangeEnd = dwEnd;
}

cstr_t CHttpRequestProtocol::getPropValue(REQUEST_PROPERTY requestProperty) {
    switch (requestProperty) {
    case RP_HOST:
        return m_szHost;
        break;
    case RP_ACCEPT:
        return m_szAccept;
        break;
    case RP_REFERER:
        return m_szRefer;
        break;
        //    case RP_CONTENT_LENGTH:
        //        m_dwContentLen = atoi(szValue);
        //        break;
    case RP_USER_PWD:
        return m_szUserPwd;
        break;
    case RP_CONTENT_TYPE:
        return m_szContentType;
        break;
    case RP_USER_AGENT:
        return m_szUserAgent;
        break;
        //    case RP_RANGE:
        //        break;
    case RP_CONNECTION:
        return m_szConnection;
        break;
    case RP_PRAGMA:
        return m_szPragma;
        break;
    case RP_CACHE_CONTROL:
        return m_szCacheControl;
        break;
    case RP_COOKIE:
        return m_strCookie.c_str();
        break;
    default:
        DBG_LOG1("CHttpRequestProtocol::getPropValue() does not support get the property: %d", requestProperty);
        break;
    }

    return nullptr;
}

void CHttpRequestProtocol::setPropValue(REQUEST_PROPERTY requestProperty, const char * szValue) {
    switch (requestProperty) {
    case RP_HOST:
        strcpy_safe(m_szHost, CountOf(m_szHost), szValue);
        break;
    case RP_ACCEPT:
        strcpy_safe(m_szAccept, CountOf(m_szAccept), szValue);
        break;
    case RP_REFERER:
        strcpy_safe(m_szRefer, CountOf(m_szRefer), szValue);
        break;
    case RP_CONTENT_LENGTH:
        m_dwContentLen = atoi(szValue);
        break;
    case RP_USER_PWD:
        strcpy_safe(m_szUserPwd, CountOf(m_szUserPwd), szValue);
        break;
    case RP_CONTENT_TYPE:
        strcpy_safe(m_szContentType, CountOf(m_szContentType), szValue);
        break;
    case RP_USER_AGENT:
        strcpy_safe(m_szUserAgent, CountOf(m_szUserAgent), szValue);
        break;
        //    case RP_RANGE:
        //        break;
    case RP_CONNECTION:
        strcpy_safe(m_szConnection, CountOf(m_szConnection), szValue);
        break;
    case RP_PRAGMA:
        strcpy_safe(m_szPragma, CountOf(m_szPragma), szValue);
        break;
    case RP_CACHE_CONTROL:
        strcpy_safe(m_szCacheControl, CountOf(m_szCacheControl), szValue);
        break;
    case RP_COOKIE:
        m_strCookie = szValue;
        // strcpy_safe(m_szCookie, CountOf(m_szCookie), szValue);
        break;
    case RP_PROXY_AUTHORIZATION:
        m_strBase64ProxyUserPass = szValue;
        break;
    default:
        DBG_LOG2("CHttpRequestProtocol::setPropValue() does not support set the property: %d, %s", requestProperty, szValue);
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
//
CHttpReturnProtocol::CHttpReturnProtocol() {
    m_dwAvailProp = RP_BASE_RET;

    m_dwResultCode = 200;
    emptyStr(m_szResult);

    emptyStr(m_szServer);
    emptyStr(m_szConnection);
    emptyStr(m_szDate);
    emptyStr(m_szContentType);
    emptyStr(m_szLastModified);
    emptyStr(m_szEtag);
    m_dwContentLen = (uint32_t)-1;
    m_dwRangeBeg = 0;
    m_dwRangeEnd = 0;
    // emptyStr(m_szCookie);
    emptyStr(m_szLocation);
    emptyStr(m_szCacheControl);
    emptyStr(m_szContentFileName); // Content-Disposition: attachment;filename=l黎明 - That's Life.lrc
    m_bTransfEncChunked = false;
}

CHttpReturnProtocol::~CHttpReturnProtocol() {
}

int CHttpReturnProtocol::writeHead() {
    char szTemp[256];

    m_buffHead.clear();

    if (m_httpVer == HTTP_V_1_1) {
        m_buffHead += "HTTP/1.1 200 OK\r\n";
    } else {
        m_buffHead += "HTTP/1.0 200 OK\r\n";
    }

    if (m_dwAvailProp & RP_SERVER) {
        m_buffHead += SZ_HH_SERVER;
        m_buffHead += m_szServer;
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_DATE) {
        m_buffHead += SZ_HH_DATE;
        m_buffHead += m_szDate;
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_LOCATION) {
        m_buffHead += SZ_HH_LOCATION;
        m_buffHead += m_szLocation;
        m_buffHead += "\r\n";
    }
    //
    //    if (m_dwAvailProp & RP_USER_PWD)
    //    {
    //        szBuff = m_buffHead.allocMemory(nSize) + m_buffHead.size();
    //        nLen = sprintf(szBuff, "Proxy-Authorization: %s", m_szUs);
    //        m_buffHead.setLength(m_buffHead.size() + nLen);
    //        nSize += nLen;
    //    }

    if (m_dwAvailProp & RP_CONTENT_LENGTH) {
        snprintf(szTemp, CountOf(szTemp), "%d", m_dwContentLen);
        m_buffHead += SZ_HH_CONTENT_LENGTH;
        m_buffHead += szTemp;
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_CONTENT_TYPE) {
        m_buffHead += SZ_HH_CONTENT_TYPE;
        m_buffHead += m_szContentType;
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_CONTENT_RANGE) {
        if (m_dwRangeEnd > m_dwRangeBeg) {
            snprintf(szTemp, CountOf(szTemp), "Range: bytes=%d-%d\r\n", m_dwRangeBeg, m_dwRangeEnd);
        } else if (m_dwRangeEnd == 0) {
            snprintf(szTemp, CountOf(szTemp), "Range: bytes=%d-\r\n", m_dwRangeBeg);
        }
        m_buffHead += szTemp;
    }

    if (m_dwAvailProp & RP_LAST_MODIFIED) {
        m_buffHead += SZ_HH_LAST_MODIFIED;
        m_buffHead += m_szLastModified;
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_ETAG) {
        m_buffHead += SZ_HH_ETAG;
        m_buffHead += m_szEtag;
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_SET_COOKIE) {
        m_buffHead += SZ_HH_SET_COOKIE;
        m_buffHead += m_strCookie.c_str();
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_CONTENT_DISP_FILE) {
        m_buffHead += SZ_HH_CONTENT_DISP_FILENAME;
        m_buffHead += m_szContentFileName;
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_CACHE_CONTROL) {
        m_buffHead += SZ_HH_CACHE_CONTROL;
        m_buffHead += m_szCacheControl;
        m_buffHead += "\r\n";
    }

    if (m_dwAvailProp & RP_CONNECTION) {
        m_buffHead += SZ_HH_CONNECTION;
        m_buffHead += m_szConnection;
        m_buffHead += "\r\n";
        //        m_buffHead += "X-Powered-By: ";
        //        m_buffHead += "PHP";
        //        m_buffHead += "\r\n";
    }

    m_buffHead += "\r\n";

    // DBG_LOGDUMPS(m_buffHead.c_str(), m_buffHead.size());
    //    DBG_LOGDUMPS(m_buffHead.getBuffer(), m_buffHead.size());

    return m_pNetFile->write(m_buffHead.data(), (int)m_buffHead.size(), nullptr);
}

//
// nSize = 0 结束
// nSize = -1 需要读取更多的数据
// nSize = -2 数据块错误
char *readTrunkSize(char *szBuffer, int &nCount, int &nSize) {
    char * szPos = szBuffer;
    char * szEnd = szBuffer + nCount;

    nSize = 0;

    if (szPos + 1 >= szEnd) {
        goto BUFFER_TOO_SMALL;
    }
    if (*szPos == '\r' && *(szPos + 1) == '\n') {
        szPos += 2;
        if (szPos + 1 >= szEnd) {
            goto BUFFER_TOO_SMALL;
        }
        if (*szPos == '\r' && *(szPos + 1) == '\n') {
            szPos += 2;
            nCount -= 2;
        }
    }

    if (szPos + 3 >= szEnd) {
        goto BUFFER_TOO_SMALL;
    }
    while (*szPos != ' ' && *szPos != '\r' && *szPos != '\n') {
        nSize *= 0x10;
        if (isDigit(*szPos)) {
            nSize += *szPos - '0';
        } else if (*szPos >= 'a' && *szPos <= 'f') {
            nSize += (*szPos - 'a' + 10);
        } else if (*szPos >= 'A' && *szPos <= 'F') {
            nSize += (*szPos - 'A' + 10);
        } else if (*szPos == 0x20) {
            szPos++;
            while (szPos < szEnd && *szPos == 0x20) {
                szPos++;
            }
            break;
        } else {
            // 无效
            nSize = -2;
            return szBuffer;
        }
        szPos++;
        if (szPos >= szEnd) {
            goto BUFFER_TOO_SMALL;
        }
    }
    while (*szPos == ' ') {
        szPos++;
    }

    if (szPos + 2 >= szEnd) {
        goto BUFFER_TOO_SMALL;
    }

    if (*szPos == '\r' && *(szPos + 1) == '\n') {
        szPos += 2;
    } else {
        nSize = -2;
        return szBuffer;
    }

    nCount -= int(szPos - szBuffer);
    assert(nCount >= 0);
    return szPos;

BUFFER_TOO_SMALL:
    nSize = -1;
    return szBuffer;
}

int CHttpReturnProtocol::readData(string *pBuff, int nLenMax) {
    if (m_bTransfEncChunked) {
        //
        // 分析m_buffHead 中部分数据
        char * szData;
        int nCount = 0;
        int nTrunkSize = 0;
        char szBuffer[2048];
        if ((int)m_buffHead.size() > m_nHeadEndPos) {
            szData = (char *)m_buffHead.data() + m_nHeadEndPos;
            nCount = (int)m_buffHead.size() - m_nHeadEndPos;
            while (nCount > 3) {
                szData = readTrunkSize(szData, nCount, nTrunkSize);
                if (nTrunkSize == -2) {
                    // Invalid trunck size!
                    DBG_LOG1("Invalid Trunk size: %s", szData - 5);
                    return ERR_HTTP_INVLIAD_TRUNK;
                } else if (nTrunkSize == -1) {
                    nTrunkSize = 0;
                    break;
                } else if (nTrunkSize == 0) {
                    return ERR_OK;
                }
                if (nTrunkSize <= nCount) {
                    pBuff->append(szData, nTrunkSize);
                    szData += nTrunkSize;
                    nCount -= nTrunkSize;
                    nTrunkSize = 0;
                } else {
                    pBuff->append(szData, nCount);
                    szData += nCount;
                    nTrunkSize -= nCount;
                    nCount = 0;
                    break;
                }
            }
            if (nCount > 0) {
                memcpy(szBuffer, szData, nCount);
            }
        }

        //
        // 接收并分析数据
        while (pBuff->size() < m_dwContentLen) {
            char *szData;
            int nRet;
            int nRead;
            szData = szBuffer;

            nRet = m_pNetFile->read(szBuffer + nCount, CountOf(szBuffer) - nCount, &nRead);
            if (nRet != ERR_OK) {
                return nRet;
            }
            if (nRead == 0) {
                if (m_dwContentLen == -1 && nCount == 0) {
                    return ERR_OK;
                } else {
                    return ERR_HTTP_DATA_NOT_FINISHED;
                }
            }
            nCount += nRead;
            do {
                if (nTrunkSize == 0) {
                    szData = readTrunkSize(szData, nCount, nTrunkSize);
                    if (nTrunkSize == -2) {
                        // Invalid trunck size!
                        DBG_LOG1("Invalid Trunk size: %s", szData);
                        return ERR_HTTP_INVLIAD_TRUNK;
                    } else if (nTrunkSize == -1) {
                        nTrunkSize = 0;
                        break;
                    } else if (nTrunkSize == 0) {
                        return ERR_OK;
                    }
                }
                if (nTrunkSize <= nCount) {
                    pBuff->append(szData, nTrunkSize);
                    szData += nTrunkSize;
                    nCount -= nTrunkSize;
                    nTrunkSize = 0;
                } else {
                    pBuff->append(szData, nCount);
                    szData += nCount;
                    nTrunkSize -= nCount;
                    nCount = 0;
                    break;
                }
            }
            while (nCount > 3);

            if (nCount > 0) {
                memmove(szBuffer, szData, nCount);
            }
        }

        if (pBuff->size() > m_dwContentLen) {
            pBuff->resize(m_dwContentLen);
        }
    } else {
        if ((int)m_buffHead.size() > m_nHeadEndPos) {
            pBuff->append(m_buffHead.data() + m_nHeadEndPos, m_buffHead.size() - m_nHeadEndPos);
        }

        int nRet = ERR_OK;
        if (m_dwContentLen == -1) {
            nRet = m_pNetFile->readTillDataStreamClosed(*pBuff);
        } else if (pBuff->size() < m_dwContentLen) {
            nRet = m_pNetFile->readExactly(*pBuff, int(m_dwContentLen - pBuff->size()));
        } else if (pBuff->size() > m_dwContentLen) {
            pBuff->resize(m_dwContentLen);
        }
        if (nRet != ERR_OK) {
            return nRet;
        }
    }

    return ERR_OK;
}

int CHttpReturnProtocol::writeData(const void *pData, size_t nLen) {
    // DBG_LOGDUMPS((char *)pData, nLen);
    return m_pNetFile->write((const char *)pData, (int)nLen, nullptr);
}

int CHttpReturnProtocol::parseHead() {
    cstr_t szHead;
    cstr_t szPos;
    cstr_t szEnd;
    szPos = szHead = m_buffHead.data();

    // DBG_LOGDUMPS(m_buffHead.c_str(), m_buffHead.size());
    m_dwAvailProp = 0;

    // HTTP/1.1 200 OK
    if (strncmp(szHead, "HTTP/", 5) == 0) {
        szPos += 5;
        if (strncmp(szPos, "1.1", 3) == 0) {
            m_httpVer = HTTP_V_1_1;
        } else if (strncmp(szPos, "1.0", 3) == 0) {
            m_httpVer = HTTP_V_1_0;
        } else {
            m_httpVer = HTTP_V_UK;
        }
        szPos = strchr(szPos, 0x20);
        if (szPos == nullptr) {
            return ERR_HTTP_BAD_FORMAT;
        }
        szPos++;

        m_dwResultCode = atoi(szPos);
        for (int i = 0; i < 3; i++) {
            if (!isDigit(*szPos)) {
                return ERR_HTTP_BAD_FORMAT;
            }
            szPos++;
        }

        szPos++;
        szEnd = strstr(szPos, SZ_RETURNA);
        if (szEnd == nullptr) {
            return ERR_HTTP_BAD_FORMAT;
        }

        strncpy_safe(m_szResult, CountOf(m_szResult), szPos, int(szEnd - szPos));
    } else {
        return ERR_HTTP_BAD_HTTP_PRO_TYPE;
    }

#define IF_PROCESS_STR_FIELD(name, szValue, rpProperty)                        \
    if (strncasecmp(szPos, name, CONST_STRLEN(name)) == 0)                        \
    {                                                                        \
        szPos += CONST_STRLEN(name);                                        \
        strncpy_safe(szValue, CountOf(szValue), szPos, size_t(szEnd - szPos));\
        m_dwAvailProp |= rpProperty;                                            \
    }

    //
    //
    szPos = szEnd + 2;
    while (1) {
        if (strncmp(szPos, SZ_RETURNA, 2) == 0 || *szPos == '\n') {
            break;
        }
        szEnd = strstr(szPos, SZ_RETURNA);
        if (!szEnd) {
            szEnd = strchr(szPos, '\n');
        }
        if (!szEnd) {
            return ERR_HTTP_BAD_FORMAT;
        }

        IF_PROCESS_STR_FIELD(SZ_HH_SERVER, m_szServer, RP_SERVER)
        else IF_PROCESS_STR_FIELD(SZ_HH_CONNECTION, m_szConnection, RP_CONNECTION)
        else IF_PROCESS_STR_FIELD(SZ_HH_DATE, m_szDate, RP_DATE)
        else IF_PROCESS_STR_FIELD(SZ_HH_LOCATION, m_szLocation, RP_LOCATION)
        else IF_PROCESS_STR_FIELD(SZ_HH_CONTENT_TYPE, m_szContentType, RP_CONTENT_TYPE)
        else IF_PROCESS_STR_FIELD(SZ_HH_LAST_MODIFIED, m_szLastModified, RP_LAST_MODIFIED)
        else IF_PROCESS_STR_FIELD(SZ_HH_ETAG, m_szEtag, RP_ETAG)
        else IF_PROCESS_STR_FIELD(SZ_HH_CACHE_CONTROL, m_szCacheControl, RP_CACHE_CONTROL)
        else IF_PROCESS_STR_FIELD(SZ_HH_CONTENT_DISP_FILENAME, m_szContentFileName, RP_CONTENT_DISP_FILE)
        else if (strncasecmp(szPos, SZ_HH_SET_COOKIE, CONST_STRLEN(SZ_HH_SET_COOKIE)) == 0)
        {
            szPos += CONST_STRLEN(SZ_HH_SET_COOKIE);
            m_strCookie = "";
            m_strCookie.append(szPos, szEnd);
            m_dwAvailProp |= RP_SET_COOKIE;
        } else if (strncasecmp(szPos, SZ_HH_TRANSF_ENCODING, CONST_STRLEN(SZ_HH_TRANSF_ENCODING)) == 0) {
            szPos += CONST_STRLEN(SZ_HH_TRANSF_ENCODING);
            while (*szPos == ' ') {
                szPos++;
            }
            if (strncasecmp(szPos, SZ_TRANSF_ENC_CHUNKED, CONST_STRLEN(SZ_TRANSF_ENC_CHUNKED)) == 0) {
                m_bTransfEncChunked = true;
            } else {
                ERR_LOG0("!!! Unsupported transfer encoding.");
            }
        } else if (strncasecmp(szPos, SZ_HH_CONTENT_LENGTH, CONST_STRLEN(SZ_HH_CONTENT_LENGTH)) == 0) {
            szPos += CONST_STRLEN(SZ_HH_CONTENT_LENGTH);
            m_dwContentLen = (uint32_t)atoi(szPos);
            m_dwAvailProp |= RP_CONTENT_LENGTH;
        } else {
            // char        szName[128];
            // strncpy_safe(szName, CountOf(szName), szPos, int(szEnd - szPos));
            // ERR_LOG1("unknown http return property: %s", szName);
        }
        szPos = szEnd + 2;
    }

    return ERR_OK;
}

void CHttpReturnProtocol::setAvailProp(uint32_t dwProp) {
    m_dwAvailProp = dwProp;
}

int CHttpReturnProtocol::getContentLength(long &nLength) {
    return ERR_OK;
}

void CHttpReturnProtocol::setContentLength(long nLength) {
    m_dwContentLen = (uint32_t)nLength;
    m_dwAvailProp |= RP_CONTENT_LENGTH;
}

void CHttpReturnProtocol::setRange(uint32_t dwStart, uint32_t dwEnd) {
}

void CHttpReturnProtocol::setServer(const char * szServer) {
    strcpy_safe(m_szServer, CountOf(m_szServer), szServer);
}

void CHttpReturnProtocol::setConnection(const char * szConnection) {
    strcpy_safe(m_szConnection, CountOf(m_szConnection), szConnection);
}

const char * CHttpReturnProtocol::getPropValue(RET_PROPERTY retProperty) {
    switch (retProperty) {
    case RP_SERVER:
        return m_szServer;
        break;
    case RP_CONNECTION:
        return m_szConnection;
        break;
    case RP_DATE:
        return m_szDate;
        break;
        //    case RP_CONTENT_LENGTH:
        //        m_dwContentLen = atoi(szValue);
        //        break;
    case RP_LOCATION:
        return m_szLocation;
        break;
    case RP_CONTENT_TYPE:
        return m_szContentType;
        break;
        //    case RP_CONTENT_RANGE:
        //        return m_szUserAgent;
        //        break;
    case RP_LAST_MODIFIED:
        return m_szLastModified;
        break;
    case RP_ETAG:
        return m_szEtag;
        break;
    case RP_SET_COOKIE:
        return m_strCookie.c_str();
        break;
    case RP_CACHE_CONTROL:
        return m_szCacheControl;
        break;
        //    case RP_CONTENT_DISPOSITION:
        //        return m_szLastModified;
        //        break;
    default:
        DBG_LOG1("CHttpRequestProtocol::getPropValue() does not support set the property: %d", retProperty);
        break;
    }

    return nullptr;
}

void CHttpReturnProtocol::setPropValue(RET_PROPERTY retProperty, const char * szValue) {
}

const char * CHttpReturnProtocol::getResult(int &nCode) {
    nCode = m_dwResultCode;
    return m_szResult;
}

int CHttpReturnProtocol::resultCodeToErrorCode(int nResultCode) {
    if (nResultCode >= 200 && nResultCode < 300) {
        return ERR_OK;
    } else if (nResultCode == 400) {
        return ERR_HTTP_400;
    } else if (nResultCode == 401) {
        return ERR_HTTP_401;
    } else if (nResultCode == 403) {
        return ERR_HTTP_403;
    } else if (nResultCode == 404) {
        return ERR_HTTP_404;
    } else if (nResultCode == 407) {
        return ERR_HTTP_407;
    } else if (nResultCode == 302) {
        return ERR_HTTP_302;
    } else if (nResultCode == 500) {
        return ERR_HTTP_500;
    } else {
        return ERR_HTTP_CODE_ERROR;
    }
}
