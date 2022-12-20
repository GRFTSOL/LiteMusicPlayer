// CharEncoding.cpp: implementation of the CCharEncoding class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "../base.h"
#include "../CharEncoding.h"
#include <iconv.h>

#define DBG_LOG1 printf

//
// ED_XXX 的定义和 __encodingCodepage 的索引顺序是一直的。
// 即__encodingCodepage[ED_XXX].encodingID = ED_XXX
EncodingCodePage    __encodingCodepage[] = 
{
    { ED_SYSDEF, "", "", "systemdefault", "ISO-8859-15" },
    { ED_UNICODE, "unicode", "", "Unicode", "unicode" },
    { ED_UNICODE_BIG_ENDIAN, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)", "unicode" },
    { ED_UTF8, "utf-8", "", "Unicode (UTF-8)", "utf-8" },
    { ED_ARABIC, "windows-1256", "arabic", "Arabic", "windows-1256" },
    { ED_BALTIC_WINDOWS, "windows-1257", "baltic", "Baltic (Windows)", "windows-1257" },
    { ED_CENTRAL_EUROPEAN_WINDOWS, "windows-1250", "", "Central European (Windows)", "windows-1250" },
    { ED_GB2312, "gb2312", "gb2312", "Chinese Simplified (GB2312)", "gb2312" },
    { ED_BIG5, "big5", "big5", "Chinese Traditional (Big5)", "big5" },
    { ED_CYRILLIC_WINDOWS, "windows-1251", "", "Cyrillic (Windows)", "windows-1251" },
    { ED_GREEK_WINDOWS, "windows-1253", "greek", "Greek (Windows)", "windows-1253" },
    { ED_HEBREW_WINDOWS, "windows-1255", "hebrew", "Hebrew (Windows)", "windows-1255" },
    { ED_JAPANESE_SHIFT_JIS, "shift_jis", "shiftjis", "Japanese (Shift-JIS)", "shift_jis" },
    { ED_KOREAN, "ks_c_5601-1987", "hangeul", "Korean", "CP949" },
    { ED_LATIN9_ISO, "iso-8859-15", "", "Latin 9 (ISO)", "iso-8859-15" },
    { ED_THAI, "windows-874", "Thai", "Thai (Windows)", "CP874" },
    { ED_TURKISH_WINDOWS, "windows-1254", "turkish", "Turkish (Windows)", "windows-1254" },
    { ED_VIETNAMESE, "windows-1258", "vietnamese", "Vietnamese (Windows)", "windows-1258" },
    { ED_WESTERN_EUROPEAN_WINDOWS, "Windows-1252", "", "Western European (Windows)", "Windows-1252" },
    { ED_EASTERN_EUROPEAN_WINDOWS, "Windows-1250", "easteurope", "Eastern European (Windows)", "Windows-1250" },
    { ED_RUSSIAN_WINDOWS, "Windows-1251", "russian", "Russian (Windows)", "Windows-1251" },
};

#define         __MaxEncodings        CountOf(__encodingCodepage)

int getCharEncodingCount() { return __MaxEncodings; }


EncodingCodePage &getSysDefaultCharEncoding()
{
    return __encodingCodepage[ED_UTF8];
}

int mbcsToUtf8(const char *str, int nLen, char *strOut, int nOut, int encodingID)
{
    if (encodingID == ED_UTF8)
    {
        if (nLen == -1)
        {
            strcpy_safe(strOut, nOut, str);
            return strlen(strOut);
        }
        else
        {
            strncpysz_safe(strOut, nOut, str, nLen);
            return nLen;
        }
    }

    emptyStr(strOut);

    if (encodingID > __MaxEncodings || encodingID < 0)
        encodingID = 0;

    assert(encodingID == __encodingCodepage[encodingID].encodingID);

    iconv_t code = iconv_open(SZ_UTF8, __encodingCodepage[encodingID].szIConvCode);
    if (code == (iconv_t)-1)
        return 0;

    int outBytesLeft = nOut;
    int n = iconv(code, (char **)&str, (size_t*)&nLen, (char **)(&strOut), (size_t*)&outBytesLeft);
    if (n == -1)
    {
        DBG_LOG1("FAILED to convert str to unicode: %s", str);
        n = 0;
    }
    else
    {
        n = nOut - outBytesLeft;
        assert(n >= 0);
        if (n < 0)
            n = 0;
    }

    strOut[n] = '\0';
    iconv_close(code);

    return n;
}

int utf8ToMbcs(const char *str, int nLen, char *strOut, int nOut, int encodingID)
{
    if (encodingID == ED_UTF8)
    {
        if (nLen == -1)
        {
            strcpy_safe(strOut, nOut, str);
            return strlen(strOut);
        }
        else
        {
            strncpysz_safe(strOut, nOut, str, nLen);
            return nLen;
        }
    }

    emptyStr(strOut);

    if (encodingID > __MaxEncodings || encodingID < 0)
        encodingID = 0;

    assert(encodingID == __encodingCodepage[encodingID].encodingID);

    iconv_t code = iconv_open(__encodingCodepage[encodingID].szIConvCode, SZ_UTF8);
    if (code == (iconv_t)-1)
        return 0;

    int outBytesLeft = nOut;
    int n = iconv(code, (char **)&str, (size_t*)&nLen, (char **)(&strOut), (size_t*)&outBytesLeft);
    if (n == -1)
    {
        DBG_LOG1("FAILED to convert str to unicode: %s", str);
        n = 0;
    }
    else
    {
        n = nOut - outBytesLeft;
        assert(n >= 0);
        if (n < 0)
            n = 0;
    }

    strOut[n] = '\0';
    iconv_close(code);

    return n;    
}

int mbcsToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID)
{
    emptyStr(strOut);

    if (encodingID > __MaxEncodings || encodingID < 0)
        encodingID = 0;

    assert(encodingID == __encodingCodepage[encodingID].encodingID);

    iconv_t    code = iconv_open(SZ_UNICODE, __encodingCodepage[encodingID].szIConvCode);
    if (code == (iconv_t)-1)
        return 0;

    int outBytesLeft = nOut * sizeof(WCHAR);
    int n = iconv(code, (char **)&str, (size_t*)&nLen, (char **)(&strOut), (size_t*)&nOut);
    if (n == -1)
    {
        DBG_LOG1("FAILED to convert str to unicode: %s", str);
        n = 0;
    }
    else
    {
        n = nOut - outBytesLeft / sizeof(WCHAR);
        assert(n >= 0);
        if (n < 0)
            n = 0;
    }

    strOut[n] = '\0';
    iconv_close(code);

    return n;
}

int ucs2ToMbcs(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID)
{
    emptyStr(strOut);
    if (encodingID > __MaxEncodings || encodingID < 0)
        encodingID = 0;

    assert(encodingID == __encodingCodepage[encodingID].encodingID);

    iconv_t code = iconv_open(__encodingCodepage[encodingID].szIConvCode, SZ_UNICODE);
    if (code == (iconv_t)-1)
        return 0;

    int outBytesLeft = nOut;
    int n = iconv(code, (char **)&str, (size_t*)&nLen, (char **)(&strOut), (size_t*)&outBytesLeft);
    if (n == -1)
        n = 0;
    else
        n = nOut - outBytesLeft;

    strOut[n] = '\0';
    iconv_close(code);

    return n;
}

