#include "UtilsTypes.h"
#include "StringEx.h"


//
//int isCharWordBoundary(WCHAR c) {
//    return ((c >= 0x4E00 && c <= 0x9FFF)
//        || (c >= 0x3000 && c <= 0x303F) // Chinese punctuation
//        || (c >= 0x3400 && c <= 0x4DFF)
//        || (c >= 0x20000 && c <= 0x2A6DF)
//        || (c >= 0xF900 && c <= 0xFAFF)
//        || (c >= 0xF900 && c <= 0xFAFF)
//        || (c >= 0x2F800 && c <= 0x2FA1F)
//        || (!isAlphaDigit(c) && c != '\'' && c != '-' && c != '_'));
//}

bool startsWith(cstr_t szText, cstr_t szBeginWith)
{
    while (*szBeginWith == *szText && *szBeginWith && *szText)
    {
        szBeginWith++;
        szText++;
    }

    return *szBeginWith == '\0';
}


bool iStartsWith(cstr_t szText, cstr_t szBeginWith)
{
    while (toLower(*szBeginWith) == toLower(*szText) && *szBeginWith && *szText)
    {
        szBeginWith++;
        szText++;
    }

    return *szBeginWith == '\0';
}

bool endsWith(cstr_t szText, cstr_t szEndWith)
{
    size_t lenText = strlen(szText);
    size_t lenEndWith = strlen(szEndWith);

    if (lenText < lenEndWith)
        return false;

    return strcmp(szText + lenText - lenEndWith, szEndWith) == 0;
}

bool iEndsWith(cstr_t szText, cstr_t szEndWith)
{
    size_t lenText = strlen(szText);
    size_t lenEndWith = strlen(szEndWith);

    if (lenText < lenEndWith)
        return false;

    return strcasecmp(szText + lenText - lenEndWith, szEndWith) == 0;
}

char *stristr(cstr_t source, cstr_t find) {
    SizedString str(source);
    int pos = str.strIStr(SizedString(find));
    if (pos == -1) {
        return nullptr;
    }

    return (char *)source + pos;
}

size_t strlen_safe(const char * str, size_t maxLength)
{
	size_t n = 0;
	while (n < maxLength && str[n])
		n++;

	return n;
}

char *strcpy_safe(char *strDestination, size_t nLenMax, const char *strSource)
{
    char    *cp = strDestination;
    char    *cpEnd = strDestination + nLenMax - 1;

    for (; cp < cpEnd && *strSource; cp++, strSource++)
    {
        *cp = *strSource;
    }

    *cp = '\0';

    return(strDestination);
}

size_t strncpy_safe(char *strDestination, size_t nLenMax, const char *strSource, size_t nToCopy)
{
    if (nToCopy >= nLenMax)
        nToCopy = nLenMax - 1;

    size_t        org = nToCopy;

    while (nToCopy && (*strDestination++ = *strSource++))    /* copy string */
            nToCopy--;

    *strDestination = '\0';

    return org - nToCopy;
}

void strrep(string &str, const char *szSrc, const char *szDest)
{
    size_t        nPos;
    size_t        nLenSrc = strlen(szSrc);
    size_t        nLenDest = strlen(szDest);
    cstr_t    szPos;

    nPos = 0;
    while (nPos < str.size())
    {
        szPos = strstr(str.c_str() + nPos, szSrc);
        if (szPos)
        {
            nPos = size_t(szPos - str.c_str());
            str.erase(str.begin() + nPos, str.begin() + nPos + nLenSrc);
            str.insert(str.begin() + nPos, szDest, szDest + nLenDest);
            nPos += nLenDest;
        }
        else
            break;
    }
}

char * __cdecl strrep(char * str, char chSrc, char chDest)
{
    char *cp = (char *) str;
    while (*cp)
    {
        if (*cp == chSrc)
            *cp = chDest;
        cp++;
    }

    return str;
}

void strrep(string &str, char chSrc, char chDest)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == chSrc)
            str.replace(i, 1, 1, chDest);
    }
}

