//
//  CharEncodingMac.mm
//
//  Created by HongyongXiao on 2021/11/21.
//

#include <Foundation/Foundation.h>
#include <iconv.h>
#include <CoreFoundation/CoreFoundation.h>
#include "../CharEncoding.h"


//
// ED_XXX 的定义和 g_encodingCodePages 的索引顺序是一直的。
// 即g_encodingCodePages[ED_XXX].nEncodingId = ED_XXX
ENCODING_CODEPAGE    g_encodingCodePages[] =
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

int getCharEncodingCount() { return CountOf(g_encodingCodePages); }

bool initSetDefaultCharEncoding()
{
    ENCODING_CODEPAGE &ecp = getCharEncodingByID(ED_SYSDEF);
    NSLocale *local = [NSLocale currentLocale];
    NSString *lang = [local objectForKey:NSLocaleLanguageCode];
    NSString *countryCode = [local objectForKey: NSLocaleCountryCode];

    // Compare country code
    if ([countryCode isEqual: @"CN"] || [countryCode isEqual: @"SG"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_GB2312).nCFStringEncoding;
    else if ([countryCode isEqual: @"TW"] || [countryCode isEqual: @"HK"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_BIG5).nCFStringEncoding;
    else if ([countryCode isEqual: @"JP"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_JAPANESE_SHIFT_JIS).nCFStringEncoding;

    // Compare language code
    else if ([lang isEqual: @"ara"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_ARABIC).nCFStringEncoding;
    else if ([lang isEqual: @"bat"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_BALTIC_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"gre"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_GREEK_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"heb"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_HEBREW_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"tha"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_THAI).nCFStringEncoding;
    else if ([lang isEqual: @"tur"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_TURKISH_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"vie"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_VIETNAMESE).nCFStringEncoding;
    else if ([lang isEqual: @"rus"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_RUSSIAN_WINDOWS).nCFStringEncoding;
    else if ([lang isEqual: @"kor"])
        ecp.nCFStringEncoding = getCharEncodingByID(ED_KOREAN).nCFStringEncoding;
    return true;
}

ENCODING_CODEPAGE &GetSysDefaultCharEncoding()
{
    return g_encodingCodePages[ED_SYSDEF];
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
                                                      getCharEncodingByID((CHAR_ENCODING)encodingID).nCFStringEncoding)];
    if (!temp)
        temp = [NSString stringWithCString:str encoding:(NSStringEncoding)NSISOLatin1StringEncoding];

    return temp;
}

int mbcsToUtf8(const char *str, int nLen, string &strOut, int encodingID)
{
    strOut.clear();

    if (encodingID == ED_UTF8)
    {
        if (nLen == -1)
        {
            strOut.assign(str);
        }
        else
        {
            strOut.assign(str, nLen);
        }
        return strOut.size();
    }

    NSString *temp = initNSString(str, nLen, encodingID);
    if (temp) {
        strOut.assign([temp UTF8String]);
    } else {
        return 0;
    }

    return strOut.size();
}

int utf8ToMbcs(const char *str, int nLen, string &strOut, int encodingID)
{
    if (encodingID == ED_UTF8)
    {
        if (nLen == -1)
        {
            strOut.assign(str);
        }
        else
        {
            strOut.assign(str, nLen);
        }
        return (int)strOut.size();
    }

    NSString *temp = [NSString stringWithUTF8String:str];
    if (temp)
    {
        strOut.resize(nLen + 5);
        [temp getCString:(char *)strOut.c_str()
               maxLength:strOut.size()
                encoding:CFStringConvertEncodingToNSStringEncoding(
                    getCharEncodingByID((CHAR_ENCODING)encodingID).nCFStringEncoding)];
    } else {
        return 0;
    }

    return strOut.size();
}

static bool _isCharEncodingInitialized_ = initSetDefaultCharEncoding();
