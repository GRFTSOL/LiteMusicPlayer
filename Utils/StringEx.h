#pragma once

#include "UtilsTypes.h"
#include "StrPrintf.h"


inline bool strIsSame(cstr_t s1, cstr_t s2) { return strcmp(s1, s2) == 0; }
inline bool strIsISame(cstr_t s1, cstr_t s2) { return strcasecmp(s1, s2) == 0; }

bool startsWith(cstr_t szText, cstr_t szBeginWith);
bool iStartsWith(cstr_t szText, cstr_t szBeginWith);

bool endsWith(cstr_t szText, cstr_t szEndWith);
bool iEndsWith(cstr_t szText, cstr_t szEndWith);

char *stristr(cstr_t source, cstr_t find);

const char * strignore(const char *str, const char *strIgnore);

size_t strlen_safe(const char * str, size_t maxLength);
char *strcpy_safe(char *strDestination, size_t nLenMax, const char *strSource);
size_t strncpy_safe(char *strDestination, size_t nLenMax, const char *strSource, size_t nToCopy);

size_t wcslen(const WCHAR *str);

char *strrep(char * str, char chSrc, char chDest);
void strrep(string &str, char chSrc, char chDest);
void strrep(string &str, const char *szSrc, const char *szDest);

void trimStr(string &str, cstr_t szChars);
void trimStr(string &str, char ch = ' ');
void trimStr(char * pszString, char ch = ' ');
void trimStr(VecStrings &vStrs, char ch = ' ');
void trimStrRight(char * pszString, cstr_t pszChars = " ");

void strSplit(const char *str, char sep, VecStrings &vStrs);
bool strSplit(const char *str, char sep, string &leftOut, string &rightOut);
bool strSplit(const char *str, const char *sep, string &leftOut, string &rightOut);

size_t itoa(int64_t value, char *buffer);
string itos(int64_t value);

string toUpper(cstr_t szString);
string toLower(cstr_t szString);

inline int toLower(int ch) {
    if ('A' <= ch && ch <= 'Z')
        return (ch + 'a' - 'A');
    else
        return ch;
}

inline int toUpper(int ch) {
    if ('a' <= ch && ch <= 'z')
        return (ch + 'A' - 'a');
    else
        return ch;
}

inline bool isDigit(int ch) {
    return ('0' <= ch) && (ch <= '9');
}

inline bool isAlpha(int ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

inline bool isAlphaDigit(int c) {
    return isAlpha(c) || isDigit(c);
}

bool isNumeric(cstr_t szString);

inline bool isWhiteSpace(int c) {
    return c == ' ' || c == '\t';
}

bool isHexChar(int ch);
inline char hexToChar(int n) { return n <= 9 ? n + '0' : 'A' + n - 10; }
string hexToStr(const uint8_t *data, size_t len);

int hexToInt(int chHex);
uint32_t hexToInt(cstr_t str);

bool saveDataAsFile(cstr_t szFile, const void *lpData, size_t nLen);

void multiStrToVStr(cstr_t szText, vector<string> &vStr);

template<class _iterator>
string strJoin(_iterator first, _iterator end, cstr_t szFormat, cstr_t szSeperator)
{
    CStrPrintf    strFormat;
    string        str;
    for (; first != end; ++first)
    {
        if (str.size())
            str += szSeperator;
        strFormat.printf(szFormat, *first);
        str += strFormat.c_str();
    }

    return str;
}

template<class _iterator>
string strJoin(_iterator first, _iterator end, cstr_t szSeperator)
{
    string        str;
    for (; first != end; ++first)
    {
        if (str.size())
            str += szSeperator;
        str += *first;
    }

    return str;
}

template<class _TCHARPTR, class _int_t>
_TCHARPTR readInt_t(_TCHARPTR str, _int_t &value)
{
    bool        bNegative;

    value = 0;
    if (*str == '-')
    {
        bNegative = true;
        str++;
    }
    else
        bNegative = false;
    while (isDigit(*str))
    {
        value *= 10;
        value += *str - '0';
        str++;
    }
    if (bNegative)
        value = -value;

    return str;
}

// szValue format: %d,%d,%d,%d
template<class TCHARDEF, class _int_t>
bool scan4IntX(TCHARDEF szValue, _int_t &n1, _int_t &n2, _int_t &n3, _int_t &n4)
{
    szValue = readInt_t(szValue, n1);
    if (*szValue != ',')
        return false;
    szValue++;
    while (*szValue == ' ')
        szValue++;

    szValue = readInt_t(szValue, n2);
    if (*szValue != ',')
        return false;
    szValue++;
    while (*szValue == ' ')
        szValue++;

    szValue = readInt_t(szValue, n3);
    if (*szValue != ',')
        return false;
    szValue++;
    while (*szValue == ' ')
        szValue++;

    szValue = readInt_t(szValue, n4);

    return true;
}

// szValue format: %d,%d
template<class TCHARDEF, class _int_t>
bool scan2IntX(TCHARDEF szValue, _int_t &n1, _int_t &n2)
{
    szValue = readInt_t(szValue, n1);
    if (*szValue != ',')
        return false;
    szValue++;
    while (*szValue == ' ')
        szValue++;

    readInt_t(szValue, n2);

    return true;
}

int stringToInt(cstr_t str, int nDefault = 0);
COLORREF stringToColor(cstr_t szColor, COLORREF nDefault = 0);

string stringFromInt(int n);
string stringFromColor(COLORREF clr);

void stringFromColor(char szStr[], COLORREF clr);

class SetStrLessICmp
{
public:
    bool operator()(const string &str1, const string &str2) const
    {
        return strcasecmp(str1.c_str(), str2.c_str()) < 0;
    }
};

typedef set<string, SetStrLessICmp>         SetICaseStr;
