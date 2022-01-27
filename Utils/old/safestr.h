// safestr.h
#ifndef _SAFE_STRING_H_
#define _SAFE_STRING_H_

#include <stdarg.h>
#include "xstr.h"


size_t strlen_safe(const char * str, size_t maxLength);
size_t wcslen_safe(const WCHAR * str, size_t maxLength);

// 安全的字符串拷贝，如果超出长度范围，则进行截断
char *strcpy_safe(char *strDestination, size_t nLenMax, const char *strSource);
size_t    strncpy_safe(char *strDestination, size_t nLenMax, const char *strSource, size_t nToCopy);
size_t    strncpysz_safe(char *strDestination, size_t nLenMax, const char *strSource, size_t nToCopy);
char *strcat_safe(char * dst, size_t size, const char * src);

WCHAR *wcscpy_safe(WCHAR *strDestination, size_t nLenMax, const WCHAR *strSource);
size_t    wcsncpy_safe(WCHAR *strDestination, size_t nLenMax, const WCHAR *strSource, size_t nToCopy);
size_t    wcsncpysz_safe(WCHAR *strDestination, size_t nLenMax, const WCHAR *strSource, size_t nToCopy);
WCHAR * __cdecl wcscat_safe(WCHAR * dst, size_t size, const WCHAR * src);

#ifdef _UNICODE
#define strlen_safe        wcslen_safe
#define strcpy_safe        wcscpy_safe
#define strncpy_safe        wcsncpy_safe
#define strncpysz_safe        wcsncpysz_safe
#define strcat_safe        wcscat_safe
#else
#define strlen_safe        strlen_safe
#define strcpy_safe        strcpy_safe
#define strncpy_safe        strncpy_safe
#define strncpysz_safe        strncpysz_safe
#define strcat_safe        strcat_safe
#endif

#ifndef _WIN32
#define vsnprintf_s vsnprintf
#define _vsntprintf vsnprintf
#define _vstprintf_s vsnprintf
#define _stprintf_s sprintf_s
#define _stprintf_s sprintf_s
#define _itoa_s    _itot_s

inline int sprintf_s(char *string, size_t sizeInBytes, const char *format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    return vsnprintf(string, sizeInBytes, format, arglist);
}

inline char *_itoa(int value, char *string, size_t stringLen, int radix)
{
    return _itox_s(value, string, stringLen, radix);
}

inline char* itoa(int value, char* result, int base)
{
    return _itox_s(value, result, 16, base);
}

inline int _itot_s(int value, char *buffer, size_t sizeInCharacters, int radix)
{
    _itox_s(value, buffer, sizeInCharacters, radix);
    return 0;
}


#endif

#endif // _SAFE_STRING_H_
