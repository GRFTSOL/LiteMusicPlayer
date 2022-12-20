#include "xstr.h"
#include "stringex_t.h"
#include "CharEncoding.h"
#include "string.h"
#include "safestr.h"

extern EncodingCodePage    __encodingCodepage[];

CharEncodingType getCharEncodingID(const char *szEncoding)
{
    if (isEmptyString(szEncoding))
        return ED_SYSDEF;

    assert(ED_END + 1 == getCharEncodingCount());
    for (int i = getCharEncodingCount() - 1; i >= 0; i--)
    {
        if (_stricmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
            return __encodingCodepage[i].encodingID;

        if (_stricmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
            return __encodingCodepage[i].encodingID;
        if (_stricmp(szEncoding, __encodingCodepage[i].szDesc) == 0)
            return __encodingCodepage[i].encodingID;
#if defined(_LINUX) || defined(_ANDROID)
        if (_stricmp(szEncoding, __encodingCodepage[i].szIConvCode) == 0)
            return __encodingCodepage[i].encodingID;
#endif

    }

    return ED_SYSDEF;
}

CharEncodingType getCharEncodingID(const WCHAR *szEncoding)
{
    assert(ED_END + 1 == getCharEncodingCount());
    for (int i = getCharEncodingCount() - 1; i >= 0; i--)
    {
        if (_xcsicmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
            return __encodingCodepage[i].encodingID;
        if (!isEmptyString(szEncoding))
        {
            if (_xcsicmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
                return __encodingCodepage[i].encodingID;
            if (_xcsicmp(szEncoding, __encodingCodepage[i].szDesc) == 0)
                return __encodingCodepage[i].encodingID;
        }
    }

    return ED_SYSDEF;
}

EncodingCodePage &getCharEncodingByID(CharEncodingType encoding)
{
    if (encoding >= getCharEncodingCount() || encoding < 0)
        encoding = ED_SYSDEF;

    assert(__encodingCodepage[encoding].encodingID == encoding);
    return __encodingCodepage[encoding];
}

//////////////////////////////////////////////////////////////////////////

bool isAnsiStr(const WCHAR *szStr)
{
    while (*szStr)
    {
        if (*szStr >= 128)
            return false;
        szStr++;
    }

    return true;
}

bool isAnsiStr(const char *szStr)
{
    while (*szStr)
    {
        if ((unsigned char)*szStr >= 128)
            return false;
        szStr++;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////

int ucs2ToUtf8(const WCHAR *str, int nLen, char *strOut, int nOut)
{
    ensureInputStrLen(str, nLen);

    cwstr_t        wchBegin = str, wEnd = str + nLen;
    int            nOutPos = 0;

    while (wchBegin < wEnd)
    {
        if (*wchBegin < 0x80)
        {
            if (nOutPos + 1 >= nOut)
                break;

            strOut[nOutPos] = (uint8_t)*wchBegin;
            nOutPos++;
        }
        else if (*wchBegin < 0x0800)
        {
            if (nOutPos + 2 >= nOut)
                break;

            strOut[nOutPos] = (uint8_t)((*wchBegin >> 6) | 0xC0);
            nOutPos++;
            strOut[nOutPos] = (uint8_t)((*wchBegin & 0x3F) | 0x80);
            nOutPos++;
        }
        else
        {
            if (nOutPos + 3 >= nOut)
                break;

            strOut[nOutPos] = (uint8_t)((*wchBegin >> 12) | 0xE0);
            nOutPos++;
            strOut[nOutPos] = (uint8_t)((*wchBegin >> 6 & 0x3F) | 0x80);
            nOutPos++;
            strOut[nOutPos] = (uint8_t)((*wchBegin & 0x3F) | 0x80);
            nOutPos++;
        }
        wchBegin++;
    }
    strOut[nOutPos] = '\0';

    return nOutPos;
}

#define MXS(c,x,s)            (((c) - (x)) <<  (s))
#define M80S(c,s)            MXS(c,0x80,s)
#define UTF8_1_to_UCS2(in)    ((WCHAR) (in)[0])
#define UTF8_2_to_UCS2(in)    ((WCHAR) (MXS((in)[0],0xC0, 6) | M80S((in)[1], 0)))
#define UTF8_3_to_UCS2(in)    ((WCHAR) (MXS((in)[0],0xE0,12) | M80S((in)[1], 6) | M80S((in)[2], 0) ))

int utf8ToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut)
{
    ensureInputStrLen(str, nLen);

    unsigned char* p = (unsigned char *)str;
    unsigned char* last = (unsigned char *)str + nLen;
    int        ul;
    for (ul = 0; (p < last) && (ul < nOut); )
    {
        if ((*p) < 0x80)
        {    
            strOut[ul++] = UTF8_1_to_UCS2(p);
            p += 1;
        }
        else if ((*p) < 0xc0)
        {    
            assert((*p));    // Invalid UTF8 First Byte
            strOut[ul++] = '?';
            p += 1;
        }
        else if ((*p) < 0xe0)
        {
            strOut[ul++] = UTF8_2_to_UCS2(p);
            p += 2;
        }
        else if ((*p) < 0xf0)
        {
            strOut[ul++] = UTF8_3_to_UCS2(p);
            p += 3;
        }
        else if ((*p) < 0xf8)
        {
            strOut[ul++] = '?';
            p += 4;
        }
        else if ((*p) < 0xfc)
        {
            strOut[ul++] = '?';
            p += 5;
        }
        else if ((*p) < 0xfe)
        {
            strOut[ul++] = '?';
            p += 6;
        }
        else
        {
            assert((*p));    // Invalid UTF8 First Byte
            strOut[ul++] = '?';
            p += 1;
        }
    }
    return ul;
}

int ansiToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut)
{
    ensureInputStrLen(str, nLen);

    if (nOut <= nLen)
        nLen = nOut - 1;

    _xcsncpy(strOut, str, nLen);
    strOut[nLen] = 0;

    return nLen;
}

int ucs2ToAnsi(const WCHAR *str, int nLen, char *strOut, int nOut)
{
    ensureInputStrLen(str, nLen);

    if (nOut <= nLen)
        nLen = nOut - 1;

    _xcsncpy(strOut, str, nLen);
    strOut[nLen] = 0;

    return nLen;
}

//////////////////////////////////////////////////////////////////////////

void ucs2ToUtf8(const WCHAR *str, int nLen, string &strOut)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen * 3 + 3);
    int n = ucs2ToUtf8(str, nLen, (char *)strOut.c_str(), (int)strOut.capacity());
    strOut.resize(n);
}

void ucs2ToMbcs(const WCHAR *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen * 2 + 2);
    int n = ucs2ToMbcs(str, nLen, (char *)strOut.c_str(), (int)strOut.capacity(), encodingID);
    strOut.resize(n);
}

void utf8ToUCS2(const char *str, int nLen, wstring_t &strOut)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen + 1);
    int n = utf8ToUCS2(str, nLen, (WCHAR *)strOut.c_str(), (int)strOut.capacity());
    strOut.resize(n);
}

