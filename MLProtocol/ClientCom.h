#pragma once

#include "../Utils/Utils.h"
#include "sockadapt.h"


//////////////////////////////////////////////////////////////////////////
//

class CClientCom {
public:
    CClientCom();
    virtual ~CClientCom();

    bool isValid();

    int connect(cstr_t szServer, int nPort);

    int recieve(char * buf, int nLen, int flags = 0)
        { return (int)recv(m_s, buf, nLen, flags); }

    int send(const char *buf, int nLen, int flags = 0)
        { return (int)::send(m_s, buf, nLen, flags); }

    int close() { ::closesocket(m_s); m_s = 0; return 0; }

    SOCKET getSocket() { return m_s; }

protected:
    SOCKET                      m_s;

};

//////////////////////////////////////////////////////////////////////////
//

class CNetFile {
public:
    CNetFile();
    ~CNetFile();

    int attach(SOCKET s) { m_s = s; return 0; }
    SOCKET getHandle() { return m_s; }

    void close() { ::closesocket(m_s); }

    int setTimeOut(int nTimeOut);

    int read(char *szBuffer, int nBufferLen, int *pnRead);
    int read(string &buf, int nMaxToRead, int *pnRead);
    int readExactly(string &buf, int nToRead);
    int readTillDataStreamClosed(string &buf);
    int write(const char *szBuffer, int nLen, int *pnWrite);

protected:
    SOCKET                      m_s;

};
