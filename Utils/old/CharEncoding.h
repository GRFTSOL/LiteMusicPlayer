#pragma once

DECLAR_CPPUNIT_TEST_REG(CharEncodingBase)

enum CharEncodingType
{
    ED_SYSDEF,                    // ϵͳȱʡ�ı���
    ED_UNICODE,                    // unicode,
    ED_UNICODE_BIG_ENDIAN,        // unicode big-endian,
    ED_UTF8,                    // utf-8
    ED_ARABIC,                    // windows-1256
    // ED_BALTIC_ISO,                // iso-8859-4
    ED_BALTIC_WINDOWS,            // windows-1257
    // ED_CENTRAL_EUROPEAN_ISO,    // iso-8859-2
    ED_CENTRAL_EUROPEAN_WINDOWS,// windows-1250,
    ED_GB2312,                    // gb2312
    ED_BIG5,                    // big5
    // ED_CYRILLIC,                // iso-8859-5
    ED_CYRILLIC_WINDOWS,        // windows-1251
    // ED_GREEK_ISO,                // iso-8859-7
    ED_GREEK_WINDOWS,            // windows-1253
    ED_HEBREW_WINDOWS,            // windows-1255
    ED_JAPANESE_SHIFT_JIS,        //shift_jis
    ED_KOREAN,                    // ks_c_5601-1987
    ED_LATIN9_ISO,                // iso-8859-15,
    ED_THAI,                    // windows-874,
    // ED_TURKISH_ISO,                // iso-8859-9
    ED_TURKISH_WINDOWS,            // windows-1254,
    ED_VIETNAMESE,                // windows-1258
    // ED_WESTERN_EUROPEAN_ISO,    // iso-8859-1
    ED_WESTERN_EUROPEAN_WINDOWS,// Windows-1252,
    ED_EASTERN_EUROPEAN_WINDOWS,// Windows-1250,
    ED_RUSSIAN_WINDOWS,            // Windows-1251,
    ED_END = ED_RUSSIAN_WINDOWS,
};

#define SZ_UTF8                    "utf-8"
#define SZ_UNICODE                "unicode"

struct EncodingCodePage
{
    CharEncodingType        encodingID;

#ifdef _MAC_OS
    int                    cfStringEncoding;
#endif

#ifdef _WIN32
    int                    codePage;        // windows codepage
    int                    fontCharset;
#endif

    cstr_t                szEncoding;
    cstr_t                szAlias;
    cstr_t                szDesc;

#ifdef _LINUX
    cstr_t                szIConvCode;
#endif

#ifdef _ANDROID
    cstr_t                szIConvCode;
#endif

};

int getCharEncodingCount();

CharEncodingType getCharEncodingID(const char *szEncoding);
CharEncodingType getCharEncodingID(const WCHAR *szEncoding);

EncodingCodePage &getSysDefaultCharEncoding();

EncodingCodePage &getCharEncodingByID(CharEncodingType encoding);

//////////////////////////////////////////////////////////////////////////


// Is the string only including ANSI characters.
bool isAnsiStr(const WCHAR *szStr);
bool isAnsiStr(const char *szStr);

//////////////////////////////////////////////////////////////////////////

int ucs2ToUtf8(const WCHAR *str, int nLen, char *strOut, int nOut);
int ucs2ToMbcs(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);
int utf8ToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut);
int utf8ToMbcs(const char *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);
int mbcsToUtf8(const char *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);
int mbcsToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID = ED_SYSDEF);
int ansiToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut);
int ucs2ToAnsi(const WCHAR *str, int nLen, char *strOut, int nOut);

