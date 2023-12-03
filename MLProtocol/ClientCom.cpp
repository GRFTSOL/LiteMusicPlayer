#include <netdb.h>
#include "ClientCom.h"


#ifdef _WIN32
#define WSAError()          WSAGetLastError()
#else
#define WSAError()          (errno + ERR_C_ERRNO_BASE)
#define SOCKET_ERROR        (-1)
#endif

//////////////////////////////////////////////////////////////////////////
//
CClientCom::CClientCom() {
    m_s = 0;
}

CClientCom::~CClientCom() {
    if (m_s) {
        ::closesocket(m_s);
    }
}

/*
#define RECV_BUFF_LEN        1024

uint32_t WINAPI  CClientCom::ThreadRecv(void *lpParam)
{
    CClientCom *pThis;
    pThis = (CClientCom *)lpParam;
    assert(pThis);

    char*    szBuffer;
    int        nLen;
    int        nTotal;
    CString &buff = pThis->m_bufRecv;

    do
    {
        nTotal = buff.getLength();
        szBuffer = buff.getBuffer(RECV_BUFF_LEN + nTotal);

        nLen = pThis->recieve(szBuffer + nTotal, RECV_BUFF_LEN - 1, 0);
        if (nLen > 0)
        {
            nTotal += nLen;
            szBuffer[nTotal] = '\0';
            buff.ReleaseBuffer(nTotal);

            if (pThis->OnRecieve(&buff))
                buff.empty();
        }
    }
    while (nLen > 0);

    int        nLastErr = WSAGetLastError();

//    DBG_LOG1("CClientCom::ThreadRecv() Exited. LastError: %d", nLastErr);
    // LOG0(LOG_LVL_INFO, "CClientCom::ThreadRecv() Exited. Not release merory and resource!!!!!!!!!");
    return ERR_OK;
}*/


int CClientCom::connect(cstr_t szServer, int nPort) {
    hostent *hostEntry;
    int nRet;
    cstr_t szServerAscii = szServer;

    hostEntry = gethostbyname(szServerAscii);

    if(!hostEntry) {
        return ERR_NET_HOST_NOT_FOUND;
    }

    m_s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(m_s == SOCKET_ERROR) {
        nRet = WSAError();
        return nRet;
    }

    // Fill in the sockaddr_in struct
    sockaddr_in serverInfo;

    memset(&serverInfo, 0, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr = *((in_addr *)*hostEntry->h_addr_list);
    serverInfo.sin_port = htons(nPort);

    nRet = ::connect(m_s, (sockaddr *)&serverInfo, sizeof(serverInfo));
    if(nRet==SOCKET_ERROR) {
        nRet = WSAError();
        ::closesocket(m_s);
        m_s = 0;
        return nRet;
    }

    linger lingSock;
    lingSock.l_onoff = 1;
    lingSock.l_linger = 20;
    setsockopt(m_s, SOL_SOCKET, SO_LINGER, (const char *)&lingSock, sizeof(lingSock));

    return ERR_OK;
}

bool CClientCom::isValid() {
    return m_s != 0;
}

//////////////////////////////////////////////////////////////////////////
//
CNetFile::CNetFile() {
    m_s = 0;
}

CNetFile::~CNetFile() {
}

int CNetFile::setTimeOut(int nTimeOut) {
#if !defined(_IPHONE)
    // 设置接收延时和发送延时
    if (setsockopt(m_s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&nTimeOut, sizeof(nTimeOut)) == SOCKET_ERROR) {
        // Task.writeLog(DOWN_LOG_INFO_ERROR, "Error occured! set Socket send TimeOut Error! Scoket Error=%d", WSAGetLastError());
        // errResult = SockErrorToMLError(WSAGetLastError());
        return WSAError();
    }
    if (setsockopt(m_s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&nTimeOut, sizeof(nTimeOut)) == SOCKET_ERROR) {
        // Task.writeLog(DOWN_LOG_INFO_ERROR, "Error occured! set Socket send TimeOut Error! Scoket Error=%d", WSAGetLastError());
        return WSAError();
    }
#endif

    return ERR_OK;
}

int CNetFile::read(char *szBuffer, int nBufferLen, int *readOut) {
    *readOut = (int)recv(m_s, szBuffer, nBufferLen, 0);
    if (*readOut == SOCKET_ERROR) {
        *readOut = 0;
        return WSAError();
    }
    return ERR_OK;
}

int CNetFile::readExactly(string &buf, int nToRead) {
    if (nToRead <= 0) {
        return ERR_OK;
    }

    int ret = ERR_OK;
    size_t size = buf.size();

    buf.resize(buf.size() + nToRead);

    while (nToRead > 0) {
        size_t got = recv(m_s, (char *)buf.data() + size, nToRead, 0);
        if (got == SOCKET_ERROR) {
            ret = WSAError();
            break;
        } else if (got == 0) {
            ret = ERR_NOT_RECIVED_ALL;
            break;
        }

        size += got;
        nToRead -= got;
    }

    buf.resize(size);

    return ERR_OK;
}

int CNetFile::readTillDataStreamClosed(string &buf) {
    int ret = ERR_OK;
    size_t size = buf.size();

    while (true) {
        int nToRead = 4096;
        if (buf.size() - size < (size_t)nToRead) {
            buf.resize(buf.size() + nToRead * 2);
        }

        size_t got = recv(m_s, (char *)buf.data() + size, nToRead, 0);
        if (got == SOCKET_ERROR) {
            ret = WSAError();
            break;
        } else if (got == 0) {
            break;
        }

        size += got;
    }

    buf.resize(size);

    return ret;
}

int CNetFile::read(string &buf, int nMaxToRead, int *readOut) {
    int ret = ERR_OK;
    size_t size = buf.size();

    if (nMaxToRead == -1) {
        *readOut = 0;

        const int RESERVE_SPACE = 4096;
        while (1) {
            if (buf.size() - size < (size_t)RESERVE_SPACE) {
                buf.resize(buf.size() + RESERVE_SPACE * 2);
            }

            size_t n = recv(m_s, (char *)buf.data() + size, RESERVE_SPACE, 0);
            if (n == SOCKET_ERROR) {
                ret = WSAError();
                break;
            } else if (n == 0) {
                break;
            }
            *readOut += n;
            size += n;
        }
    } else {
        buf.resize(size + nMaxToRead);

        *readOut = (int)recv(m_s, (char *)buf.data() + size, nMaxToRead, 0);
        if (*readOut == SOCKET_ERROR) {
            *readOut = 0;
            ret = WSAError();
        } else {
            size += *readOut;
        }
    }

    buf.resize(size);

    return ERR_OK;
}

int CNetFile::write(const char *szBuffer, int nLen, int *pnWrite) {
    if (SOCKET_ERROR != send(m_s, szBuffer, nLen, 0)) {
        return ERR_OK;
    } else {
        return WSAError();
    }
}