void utf8ToMbcs(const char *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen + 1);
    int n = utf8ToMbcs(str, nLen, (char *)strOut.c_str(), (int)strOut.capacity(), encodingID);
    strOut.resize(n);
}

void mbcsToUtf8(const char *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen * 3 / 2 + 3);
    int n = mbcsToUtf8(str, nLen, (char *)strOut.c_str(), (int)strOut.capacity(), encodingID);
    strOut.resize(n);
}

void mbcsToUCS2(const char *str, int nLen, wstring_t &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen + 1);
    int n = mbcsToUCS2(str, nLen, (WCHAR *)strOut.c_str(), (int)strOut.capacity(), encodingID);
    strOut.resize(n);
}

void ansiToUCS2(const char *str, int nLen, wstring_t &strOut)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen);
    _xcsncpy((WCHAR *)strOut.c_str(), str, nLen);
}

void ucs2ToAnsi(const WCHAR *str, int nLen, string &strOut)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen);
    _xcsncpy((char *)strOut.c_str(), str, nLen);
}

//////////////////////////////////////////////////////////////////////////

void ucs2ToUtf8(const WCHAR *str, int nLen, string &strOut)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen * 3 + 3);
    int n = ucs2ToUtf8(str, nLen, strOut.data(), (int)strOut.capacity());
    strOut.resize(n);
}

void ucs2ToMbcs(const WCHAR *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen * 2 + 2);
    int n = ucs2ToMbcs(str, nLen, strOut.data(), (int)strOut.capacity(), encodingID);
    strOut.resize(n);
}