#ifndef WIN32
char *_strupr(char *str)
{
    char    *s = str;
    while (*s)
    {
        int        f = (unsigned char)(*(s));
        if ( (f >= 'a') && (f <= 'z') )
        {
            *s -= ('a' - 'A');
        }
        s++;
    }
    return str;
}
#endif

const char * strignore(const char *str, const char *strIgnore)
{
    const char *cp = (char *) str;
    while (*cp)
    {
        const char *cp2 = strIgnore;
        while (*cp2 && *cp2 != *cp)
            cp2++;
        if (*cp2 == '\0')
            return cp;
        cp++;
    }

    return cp;
}

void strdel(string &str, char ch)
{
    for (size_t i = 0; i < str.size(); )
    {
        if (str[i] == ch)
            str.erase(i, 1);
        else
            i++;
    }
}

void strdel(char *szStr, char ch)
{
    char    *szPos;
    char    *szPos2;
    szPos = szStr;
    szPos2 = szStr;
    while (*szPos)
    {
        if (*szPos != ch)
        {
            *szPos2 = *szPos;
            szPos2++;
        }
        szPos++;
    }
    *szPos2 = '\0';
}


char * __cdecl strchrtill (
        const char * string,
        int ch,
        int chTill
        )
{
        while (*string && *string != (char)ch && *string != (char)chTill)
                string++;

        if (*string == (char)chTill)
            return nullptr;

        if (*string == (char)ch)
                return((char *)string);
        return(nullptr);
}

void trimStr(string &str, cstr_t szChars)
{
    string::iterator    it;
    long        i;

    for (i = str.size() - 1; i >= 0; i--)
    {
        if (!strchr(szChars, str[i]))
            break;
    }
    if (i != str.size() - 1)
        str.erase(str.begin() + 1 + i, str.end());

    for (it = str.begin(); it != str.end(); it++)
    {
        if (!strchr(szChars, *it))
            break;
    }
    if (it != str.begin())
        str.erase(str.begin(), it);
}

void trimStr(string &str, char ch)
{
    string::iterator    it;
    long        i;

    for (i = str.size() - 1; i >= 0; i--)
    {
        if (str[i] != ch)
            break;
    }
    if (i != str.size() - 1)
        str.erase(str.begin() + 1 + i, str.end());

    for (it = str.begin(); it != str.end(); it++)
    {
        if (*it != ch)
            break;
    }
    if (it != str.begin())
        str.erase(str.begin(), it);
}

//
// 去掉字符串两端的空格
void trimStr(char * szString, char ch)
{
    char *    szBeg;
    szBeg = szString;

    while (*szBeg == ch)
        szBeg++;

    long nLen = strlen(szBeg);
    if (nLen > 0)
    {
        while (szBeg[nLen - 1] == ch)
            nLen--;
        szBeg[nLen] = '\0';
    }

    if (szBeg != szString)
        memmove(szString, szBeg, sizeof(char) * (nLen + 1));
}

void trimStrRight(char * pszStr, cstr_t pszChars/* = " "*/)
{
    if (nullptr == pszStr || nullptr == pszChars)
        return;

    char * p = pszStr + strlen(pszStr) - 1;
    while (p >= pszStr)
    {
        cstr_t pCh = pszChars;

        // Matching taget char set.
        while (*pCh)
        {
            if (*pCh == *p)
            {
                *p = 0;
                break;
            }
            ++pCh;
        }

        // Not matched?
        if (*p != 0)
            return;
        --p;
    }
}

void trimStr(VecStrings &vStrs, char ch) {
    for (auto &s : vStrs) {
        trimStr(s, ch);
    }
}

void strSplit(const char *str, char sep, VecStrings &vStrs) {
    makeSizedString(str).split(sep, vStrs);
}

bool strSplit(const char *str, char sep, string &leftOut, string &rightOut) {
    SizedString s(str), left, right;

    if (s.split(sep, left, right)) {
        leftOut.assign((const char *)left.data, left.len);
        rightOut.assign((const char *)right.data, right.len);
        return true;
    }

    return false;
}

