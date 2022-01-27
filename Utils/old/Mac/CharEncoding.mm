// CharEncoding.cpp: implementation of the CCharEncoding class.
//
//////////////////////////////////////////////////////////////////////


#import <Foundation/Foundation.h>

#include <iconv.h>
#include <CoreFoundation/CoreFoundation.h>
#include "stringex_t.h"
#include "CharEncoding.h"
#include "safestr.h"
#include "string.h"


//
// ED_XXX 的定义和 __encodingCodepage 的索引顺序是一直的。
// 即__encodingCodepage[ED_XXX].nEncodingId = ED_XXX
ENCODING_CODEPAGE    __encodingCodepage[] =
{
    { ED_SYSDEF, kCFStringEncodingWindowsLatin1, "", "", "systemdefault" },
    { ED_UNICODE, kCFStringEncodingUTF16LE, "unicode", "", "Unicode" },
    { ED_UNICODE_BIG_ENDIAN, kCFStringEncodingUTF16BE, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)" },
    { ED_UTF8, kCFStringEncodingUTF8, "utf-8", "", "Unicode (UTF-8)" },
    { ED_ARABIC, kCFStringEncodingWindowsArabic, "windows-1256", "arabic", "Arabic" },
    { ED_BALTIC_WINDOWS, kCFStringEncodingWindowsBalticRim, "windows-1257", "baltic", "Baltic (Windows)" },
    { ED_CENTRAL_EUROPEAN_WINDOWS, kCFStringEncodingWindowsLatin1, "windows-1250", "", "Central European (Windows)" },
    { ED_GB2312, kCFStringEncodingGB_18030_2000, "gb2312", "gb2312", "Chinese Simplified (GB2312)" },
    { ED_BIG5, kCFStringEncodingDOSChineseTrad, "big5", "big5", "Chinese Traditional (Big5)" },
    { ED_CYRILLIC_WINDOWS, kCFStringEncodingWindowsCyrillic, "windows-1251", "", "Cyrillic (Windows)" },
    { ED_GREEK_WINDOWS, kCFStringEncodingWindowsGreek, "windows-1253", "greek", "Greek (Windows)" },
    { ED_HEBREW_WINDOWS, kCFStringEncodingWindowsHebrew, "windows-1255", "hebrew", "Hebrew (Windows)" },
    { ED_JAPANESE_SHIFT_JIS, kCFStringEncodingDOSJapanese, "shift_jis", "shiftjis", "Japanese (Shift-JIS)" },
    { ED_KOREAN, kCFStringEncodingDOSKorean, "ks_c_5601-1987", "hangeul", "Korean" },
    { ED_LATIN9_ISO, kCFStringEncodingWindowsLatin1, "iso-8859-15", "", "Latin 9 (ISO)" },
    { ED_THAI, kCFStringEncodingDOSThai, "windows-874", "Thai", "Thai (Windows)" },
    { ED_TURKISH_WINDOWS, kCFStringEncodingWindowsLatin5, "windows-1254", "turkish", "Turkish (Windows)" },
    { ED_VIETNAMESE, kCFStringEncodingWindowsVietnamese, "windows-1258", "vietnamese", "Vietnamese (Windows)" },
    { ED_WESTERN_EUROPEAN_WINDOWS, kCFStringEncodingWindowsLatin2, "Windows-1252", "", "Western European (Windows)" },
    { ED_EASTERN_EUROPEAN_WINDOWS, kCFStringEncodingWindowsLatin1, "Windows-1250", "easteurope", "Eastern European (Windows)" },
    { ED_RUSSIAN_WINDOWS, kCFStringEncodingWindowsCyrillic, "Windows-1251", "russian", "Russian (Windows)" },
};

#define         __MaxEncodings        CountOf(__encodingCodepage)

int GetCharEncodingCount() { return __MaxEncodings; }