void utf8ToUCS2(const char *str, int nLen, string &strOut)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen + 1);
    int n = utf8ToUCS2(str, nLen, strOut.data(), (int)strOut.capacity());
    strOut.resize(n);
}

void utf8ToMbcs(const char *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen + 1);
    int n = utf8ToMbcs(str, nLen, strOut.data(), (int)strOut.capacity(), encodingID);
    strOut.resize(n);
}

void mbcsToUtf8(const char *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen * 3 / 2 + 3);
    int n = mbcsToUtf8(str, nLen, strOut.data(), (int)strOut.capacity(), encodingID);
    strOut.resize(n);
}

void mbcsToUCS2(const char *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen + 1);
    int n = mbcsToUCS2(str, nLen, strOut.data(), (int)strOut.capacity(), encodingID);
    strOut.resize(n);
}

void ansiToUCS2(const char *str, int nLen, string &strOut)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen);
    _xcsncpy((WCHAR *)strOut.c_str(), str, nLen);
}

void ucs2ToAnsi(const WCHAR *str, int nLen, string &strOut)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen);
    _xcsncpy((char *)strOut.c_str(), str, nLen);
}

//////////////////////////////////////////////////////////////////////////

void convertStr(const char *str, int nLen, string &strOut, int encodingID)
{
    mbcsToUCS2(str, nLen, strOut, encodingID);
}

void convertStr(const char *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.clear();
    strOut.append(str, nLen);
}

void convertStr(const WCHAR *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);
    strOut.clear();
    strOut.append(str, nLen);
}

void convertStr(const WCHAR *str, int nLen, string &strOut, int encodingID)
{
    ucs2ToMbcs(str, nLen, strOut, encodingID);
}

//////////////////////////////////////////////////////////////////////////

void convertStr(const char *str, int nLen, wstring_t &strOut, int encodingID)
{
    mbcsToUCS2(str, nLen, strOut, encodingID);
}

void convertStr(const char *str, int nLen, string &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);

    strOut.clear();
    strOut.append(str, nLen);
}

void convertStr(const WCHAR *str, int nLen, wstring_t &strOut, int encodingID)
{
    ensureInputStrLen(str, nLen);
    strOut.clear();
    strOut.append(str, nLen);
}

void convertStr(const WCHAR *str, int nLen, string &strOut, int encodingID)
{
    ucs2ToMbcs(str, nLen, strOut, encodingID);
}

//////////////////////////////////////////////////////////////////////////

void convertStr2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID)
{
    mbcsToUCS2(str, nLen, strOut, nOut, encodingID);
}

void convertStr2(const char *str, int nLen, char *strOut, int nOut, int encodingID)
{
    if (nLen == -1)
        strcpy_safe(strOut, nOut, str);
    else
    {
        nLen = (int)strlen(str);
        strncpy_safe(strOut, nOut, str, nLen);
    }
}

void convertStr2(const WCHAR *str, int nLen, WCHAR *strOut, int nOut, int encodingID)
{
    if (nLen == -1)
        wcscpy_safe(strOut, nOut, str);
    else
    {
        nLen = (int)wcslen(str);
        wcsncpy_safe(strOut, nOut, str, nLen);
    }
}

void convertStr2(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID)
{
    ucs2ToMbcs(str, nLen, strOut, nOut, encodingID);
}

void uCS2BEToLE(WCHAR *str, int nLen)
{
    // Swap the char of every WCHAR.
    if (nLen == -1)
        nLen = (int)wcslen(str);

    uint8_t    *p = (uint8_t *)str;
    int        lenBytes = nLen * sizeof(WCHAR);
    for (int i = 0; i < lenBytes; i += 2)
    {
        uint8_t t = p[i];
        p[i] = p[i + 1];
        p[i + 1] = t;
    }
}

void uCS2LEToBE(WCHAR *str, int nLen)
{
    uCS2BEToLE(str, nLen);
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CharEncodingBase)