void ucs2ToUtf8(const WCHAR *str, int nLen, string &strOut);
void ucs2ToMbcs(const WCHAR *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void mbcsToUtf8(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void mbcsToUCS2(const char *str, int nLen, wstring_t &strOut, int encodingID = ED_SYSDEF);
void utf8ToMbcs(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void utf8ToUCS2(const char *str, int nLen, wstring_t &strOut);
void ansiToUCS2(const char *str, int nLen, wstring_t &strOut);
void ucs2ToAnsi(const WCHAR *str, int nLen, string &strOut);

void ucs2ToUtf8(const WCHAR *str, int nLen, string &strOut);
void ucs2ToMbcs(const WCHAR *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void utf8ToUCS2(const char *str, int nLen, string &strOut);
void utf8ToMbcs(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void mbcsToUtf8(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void mbcsToUCS2(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void ansiToUCS2(const char *str, int nLen, string &strOut);
void ucs2ToAnsi(const WCHAR *str, int nLen, string &strOut);

void uCS2BEToLE(WCHAR *str, int nLen);
void uCS2LEToBE(WCHAR *str, int nLen);

void convertStr(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void convertStr(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void convertStr(const WCHAR *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void convertStr(const WCHAR *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);

void convertStr(const char *str, int nLen, wstring_t &strOut, int encodingID = ED_SYSDEF);
void convertStr(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void convertStr(const WCHAR *str, int nLen, wstring_t &strOut, int encodingID = ED_SYSDEF);
void convertStr(const WCHAR *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);

void convertStr2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID = ED_SYSDEF);
void convertStr2(const char *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);
void convertStr2(const WCHAR *str, int nLen, WCHAR *strOut, int nOut, int encodingID = ED_SYSDEF);
void convertStr2(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);


//
// Current string encoding is one of the following types:
// 1) UNICODE: if UNICODE macro is defined
// 2) UTF8:    if UTF8 macro is defined
// 3) MBCS:    if the above two macros are not defined, and it is default.
//

template<class _tstring>
inline void ucs2ToTString(const WCHAR *str, int nLen, _tstring &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
    if (nLen == -1)
        strOut = str;
    else
    {
        strOut.clear();
        strOut.append(str, nLen);
    }
#elif defined(UTF8)
    ucs2ToUtf8(str, nLen, strOut);
#else
    ucs2ToMbcs(str, nLen, strOut, encodingID);
#endif
}

template<class _tstring>
inline void uTF8ToTString(const char *str, int nLen, _tstring &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
    utf8ToUCS2(str, nLen, strOut);
#elif defined(UTF8)
    if (nLen == -1)
        strOut = str;
    else
    {
        strOut.clear();
        strOut.append(str, nLen);
    }
#else
    utf8ToMbcs(str, nLen, strOut, encodingID);
#endif
}

template<class _tstring>
inline void mbcsToTString(const char *str, int nLen, _tstring &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
    mbcsToUCS2(str, nLen, strOut, encodingID);
#elif defined(UTF8)
    mbcsToUtf8(str, nLen, strOut, encodingID);
#else
    if (nLen == -1)
        strOut = str;
    else
    {
        strOut.clear();
        strOut.append(str, nLen);
    }
#endif
}

template<class _tstring>
inline void ansiToTString(const char *str, int nLen, _tstring &strOut)
{
#ifdef UNICODE
    ansiToUCS2(str, nLen, strOut);
#else
    if (nLen == -1)
        strOut = str;
    else
    {
        strOut.clear();
        strOut.append(str, nLen);
    }
#endif
}

template<class _wstring>
inline void tStringToUCS2(const char *str, int nLen, _wstring &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
    if (nLen == -1)
        strOut = str;
    else
    {
        strOut.clear();
        strOut.append(str, nLen);
    }
#elif defined(UTF8)
    utf8ToUCS2(str, nLen, strOut);
#else
    mbcsToUCS2(str, nLen, strOut, encodingID);
#endif
}

template<class _string>
inline void tStringToMBCS(const char *str, int nLen, _string &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
    ucs2ToMbcs(str, nLen, strOut, encodingID);
#elif defined(UTF8)
    utf8ToMbcs(str, nLen, strOut, encodingID);
#else
    if (nLen == -1)
        strOut = str;
    else
    {
        strOut.clear();
        strOut.append(str, nLen);
    }
#endif
}

template<class _string>
inline void stringToAnsi(const char *str, int nLen, _string &strOut)
{
#ifdef UNICODE
    ucs2ToAnsi(str, nLen, strOut);
#else
    if (nLen == -1)
        strOut = str;
    else
    {
        strOut.clear();
        strOut.append(str, nLen);
    }
#endif
}

