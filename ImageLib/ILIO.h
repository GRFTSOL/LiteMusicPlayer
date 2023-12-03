#ifndef ImageLib_ILIO_h
#define ImageLib_ILIO_h

#pragma once

#include "../Utils/Utils.h"


class IILIO {
public:
    IILIO();
    virtual ~IILIO();

    virtual size_t read(void *buf, size_t nSize) = 0;
    virtual size_t write(const void *buf, size_t nSize) { return nSize; }
    // nOrigin: SEEK_SET, SEEK_CUR, SEEK_END
    virtual bool seek(int nOffset, int nOrigin) = 0;
    virtual bool getSize(size_t &nSize) = 0;
    virtual size_t getPos() = 0;

    virtual bool isEOF() = 0;

    virtual void close() = 0;

};

class CFileILIO : public IILIO {
public:
    CFileILIO();
    ~CFileILIO();

    bool open(cstr_t szFile);

public:
    virtual size_t read(void *buf, size_t nSize);
    virtual bool seek(int nOffset, int nOrigin);
    virtual bool getSize(size_t &nSize);
    virtual size_t getPos();

    virtual bool isEOF();

    virtual void close();

protected:
    FILE                        *m_fp;

};

class CBuffILIO : public IILIO {
public:
    CBuffILIO();
    ~CBuffILIO();

    bool open(const void *buff, size_t nLength);

public:
    virtual size_t read(void *buf, size_t nSize);
    virtual bool seek(int nOffset, int nOrigin);
    virtual bool getSize(size_t &nSize);
    virtual size_t getPos();

    virtual bool isEOF();

    virtual void close();

protected:
    uint8_t                     *m_buf;
    size_t                      m_nLength;
    size_t                      m_nPos;

};

#endif // !defined(ImageLib_ILIO_h)