class CTestCaseCharEncodingBase : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCaseCharEncodingBase);
    CPPUNIT_TEST(testUcs2ToUtf8);
    CPPUNIT_TEST(testAnsiToUcs2);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testUcs2ToUtf8()
    {
        string out;
        string outx;
        char szOut[256];

        WCHAR        input[255];
        _xcscpy(input, "test String abc");

        // test with full length: -1
        ucs2ToUtf8(input, -1, out);
        ucs2ToUtf8(input, -1, outx);
        int n = ucs2ToUtf8(input, -1, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == wcslen(input));
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

        // test with shorter length
        int            nLen = 5;
        ucs2ToUtf8(input, nLen, out);
        ucs2ToUtf8(input, nLen, outx);
        n = ucs2ToUtf8(input, nLen, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == nLen);
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

        // test with insufficient length.
        nLen = 5;
        n = ucs2ToUtf8(input, -1, szOut, nLen + 1);
        CPPUNIT_ASSERT(n == nLen);
        CPPUNIT_ASSERT(strlen(szOut) == nLen);
        CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);

#ifdef _WIN32
        {
            wcscpy(input, L"���Ĳ������Ĳ������Ĳ������Ĳ������Ĳ������Ĳ������Ĳ���");
            ucs2ToUtf8(input, -1, out);

            // Use mbcsToUCS2 with ED_UTF8 encoding to convert.
            wstring_t strInputNew;
            mbcsToUCS2(out.c_str(), out.size(), strInputNew, ED_UTF8);
            CPPUNIT_ASSERT(strInputNew == input);

            wcscpy(input, L"cosMo@����P���ǥåɥܩ`��P feat.�����ߥ�");
            ucs2ToUtf8(input, -1, out);

            // Use mbcsToUCS2 with ED_UTF8 encoding to convert.
            mbcsToUCS2(out.c_str(), out.size(), strInputNew, ED_UTF8);
            CPPUNIT_ASSERT(strInputNew == input);

            wstring_t strOutW;
            utf8ToUCS2(out.c_str(), out.length(), strOutW);
            CPPUNIT_ASSERT(strOutW == strInputNew);
        }
#endif
    }

    void testUcs2ToMbcs()
    {
        string out;
        string outx;
        char szOut[256];

        WCHAR        input[255];
        _xcscpy(input, "test String abc");

        // test with full length: -1
        ucs2ToMbcs(input, -1, out);
        ucs2ToMbcs(input, -1, outx);
        int n = ucs2ToMbcs(input, -1, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == wcslen(input));
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

        // test with shorter length
        int            nLen = 5;
        ucs2ToMbcs(input, nLen, out);
        ucs2ToMbcs(input, nLen, outx);
        n = ucs2ToMbcs(input, nLen, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == nLen);
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

        // test with insufficient length.
        nLen = 5;
        n = ucs2ToMbcs(input, -1, szOut, nLen);
        CPPUNIT_ASSERT(n == nLen);
        CPPUNIT_ASSERT(strlen(szOut) == nLen);
        CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);

#ifdef _WIN32
        {
            wcscpy(input, L"���Ĳ������Ĳ������Ĳ������Ĳ������Ĳ������Ĳ������Ĳ���");
            ucs2ToMbcs(input, -1, out);
            CPPUNIT_ASSERT(out.size() == wcslen(input));

            wstring_t strInputNew;
            mbcsToUCS2(out.c_str(), out.size(), strInputNew);
            CPPUNIT_ASSERT(strInputNew == input);
        }
#endif
    }

    void testMbcsToUcs2()
    {
        wstring_t out;
        string outx;
        WCHAR szOut[256];

        cstr_t        input = "test String abc";

        // test with full length: -1
        mbcsToUCS2(input, -1, out);
        mbcsToUCS2(input, -1, outx);
        int n = mbcsToUCS2(input, -1, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == strlen(input));
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

        // test with shorter length
        int            nLen = 5;
        mbcsToUCS2(input, nLen, out);
        mbcsToUCS2(input, nLen, outx);
        n = mbcsToUCS2(input, nLen, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == nLen);
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

        // test with insufficient length.
        nLen = 5;
        n = mbcsToUCS2(input, -1, szOut, nLen);
        CPPUNIT_ASSERT(n == nLen);
        CPPUNIT_ASSERT(wcslen(szOut) == nLen);
        CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);
    }

    void testMbcsToUtf8()
    {
        string out;
        string outx;
        char szOut[256];

        cstr_t        input = "test String abc";

        // test with full length: -1
        mbcsToUtf8(input, -1, out);
        mbcsToUtf8(input, -1, outx);
        int n = mbcsToUtf8(input, -1, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == strlen(input));
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

        // test with shorter length
        int            nLen = 5;
        mbcsToUtf8(input, nLen, out);
        mbcsToUtf8(input, nLen, outx);
        n = mbcsToUtf8(input, nLen, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == nLen);
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

        // test with insufficient length.
        nLen = 5;
        n = mbcsToUtf8(input, -1, szOut, nLen);
        CPPUNIT_ASSERT(n == nLen);
        CPPUNIT_ASSERT(strlen(szOut) == nLen);
        CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);

