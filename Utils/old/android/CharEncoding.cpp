#include "../stdafx.h"
#include "../../base/base.h"
#include "../CharEncoding.h"

#define DBG_LOG1            printf
#include <android/log.h>


//__android_log_write(ANDROID_LOG_ERROR, "Tag", encoding);

// The order of ED_XXX is the order of __encodingCodepage
// __encodingCodepage[ED_XXX].encodingID = ED_XXX
EncodingCodePage __encodingCodepage[] = {
    { ED_SYSDEF, "iso-8859-15", "", "Latin 9 (ISO)", "ISO8859_15_FDIS" },
    { ED_UNICODE, "unicode", "", "Unicode", "UnicodeLittleUnmarked" },
    { ED_UNICODE_BIG_ENDIAN, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)", "UnicodeBigUnmarked" },
    { ED_UTF8, "utf-8", "", "Unicode (UTF-8)", "UTF8" },
    { ED_ARABIC, "windows-1256", "arabic", "Arabic", "Cp1256" },
    { ED_BALTIC_WINDOWS, "windows-1257", "baltic", "Baltic (Windows)", "Cp1257" },
    { ED_CENTRAL_EUROPEAN_WINDOWS, "windows-1250", "", "Central European (Windows)", "Cp1250" },
    { ED_GB2312, "gb2312", "gb2312", "Chinese Simplified (GB2312)", "EUC_CN" },
    { ED_BIG5, "big5", "big5", "Chinese Traditional (Big5)", "big5" },
    { ED_CYRILLIC_WINDOWS, "windows-1251", "", "Cyrillic (Windows)", "Cp1251" },
    { ED_GREEK_WINDOWS, "windows-1253", "greek", "Greek (Windows)", "Cp1253" },
    { ED_HEBREW_WINDOWS, "windows-1255", "hebrew", "Hebrew (Windows)", "Cp1255" },
    { ED_JAPANESE_SHIFT_JIS, "shift_jis", "shiftjis", "Japanese (Shift-JIS)", "SJIS" },
    { ED_KOREAN, "ks_c_5601-1987", "hangeul", "Korean", "CP949" },
    { ED_LATIN9_ISO, "iso-8859-15", "", "Latin 9 (ISO)", "ISO8859_15_FDIS" },
    { ED_THAI, "windows-874", "Thai", "Thai (Windows)", "CP874" },
    { ED_TURKISH_WINDOWS, "windows-1254", "turkish", "Turkish (Windows)", "Cp1254" },
    { ED_VIETNAMESE, "windows-1258", "vietnamese", "Vietnamese (Windows)", "Cp1258" },
    { ED_WESTERN_EUROPEAN_WINDOWS, "Windows-1252", "", "Western European (Windows)", "Cp1252" },
    { ED_EASTERN_EUROPEAN_WINDOWS, "Windows-1250", "easteurope", "Eastern European (Windows)", "Cp1250" },
    { ED_RUSSIAN_WINDOWS, "Windows-1251", "russian", "Russian (Windows)", "Cp1251" },
};

#define         __MaxEncodings        CountOf(__encodingCodepage)

int getCharEncodingCount() { return __MaxEncodings; }
int g_defaultSysEncoding = ED_SYSDEF;

EncodingCodePage &getSysDefaultCharEncoding() {
    return __encodingCodepage[g_defaultSysEncoding];
}

void setSysDefaultCharEncoding(cstr_t szEncoding) {
    g_defaultSysEncoding = getCharEncodingID(szEncoding);
    __encodingCodepage[ED_SYSDEF] = __encodingCodepage[g_defaultSysEncoding];
    __encodingCodepage[ED_SYSDEF].encodingID = ED_SYSDEF;
}

jclass g_clsString = nullptr;
jmethodID g_methodStringInitWithBufEncoding = nullptr;
jmethodID g_methodStringGetBytesEncoding = nullptr;

