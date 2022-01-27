// StrPrintf.h: interface for the CStrPrintf class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRPRINTF_H__71471822_3722_4AF3_9FF3_3E99A3EE8635__INCLUDED_)
#define AFX_STRPRINTF_H__71471822_3722_4AF3_9FF3_3E99A3EE8635__INCLUDED_

#pragma once


class CStrPrintf
{
public:
    CStrPrintf(cstr_t szFormat, ...);

    CStrPrintf()
    {
        m_nSize = 0;
        m_nCapacity = 0;
        m_pszBuffer = nullptr;
    }

    ~CStrPrintf()
    {
        if (m_pszBuffer)
            delete[] m_pszBuffer;
    }

    void printf(cstr_t szFormat, ...);

    void vprintf(cstr_t szFormat, va_list args);

    cstr_t c_str() { if (m_pszBuffer) return m_pszBuffer; else return ""; }
    size_t size() { return m_nSize; }

protected:
    char            *m_pszBuffer;
    size_t        m_nSize;
    size_t        m_nCapacity;

};

#endif // !defined(AFX_STRPRINTF_H__71471822_3722_4AF3_9FF3_3E99A3EE8635__INCLUDED_)