void InitSetDefaultCharEncoding()
{
    ENCODING_CODEPAGE &ecp = GetCharEncodingByID(ED_SYSDEF);
    NSLocale *local = [NSLocale currentLocale];
    NSString *lang = [local objectForKey:NSLocaleLanguageCode];
    NSString *countryCode = [local objectForKey: NSLocaleCountryCode];
    
    // Compare country code
    if ([countryCode isEqual: @"CN"] || [countryCode isEqual: @"SG"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_GB2312).nCFStringEncoding;
    else if ([countryCode isEqual: @"TW"] || [countryCode isEqual: @"HK"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_BIG5).nCFStringEncoding;
    else if ([countryCode isEqual: @"JP"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_JAPANESE_SHIFT_JIS).nCFStringEncoding;
    
    // Compare language code
    else if ([lang isEqual: @"ara"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_ARABIC).nCFStringEncoding;
    else if ([lang isEqual: @"bat"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_BALTIC_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"gre"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_GREEK_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"heb"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_HEBREW_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"tha"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_THAI).nCFStringEncoding;
    else if ([lang isEqual: @"tur"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_TURKISH_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"vie"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_VIETNAMESE).nCFStringEncoding;
    else if ([lang isEqual: @"rus"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_RUSSIAN_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"kor"])
        ecp.nCFStringEncoding = GetCharEncodingByID(ED_KOREAN).nCFStringEncoding;
}

ENCODING_CODEPAGE &GetSysDefaultCharEncoding()
{
    static bool defaultSet = false;
    if (!defaultSet) {
        InitSetDefaultCharEncoding();
        defaultSet = true;
    }
    return __encodingCodepage[ED_SYSDEF];
}

NSString *initNSString(const char *str, int nLen, int encodingID)
{
    NSString    *temp;
    string    strIn;
    if (str[nLen] != '\0')
    {
        strIn.append(str, nLen);
        str = strIn.c_str();
    }
    
    temp = [NSString stringWithCString:str encoding:(NSStringEncoding)
            CFStringConvertEncodingToNSStringEncoding(
                                                      GetCharEncodingByID((CHAR_ENCODING)encodingID).nCFStringEncoding)];
    if (!temp)
        temp = [NSString stringWithCString:str encoding:(NSStringEncoding)NSISOLatin1StringEncoding];
    
    return temp;
}

int MbcsToUtf8(const char *str, int nLen, char *strOut, int nOut, int encodingID)
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
    
    NSString *temp = initNSString(str, nLen, encodingID);
    if (temp)
        strcpy_safe(strOut, nOut, [temp UTF8String]);
    else
        return 0;
    
    return strlen(strOut);
}

int Utf8ToMbcs(const char *str, int nLen, char *strOut, int nOut, int encodingID)
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
    
    NSString    *temp;
    temp = [NSString stringWithUTF8String:str];
    if (temp)
    {
        [temp getCString:strOut
              maxLength:nOut
               encoding:CFStringConvertEncodingToNSStringEncoding(
                                                                  GetCharEncodingByID((CHAR_ENCODING)encodingID).nCFStringEncoding)];
    } else {
        return 0;
    }

    return strlen(strOut);    
}

int MbcsToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID)
{
    emptyStr(strOut);
    
    NSString *temp = initNSString(str, nLen, encodingID);
    if (temp)
    {
        if (nOut > [temp length])
            nOut = [temp length];
        [temp getCharacters:(unichar *)strOut range: NSMakeRange(0, nOut)];
        strOut[nOut] = '\0';
    } else {
        return 0;
    }

    return wcslen(strOut);
}

int ucs2ToMbcs(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID)
{
    emptyStr(strOut);
    
    NSString    *temp;
    temp = [[NSString alloc] initWithCharacters:(const unichar*)str length:nLen];
    if (temp) {
        [temp getCString:strOut
              maxLength:nOut
               encoding:CFStringConvertEncodingToNSStringEncoding(
                                                                  GetCharEncodingByID((CHAR_ENCODING)encodingID).nCFStringEncoding)];
        [temp release];
    } else {
        return 0;
    }
    
    return strlen(strOut);
}
