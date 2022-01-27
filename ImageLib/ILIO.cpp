// ILIO.cpp: implementation of the CILIO class.
//
//////////////////////////////////////////////////////////////////////

#include "ILIO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IILIO::IILIO()
{

}

IILIO::~IILIO()
{

}

//////////////////////////////////////////////////////////////////////////
CFileILIO::CFileILIO()
{
    m_fp = nullptr;
}

CFileILIO::~CFileILIO()
{
    close();
}

bool CFileILIO::open(cstr_t szFile)
{
    close();

    m_fp = fopen(szFile, "rb");
    if (!m_fp)
        return false;

    return true;
}

size_t CFileILIO::read(void *buf, size_t nSize)
{
    return fread(buf, 1, nSize, m_fp);
}

bool CFileILIO::seek(int nOffset, int nOrigin)
{
    return fseek(m_fp, nOffset, nOrigin) == 0;
}

bool CFileILIO::getSize(size_t &nSize)
{
    size_t    pos = ftell(m_fp);
    fseek(m_fp, 0, SEEK_END);
    nSize = ftell(m_fp);
    fseek(m_fp, pos, SEEK_SET);
    return true;
}

size_t CFileILIO::getPos()
{
    return ftell(m_fp);
}

bool CFileILIO::isEOF()
{
    return feof(m_fp) != 0;
}

void CFileILIO::close()
{
    if (m_fp)
    {
        fclose(m_fp);
        m_fp = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////

CBuffILIO::CBuffILIO()
{
    m_buf = nullptr;
}

CBuffILIO::~CBuffILIO()
{
}

bool CBuffILIO::open(const void *buff, size_t nLength)
{
    m_buf = (uint8_t*)buff;
    m_nLength = nLength;
    m_nPos = 0;

    return true;
}

size_t CBuffILIO::read(void *buf, size_t nSize)
{
    if (m_nPos >= m_nLength)
        return 0;

    if (nSize + m_nPos > m_nLength)
        nSize = m_nLength - m_nPos;

    memcpy(buf, m_buf + m_nPos, nSize);
    m_nPos += nSize;

    return nSize;
}

bool CBuffILIO::seek(int nOffset, int nOrigin)
{
    if (nOrigin == SEEK_SET)
        m_nPos = nOffset;
    else if (nOrigin == SEEK_CUR)
        m_nPos += nOffset;
    else if (nOrigin == SEEK_END)
        m_nPos = m_nLength + nOffset;

    if (m_nPos > m_nLength)
        m_nPos = m_nLength;

    return true;
}

bool CBuffILIO::getSize(size_t &nSize)
{
    nSize = m_nLength;
    return true;
}

size_t CBuffILIO::getPos()
{
    return m_nPos;
}

bool CBuffILIO::isEOF()
{
    return m_nPos >= m_nLength;
}

void CBuffILIO::close()
{
    m_buf = nullptr;
}