bool strSplit(const char *str, const char *sep, string &leftOut, string &rightOut) {
    SizedString s(str), left, right;

    if (s.split(sep, left, right)) {
        leftOut.assign((const char *)left.data, left.len);
        rightOut.assign((const char *)right.data, right.len);
        return true;
    }

    return false;
}

size_t itoa(int64_t num, char *buffer) {
    bool isNegative = false;
    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    size_t i = 0;

    do {
        buffer[i++] = (num % 10) + '0';
        num = num / 10;
    } while (num != 0);

    // append '-'
    if (isNegative) {
        buffer[i++] = '-';
    }

    // Reverse buffer
    char *p = buffer, *end = buffer + i - 1;
    while (p < end) {
        char tmp = *p;
        *p = *end;
        *end = tmp;

        p++;
        end--;
    }

    return i;
}

string itos(int64_t value) {
    char buf[32];

    size_t n = itoa(value, buf);

    return string(buf, n);
}

string toUpper(cstr_t szString)
{
    string        str = szString;

    for (int i = 0; szString[i] != '\0'; i ++)
        if ('a' <= szString[i]  && szString[i] <= 'z')
            str[i] += 'A' - 'a';

    return str;
}

string toLower(cstr_t szString)
{
    string        str = szString;

    str = szString;
    for (int i = 0; szString[i] != '\0'; i ++)
        if ('A' <= szString[i]  && szString[i] <= 'Z')
            str[i] += 'a' - 'A';

    return str;
}

// Return true if:
// * Has at least one number.
// * No other chars.
bool isNumeric(cstr_t szString)
{
    cstr_t szTemp = szString;

    while (isDigit(*szTemp))
        szTemp++;
    
    if (*szTemp != '\0')
        return false;

    // empty String
    if (szString == szTemp)
        return false;

    return true;
}

bool isHexChar(int ch)
{
    return (isDigit(ch) || 
        (ch >= 'a' && ch <= 'f') ||
        (ch >= 'A' && ch <= 'F'));
}

int hexToInt(int chHex)
{
    if (isDigit(chHex))
    {
        return chHex - '0';
    }
    else if (chHex >= 'a' && chHex <= 'f')
        return (chHex - 'a' + 10);
    else if (chHex >= 'A' && chHex <= 'F')
        return (chHex - 'A' + 10);
    else
        return 0;
}

uint32_t hexToInt(cstr_t str)
{
    if (strncasecmp(str, "0x", 2) == 0)
        str += 2;

    size_t len = strlen(str);
    uint32_t    val = 0;

    for (size_t i = 0; i < len; i++)
    {
        if ('0' <= str[i] && str[i] <= '9')
            val = val * 16 + str[i] - '0';
        else if ('a' <= str[i] && str[i] <= 'f')
            val = val * 16 + str[i] - 'a' + 10;
        else if ('A' <= str[i] && str[i] <= 'F')
            val = val * 16 + str[i] - 'A' + 10;
    }

    return val;
}

string hexToStr(const uint8_t *data, size_t len) {
    string str;
    str.resize(len * 2);
    char *p = (char *)str.data();
    for (size_t i = 0; i < len; i++) {
        *p++ = hexToChar(data[i] / 16);
        *p++ = hexToChar(data[i] % 16);
    }

    return str;
}

bool saveDataAsFile(cstr_t szFile, const void *lpData, size_t nLen)
{
    FILE    *fp;

    fp = fopen(szFile, "wb");
    if (fp == nullptr)
        return false;

    if (fwrite(lpData, 1, nLen, fp) != nLen)
    {
        fclose(fp);
        return false;
    }

    fclose(fp);

    return true;
}

void multiStrToVStr(cstr_t szText, vector<string> &vStr)
{
    while (*szText)
    {
        vStr.push_back(szText);
        while (*szText != '\0')
            szText++;
        szText++;
    }
}

