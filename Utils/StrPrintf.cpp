// StrPrintf.cpp: implementation of the CStrPrintf class.
//
//////////////////////////////////////////////////////////////////////

#include "UtilsTypes.h"
#include "StrPrintf.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStrPrintf::CStrPrintf(cstr_t szFormat, ...)
{
    m_nSize = 0;
    m_nCapacity = 256;
    m_pszBuffer = nullptr;

    va_list        args;

    va_start(args, szFormat);
    vprintf(szFormat, args);
    va_end(args);
}

void CStrPrintf::printf(cstr_t szFormat, ...)
{
    va_list        args;

    va_start(args, szFormat);
    vprintf(szFormat, args);
    va_end(args);
}

void CStrPrintf::vprintf(cstr_t szFormat, va_list args)
{
    m_nSize = 0;

    if (!m_pszBuffer)
    {
        m_nCapacity = 256;
        m_pszBuffer = new char[m_nCapacity];
    }

    while (1)
    {
        m_nSize = vsnprintf(m_pszBuffer, m_nCapacity - 1, szFormat, args);

        // was there an error? was the expanded string too long?
        if ((long)m_nSize < 0)
        {
            if (m_nCapacity >= 100 * 1024)
                break;

            delete[] m_pszBuffer;
            m_pszBuffer = nullptr;
            m_nCapacity *= 2;
            m_pszBuffer = new char[m_nCapacity];
        }
        else
            break;
    }

    if (m_nSize == m_nCapacity - 1)
        m_pszBuffer[m_nSize] = '\0';

    if ((long)m_nSize < 0)
    {
        strcpy(m_pszBuffer, "Insufficient buffer space.");
        m_nSize = strlen(m_pszBuffer);
    }
}