#ifdef _WIN32
        {
            input = "���Ĳ������Ĳ������Ĳ������Ĳ������Ĳ������Ĳ������Ĳ���";
            mbcsToUtf8(input, -1, out);

            string strInputNew;
            utf8ToMbcs(out.c_str(), out.size(), strInputNew);
            CPPUNIT_ASSERT(strInputNew == input);
        }
#endif
    }

    void testUtf8ToMbcs()
    {
        string out;
        string outx;
        char szOut[256];

        cstr_t        input = "test String abc";

        // test with full length: -1
        utf8ToMbcs(input, -1, out);
        utf8ToMbcs(input, -1, outx);
        int n = utf8ToMbcs(input, -1, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == strlen(input));
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

        // test with shorter length
        int            nLen = 5;
        utf8ToMbcs(input, nLen, out);
        utf8ToMbcs(input, nLen, outx);
        n = utf8ToMbcs(input, nLen, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == nLen);
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

        // test with insufficient length.
        nLen = 5;
        n = utf8ToMbcs(input, -1, szOut, nLen);
        CPPUNIT_ASSERT(n == nLen);
        CPPUNIT_ASSERT(strlen(szOut) == nLen);
        CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);
    }

    void testUtf8ToUcs2()
    {
        wstring_t out;
        string outx;
        WCHAR szOut[256];

        cstr_t        input = "test String abc";

        // test with full length: -1
        utf8ToUCS2(input, -1, out);
        utf8ToUCS2(input, -1, outx);
        int n = utf8ToUCS2(input, -1, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == strlen(input));
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcscmp(out.c_str(), input) == 0);

        // test with shorter length
        int            nLen = 5;
        utf8ToUCS2(input, nLen, out);
        utf8ToUCS2(input, nLen, outx);
        n = utf8ToUCS2(input, nLen, szOut, CountOf(szOut));
        CPPUNIT_ASSERT(out.size() == n);
        CPPUNIT_ASSERT(outx.size() == n);
        CPPUNIT_ASSERT(out.size() == nLen);
        CPPUNIT_ASSERT(out == outx.c_str());
        CPPUNIT_ASSERT(out == szOut);
        CPPUNIT_ASSERT(_xcsncmp(out.c_str(), input, nLen) == 0);

        // test with insufficient length.
        nLen = 5;
        n = utf8ToUCS2(input, -1, szOut, nLen);
        CPPUNIT_ASSERT(n == nLen);
        CPPUNIT_ASSERT(wcslen(szOut) == nLen);
        CPPUNIT_ASSERT(_xcsncmp(szOut, input, nLen) == 0);
    }

    void testAnsiToUcs2()
    {
        const int bufLen = 256;
        uint8_t    buf[bufLen];

        wstring_t    strUcs2;
        ansiToUCS2((const char *)buf, bufLen, strUcs2);
        CPPUNIT_ASSERT(strUcs2.size() == bufLen);

        string        strAnsi;
        ucs2ToAnsi(strUcs2.c_str(), strUcs2.size(), strAnsi);
        CPPUNIT_ASSERT(strAnsi.size() == bufLen);
        CPPUNIT_ASSERT(memcmp(strAnsi.c_str(), buf, bufLen) == 0);

        cstr_t    strTest = "abcdefalsgeoie1123";
        ansiToUCS2(strTest, -1, strUcs2);
        CPPUNIT_ASSERT(strUcs2.size() == strlen(strTest));

        ucs2ToAnsi(strUcs2.c_str(), -1, strAnsi);
        CPPUNIT_ASSERT(strAnsi.size() == strlen(strTest));
        CPPUNIT_ASSERT(memcmp(strAnsi.c_str(), strTest, strlen(strTest) + 1) == 0);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCharEncodingBase);


#endif // _CPPUNIT_TEST