// "#FFFFFF"
template<class _TCHARPTR, class _int_t>
_TCHARPTR readColorValue_t(_TCHARPTR szColor, _int_t &nClr)
{
    nClr = 0;

    if (*szColor == '#')
        szColor++;

    int        n;

    for (int i = 0; i < 6 && *szColor; i++)
    {
        if (*szColor >= '0' && *szColor <= '9')
            n = *szColor - '0';
        else if (*szColor >= 'a' && *szColor <= 'f')
            n = 10 + *szColor - 'a';
        else if (*szColor >= 'A' && *szColor <= 'F')
            n = 10 + *szColor - 'A';
        else
            break;

        nClr = nClr * 16 + n;
        szColor++;
    }

    nClr = ((nClr & 0xFF) << 16) + (nClr & 0xFF00) + ((nClr & 0xFF0000) >> 16);

    return szColor;
}

int stringToInt(cstr_t str, int nDefault)
{
    int value = 0;
    cstr_t end = readInt_t(str, value);
    if (end == str)
        return nDefault;

    return value;
}

// "#FFFFFF"
COLORREF stringToColor(cstr_t szColor, COLORREF nDefault)
{
    if (isEmptyString(szColor))
        return nDefault;

    uint32_t        clr;
    cstr_t p = readColorValue_t(szColor, clr);
    if (*p != '\0')
        return nDefault;
    else
        return clr;
}

string stringFromInt(int value)
{
    return itos(value);
}

string stringFromColor(COLORREF clr)
{
    char    szColor[32];

    stringFromColor(szColor, clr);
    return szColor;
}