bool prepareJString(JNIEnv *env) {
    if (g_clsString == nullptr) {
        jclass clsString = env->FindClass("java/lang/String");
        if (clsString == nullptr) {
            return false;
        }

        g_clsString = reinterpret_cast<jclass>(env->NewGlobalRef(clsString));
    }

    if (g_methodStringInitWithBufEncoding == nullptr) {
        jmethodID mtdString = env->GetMethodID(g_clsString, "<init>", "([BLjava/lang/String;)V");
        if (mtdString == nullptr) {
            return false;
        }

        g_methodStringInitWithBufEncoding = mtdString;
    }

    if (g_methodStringGetBytesEncoding == nullptr) {
        jmethodID mtdString = env->GetMethodID(g_clsString, "getBytes", "(Ljava/lang/String;)[B");
        if (mtdString == nullptr) {
            return false;
        }

        g_methodStringGetBytesEncoding = mtdString;
    }

    return true;
}

jstring newJString(JNIEnv *env, const char *str, int nLen, const char *encoding);

int getBytesOfJString(JNIEnv *env, jstring jstr, char *strOut, int nOut, const char *encoding) {
    if (!prepareJString(env) || nOut <= 0) {
        return 0;
    }

    jbyteArray buf = (jbyteArray)env->CallObjectMethod(jstr, g_methodStringGetBytesEncoding, env->NewStringUTF(encoding));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return 0;
    }
    if (buf == nullptr) {
        return 0;
    }

    int length = env->GetArrayLength(buf);
    if (length > nOut - 1) {
        length = nOut - 1;
    }

    env->GetByteArrayRegion(buf, 0, length, (jbyte *)strOut);
    strOut[length] = '\0';

    env->DeleteLocalRef(buf);

    return length;
}

int mbcsToUtf8(const char *str, int nLen, char *strOut, int nOut, int encodingID) {
    if (encodingID == ED_UTF8) {
        if (nLen == -1) {
            strcpy_safe(strOut, nOut, str);
            return strlen(strOut);
        } else {
            strncpysz_safe(strOut, nOut, str, nLen);
            return nLen;
        }
    }

    emptyStr(strOut);

    if (encodingID > __MaxEncodings || encodingID < 0) {
        encodingID = 0;
    }
    assert(encodingID == __encodingCodepage[encodingID].encodingID);

    JNIEnv *env;
    g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

    jstring jstr = newJString(env, str, nLen, __encodingCodepage[encodingID].szIConvCode);
    if (jstr != nullptr) {
        const char * utf8 = env->GetStringUTFChars(jstr, nullptr);

        strcpy_safe(strOut, nOut, utf8);

        env->ReleaseStringUTFChars(jstr, utf8);
        env->DeleteLocalRef(jstr);
    }

    return strlen(strOut);
}

int utf8ToMbcs(const char *str, int nLen, char *strOut, int nOut, int encodingID) {
    if (encodingID == ED_UTF8) {
        if (nLen == -1) {
            strcpy_safe(strOut, nOut, str);
            return strlen(strOut);
        } else {
            strncpysz_safe(strOut, nOut, str, nLen);
            return nLen;
        }
    }

    emptyStr(strOut);

    if (encodingID > __MaxEncodings || encodingID < 0) {
        encodingID = 0;
    }
    assert(encodingID == __encodingCodepage[encodingID].encodingID);

    string strW;

    utf8ToUCS2(str, nLen, strW);

    return ucs2ToMbcs(strW.c_str(), strW.size(), strOut, nOut, encodingID);
}

int mbcsToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID) {
    emptyStr(strOut);

    if (encodingID > __MaxEncodings || encodingID < 0) {
        encodingID = 0;
    }
    assert(encodingID == __encodingCodepage[encodingID].encodingID);

    JNIEnv *env;
    g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

    jstring jstr = newJString(env, str, nLen, __encodingCodepage[encodingID].szIConvCode);
    if (jstr != nullptr) {
        const jchar * ucs2 = env->GetStringChars(jstr, nullptr);

        wcscpy_safe(strOut, nOut, ucs2);

        env->ReleaseStringChars(jstr, ucs2);

        env->DeleteLocalRef(jstr);
    }

    return wcslen(strOut);
}

int ucs2ToMbcs(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID) {
    emptyStr(strOut);
    if (encodingID > __MaxEncodings || encodingID < 0) {
        encodingID = 0;
    }
    assert(encodingID == __encodingCodepage[encodingID].encodingID);

    JNIEnv *env;
    g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

    jstring jstr = env->newString(str, nLen);

    return getBytesOfJString(env, jstr, strOut, nOut, __encodingCodepage[encodingID].szIConvCode);
}
