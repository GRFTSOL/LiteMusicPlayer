#include "safestr.h"


size_t strlen_safe(const char * str, size_t maxLength) {
    size_t n = 0;
    while (n < maxLength && str[n]) {
        n++;
    }

    return n;
}

size_t wcslen_safe(const WCHAR * str, size_t maxLength) {
    size_t n = 0;
    while (n < maxLength && str[n]) {
        n++;
    }

    return n;
}

// 安全的字符串拷贝，如果超出长度范围，则进行截断
char *strcpy_safe(char *strDestination, size_t nLenMax, const char *strSource) {
    char *cp = strDestination;
    char *cpEnd = strDestination + nLenMax - 1;

    for (; cp < cpEnd && *strSource; cp++, strSource++) {
        *cp = *strSource;
    }

    *cp = '\0';

    return(strDestination);
}

// 安全的字符串拷贝，如果超出长度范围，则进行截断
size_t    strncpy_safe(char *strDestination, size_t nLenMax, const char *strSource, size_t nToCopy) {
    if (nToCopy >= nLenMax) {
        nToCopy = nLenMax - 1;
    }

    size_t org = nToCopy;

    while (nToCopy && (*strDestination++ = *strSource++))    /* copy string */ {
        nToCopy--;
    }

    *strDestination = '\0';

    return org - nToCopy;
}

// 安全的字符串拷贝，如果超出长度范围，则进行截断
// 并且将结尾设置为以'\0'结束
size_t    strncpysz_safe(char *strDestination, size_t nLenMax, const char *strSource, size_t nToCopy) {
    if (nToCopy >= nLenMax) {
        nToCopy = nLenMax - 1;
    }

    size_t org = nToCopy;

    while (nToCopy && (*strDestination++ = *strSource++))    /* copy string */ {
        nToCopy--;
    }

    *strDestination = '\0';

    return org - nToCopy;
}

// 安全的字符串拷贝，如果超出长度范围，则进行截断
char *strcat_safe(char * dst, size_t size, const char * src) {
    char * cp = dst;
    char * endcp = dst + size - 1;

    while( *cp ) {
        cp++; /* find end of dst */
    }

    while((cp < endcp) && (*cp++ = *src++)) ;       /* copy src to end of dst */

    *cp = '\0';

    return( dst ); /* return dst */
}

//////////////////////////////////////////////////////////////////////////
// UNICODE version

// 安全的字符串拷贝，如果超出长度范围，则进行截断
size_t    wcsncpy_safe(WCHAR *strDestination, size_t nLenMax, const WCHAR *strSource, size_t nToCopy) {
    if (nToCopy >= nLenMax) {
        nToCopy = nLenMax - 1;
    }

    size_t org = nToCopy;

    while (nToCopy && (*strDestination++ = *strSource++))    /* copy string */ {
        nToCopy--;
    }

    *strDestination = '\0';

    return org - nToCopy;
}

// 安全的字符串拷贝，如果超出长度范围，则进行截断
// 并且将结尾设置为以'\0'结束
size_t    wcsncpysz_safe(WCHAR *strDestination, size_t nLenMax, const WCHAR *strSource, size_t nToCopy) {
    if (nToCopy >= nLenMax) {
        nToCopy = nLenMax - 1;
    }

    size_t org = nToCopy;

    while (nToCopy && (*strDestination++ = *strSource++))    /* copy string */ {
        nToCopy--;
    }

    *strDestination = '\0';

    return org - nToCopy;
}

// 安全的字符串拷贝，如果超出长度范围，则进行截断
WCHAR *wcscpy_safe(WCHAR *strDestination, size_t nLenMax, const WCHAR *strSource) {
    WCHAR *cp = strDestination;
    WCHAR *cpEnd = strDestination + nLenMax - 1;

    for (; cp < cpEnd && *strSource; cp++, strSource++) {
        *cp = *strSource;
    }

    *cp = '\0';

    return(strDestination);
}

WCHAR *wcscat_safe(WCHAR * dst, size_t size, const WCHAR * src) {
    WCHAR * cp = dst;
    WCHAR * endcp = dst + size - 1;

    while( *cp ) {
        cp++; /* find end of dst */
    }

    while((cp < endcp) && (*cp++ = *src++)) ;       /* copy src to end of dst */

    *cp = '\0';

    return( dst ); /* return dst */
}

/*
void separateWords(cstr_t szText, vector<string> &vWords, char chSeparator)
{
    string        str;
    cstr_t            szBeg;

    while (*szText == chSeparator)
        szText++;

    szBeg = szText;

    while (*szText != '\0')
    {
        szText = szBeg;
        while (*szText != chSeparator && *szText != '\0')
            szText++;

        str.NCopy(szBeg, szText);
        vWords.push_back(str.c_str());

        szBeg = szText + 1;
    }
}

void separateWords(cstr_t szText, vector<string> &vWords, cstr_t szSeparator)
{
    string        str;
    cstr_t            szBeg, szLastBeg;

    szBeg = szText;
    szLastBeg = szBeg;

    szBeg = strstr(szBeg, szSeparator);

    while (szBeg)
    {
        // copy szLastBeg ---> szBeg
        str.NCopy(szLastBeg, szBeg);
        vWords.push_back(str.c_str());

        szLastBeg = szBeg + strlen(szSeparator);
        szBeg = strstr(szLastBeg, szSeparator);
    }

    vWords.push_back(szLastBeg);
}
*/