void stringFromColor(char szStr[], COLORREF clr)
{
    sprintf(szStr, "#%02X%02X%02X", clr & 0xFF, (clr >> 8) & 0xff, (clr >> 16) & 0xff);
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(StringEx)

class CTestCaseStringEx : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCaseStringEx);
    CPPUNIT_TEST(testUrlParse);
    CPPUNIT_TEST(testReadInt_t);
    CPPUNIT_TEST(testReadColorValue_t);
    CPPUNIT_TEST(testTrimStrRight);
    CPPUNIT_TEST(testStrSplit);
    CPPUNIT_TEST(testStrSplit2);
    CPPUNIT_TEST(testStrJoin);
    CPPUNIT_TEST(testStrReplace);
    CPPUNIT_TEST(testStrBeginWith);
    CPPUNIT_TEST(testStrEndWith);
    CPPUNIT_TEST(test_xcsncpy);
    CPPUNIT_TEST(testUriUnquote);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testUrlParse()
    {
        cstr_t szUrl;

        string scheme, domain, path;
        int port;

        szUrl = "scheme://domain:10/path?query_string#fragment_id";
        CPPUNIT_ASSERT(urlParse(szUrl, scheme, domain, port, path));
        CPPUNIT_ASSERT(scheme == "scheme");
        CPPUNIT_ASSERT(domain == "domain");
        CPPUNIT_ASSERT(path == "path?query_string#fragment_id");
        CPPUNIT_ASSERT(port == 10);

        szUrl = "scheme://domain/path?query_string#fragment_id";
        CPPUNIT_ASSERT(urlParse(szUrl, scheme, domain, port, path));
        CPPUNIT_ASSERT(scheme == "scheme");
        CPPUNIT_ASSERT(domain == "domain");
        CPPUNIT_ASSERT(path == "path?query_string#fragment_id");
        CPPUNIT_ASSERT(port == -1);

        szUrl = "scheme://domain";
        CPPUNIT_ASSERT(urlParse(szUrl, scheme, domain, port, path));
        CPPUNIT_ASSERT(scheme == "scheme");
        CPPUNIT_ASSERT(domain == "domain");
        CPPUNIT_ASSERT(path == "");
        CPPUNIT_ASSERT(port == -1);

        szUrl = "scheme://domain:12";
        CPPUNIT_ASSERT(urlParse(szUrl, scheme, domain, port, path));
        CPPUNIT_ASSERT(scheme == "scheme");
        CPPUNIT_ASSERT(domain == "domain");
        CPPUNIT_ASSERT(path == "");
        CPPUNIT_ASSERT(port == 12);
    }

    void testTrimStrRight()
    {
        char szStr[1024] = "abcdef1213   ";
        trimStrRight(szStr);
        CPPUNIT_ASSERT(strcmp(szStr, "abcdef1213") == 0);
        trimStrRight(szStr);
        CPPUNIT_ASSERT(strcmp(szStr, "abcdef1213") == 0);
        trimStrRight(szStr, "314");
        CPPUNIT_ASSERT(strcmp(szStr, "abcdef12") == 0);
        trimStrRight(szStr, "cd21f");
        CPPUNIT_ASSERT(strcmp(szStr, "abcde") == 0);
        trimStrRight(szStr, "cd21f");
        CPPUNIT_ASSERT(strcmp(szStr, "abcde") == 0);
        trimStrRight(szStr, "cde21f");
        CPPUNIT_ASSERT(strcmp(szStr, "ab") == 0);
        trimStrRight(szStr, " ab+1234fo");
        CPPUNIT_ASSERT(strcmp(szStr, "") == 0);
    }

    void testReadInt_t()
    {
        int            value;
        cstr_t        p;

        p = readInt_t("10", value);
        CPPUNIT_ASSERT(*p == '\0' && value == 10);

        p = readInt_t("-110", value);
        CPPUNIT_ASSERT(*p == '\0' && value == -110);

        p = readInt_t("99c", value);
        CPPUNIT_ASSERT(*p == 'c' && value == 99);

        p = readInt_t("", value);
        CPPUNIT_ASSERT(*p == '\0' && value == 0);

        p = readInt_t("a1bc", value);
        CPPUNIT_ASSERT(*p == 'a' && value == 0);
    }

    void testReadColorValue_t()
    {
        COLORREF    clr;
        cstr_t        p;

        p = readColorValue_t("#576C91", clr);
        CPPUNIT_ASSERT(*p == '\0' && clr == RGB(0x57, 0x6c, 0x91));

        p = readColorValue_t("#576C91a", clr);
        CPPUNIT_ASSERT(*p == 'a' && clr == RGB(0x57, 0x6c, 0x91));

        p = readColorValue_t("#76C91", clr);
        CPPUNIT_ASSERT(*p == '\0' && clr == RGB(0x7, 0x6c, 0x91));

        p = readColorValue_t("576C91", clr);
        CPPUNIT_ASSERT(*p == '\0' && clr == RGB(0x57, 0x6c, 0x91));

        p = readColorValue_t("576C910", clr);
        CPPUNIT_ASSERT(*p == '0' && clr == RGB(0x57, 0x6c, 0x91));
    }

    void testStrSplit()
    {
        VecStrings        vStr;
        SetICaseStr setStr;
        cstr_t        szInput = "a,ab,a10,,A,";
        cstr_t        strResult[] = { "a", "ab", "a10", "", "A", "" };

        strSplit(szInput, ',', vStr);
        strSplit(szInput, ',', setStr);

        CPPUNIT_ASSERT(vStr.size() == CountOf(strResult));

        for (int i = 0; i < vStr.size(); i++)
        {
            if (strcmp(vStr[i].c_str(), strResult[i]) != 0)
                CPPUNIT_FAIL_T(CStrPrintf("strSplit test, case: %d, %s, %s", i, vStr[i].c_str(), strResult[i]).c_str());
        }

        CPPUNIT_ASSERT(setStr.size() + 2 == CountOf(strResult));

        for (int i = 0; i < vStr.size(); i++)
        {
            if (setStr.find(strResult[i]) == setStr.end())
                CPPUNIT_FAIL_T(CStrPrintf("strSplit(set) test, case(Can't find): %d, %s", i, strResult[i]).c_str());
        }
    }

    void testStrSplit2()
    {
        string        strLeft, strRight;
        cstr_t        szInput = "name=valuexxx=x";

        strSplit(szInput, '=', strLeft, strRight);
        CPPUNIT_ASSERT(strLeft == "name");
        CPPUNIT_ASSERT(strRight == "valuexxx=x");

        szInput = "name - valuexxx=x";
        strSplit(szInput, " - ", strLeft, strRight);
        CPPUNIT_ASSERT(strLeft == "name");
        CPPUNIT_ASSERT(strRight == "valuexxx=x");
    }

    void testStrJoin()
    {
        string        str;

        int vInt[] = { 1, 2, 3, 4 };
        str = strJoin(vInt, vInt + CountOf(vInt), "%d", ",");
        CPPUNIT_ASSERT(str == "1,2,3,4");

        VecStrings        vStr;
        vStr.push_back("1");
        vStr.push_back("2");
        vStr.push_back("3");
        str = strJoin(vStr.begin(), vStr.end(), ",");
        CPPUNIT_ASSERT(str == "1,2,3");
    }

    void testStrReplace()
    {
        cstr_t        szSrc[] = { "abca", "baAcd", "a" };
        cstr_t        szDst[] = { "xbcx", "bxAcd", "x" };

        for (int i = 0; i < CountOf(szSrc); i++)
        {
            string        str = szSrc[i];
            strrep(str, 'a', 'x');
            CPPUNIT_ASSERT(strcmp(szDst[i], str.c_str()) == 0);
        }
    }

    void testStrBeginWith()
    {
        cstr_t        szSrc[] = { "abca", "baAcd", "aXDegae", "aXDegae" };
        cstr_t        szBeg[] = { "", "b", "aXD", "aXDegae" };
        cstr_t        szIBeg[] = { "", "B", "axd", "aXDegaE" };
        cstr_t        szBegFalse[] = { "ac", "bb", "x", "aXDegaeD" };

        for (int i = 0; i < CountOf(szSrc); i++)
        {
            CPPUNIT_ASSERT(startsWith(szSrc[i], szBeg[i]));
            CPPUNIT_ASSERT(iStartsWith(szSrc[i], szIBeg[i]));
            CPPUNIT_ASSERT(!startsWith(szSrc[i], szBegFalse[i]));
        }
    }

    void testStrEndWith()
    {
        cstr_t        szSrc[] = { "abca", "baAcd", "aXDegae", "aXDegae" };
        cstr_t        szEnd[] = { "", "d", "egae", "aXDegae" };
        cstr_t        szIEnd[] = { "", "D", "EGaE", "aXDegaE" };
        cstr_t        szEndFalse[] = { "ac", "bb", "x", "aXDegaeD" };

        for (int i = 0; i < CountOf(szSrc); i++)
        {
            CPPUNIT_ASSERT(endsWith(szSrc[i], szEnd[i]));
            CPPUNIT_ASSERT(iEndsWith(szSrc[i], szIEnd[i]));
            CPPUNIT_ASSERT(!endsWith(szSrc[i], szEndFalse[i]));
        }
    }

    void test_xcsncpy()
    {
        cstr_t        szSrc[] = { "abca", "baAcd", "aXDegae", "aXDegae" };
        char        szTemp[64];
        WCHAR        szTempW[64];

        for (int i = 0; i < CountOf(szSrc); i++)
        {
            _xcsncpy(szTemp, szSrc[i], 1);
            CPPUNIT_ASSERT(szTemp[0] == szSrc[i][0]);
            CPPUNIT_ASSERT(szTemp[1] == 0);

            _xcsncpy(szTemp, szSrc[i], strlen(szSrc[i]));
            CPPUNIT_ASSERT(_xcscmp(szTemp, szSrc[i]) == 0);

            _xcsncpy(szTempW, szSrc[i], strlen(szSrc[i]));
            CPPUNIT_ASSERT(_xcscmp(szTempW, szSrc[i]) == 0);
        }
    }

    void testUriUnquote()
    {
        cstr_t cases[] = { "%E5%BC%A0%E6%AF%85", "artist.movie.1080p.xxx", 
            "", "%2%X1", };

        cstr_t results[] = { "张毅", "artist.movie.1080p.xxx", "", "%2%X1", };

        CPPUNIT_ASSERT(uriIsQuoted(cases[0]));
        for (int i = 0; i < CountOf(cases); i++)
        {
            string r = uriUnquote(cases[i]);
            CPPUNIT_ASSERT(strcmp(results[i], r.c_str()) == 0);
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseStringEx);

#endif // _CPPUNIT_TEST
