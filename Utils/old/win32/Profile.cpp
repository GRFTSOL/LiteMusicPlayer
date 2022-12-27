/********************************************************************
    Created  :    2002/01/04    21:30
    FileName :    Profile.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "Profile.h"
#include "stdio.h"
#include "uuencode.h"
#include "app.h"


/*
test Encrypt ....
int testEncryptProfile()
{
    CProfile    file;
    file.init("c:\\a.ini", nullptr, "test");

    char        szReturn[256];
    string    str;

    // read k1
    file.encryptGetString("k1", "k1default", szReturn, CountOf(szReturn));
    str = file.encryptGetString("k1", "k1default2");

    // write k1
    file.encryptWriteString("k1", "k1value");

    // read k1
    file.encryptGetString("k1", "k1default", szReturn, CountOf(szReturn));
    str = file.encryptGetString("k1", "k1default2");

    return 1;
}

int k = testEncryptProfile();*/

bool writePrivateProfileInt(
    cstr_t lpAppName,  // section name
    cstr_t lpKeyName,  // key name
    int value,            // int to add
    cstr_t lpFileName  // initialization file
    ) {
    char szBuffer[16];

    _itot_s(value, szBuffer, CountOf(szBuffer), 10);
    return writePrivateProfileString(lpAppName, lpKeyName, szBuffer, lpFileName);
}

uint32_t regGetProfileString(HKEY hRoot, cstr_t szItem, cstr_t szKeyName, cstr_t szDefault, char * szReturnedString, uint32_t nSize) {
    HKEY hKey;

    emptyStr(szReturnedString);

    if (RegOpenKeyEx(hRoot, szItem,
        0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        uint32_t nSizeRet = nSize * sizeof(char);
        uint32_t dwType = REG_SZ;
        if (RegQueryValueEx(hKey, szKeyName, nullptr, &dwType, (uint8_t *)szReturnedString, &nSizeRet) != ERROR_SUCCESS) {
            strcpy_safe(szReturnedString,nSize, szDefault);
        } else {
            nSizeRet /= sizeof(char);
            if (nSizeRet >= nSize) {
                nSizeRet = nSize - 1;
            }
            szReturnedString[nSizeRet] = '\0';
            if (dwType != REG_SZ && dwType != REG_EXPAND_SZ) {
                if (dwType == REG_DWORD) {
                    _itot_s(regGetProfileInt(hRoot, szItem, szKeyName, atoi(szDefault)),
                        szReturnedString, nSize, 10);
                } else {
                    assert(0);
                }
            }
        }
        RegCloseKey(hKey);
    } else {
        strcpy_safe(szReturnedString, nSize, szDefault);
    }

    return strlen(szReturnedString);
}

bool regWriteProfileString(HKEY hRoot, cstr_t szItem, cstr_t szKeyName, cstr_t szValue) {
    HKEY hKey;
    uint32_t dwDispositon;
    bool bRet = false;

    if (RegCreateKeyEx(hRoot, szItem,
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hKey, &dwDispositon) == ERROR_SUCCESS) {
        uint32_t dwType = REG_SZ;
        if (RegSetValueEx(hKey, szKeyName, 0, dwType, (uint8_t *)szValue, lstrlen(szValue) * sizeof(char)) == ERROR_SUCCESS) {
            bRet = true;
        }
        RegCloseKey(hKey);
    }

    return bRet;
}

int regGetProfileInt(HKEY hRoot, cstr_t szSubKey, cstr_t szKeyName, int nDefault) {
    HKEY hKey;
    int value = nDefault;

    if (RegOpenKeyEx(hRoot, szSubKey,
        0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        uint32_t nSizeRet = sizeof(nDefault);
        uint32_t dwType = REG_DWORD;
        if (RegQueryValueEx(hKey, szKeyName, nullptr, &dwType, (uint8_t *)&value, &nSizeRet) != ERROR_SUCCESS) {
            // LOG1(LOG_LVL_ERROR, "RegQueryValueEx: %s, FAILED!", szKeyName);
        } else if (dwType == REG_SZ) {
            char szValue[256];
            uint32_t nSizeRet = CountOf(szValue) * sizeof(char);
            uint32_t dwType = REG_SZ;
            if (RegQueryValueEx(hKey, szKeyName, nullptr, &dwType, (uint8_t *)szValue, &nSizeRet) != ERROR_SUCCESS) {
                value = nDefault;
            } else {
                nSizeRet /= sizeof(char);
                if (nSizeRet >= CountOf(szValue)) {
                    nSizeRet = CountOf(szValue) - 1;
                }
                szValue[nSizeRet] = '\0';
                value = atoi(szValue);
            }
        } else {
            assert(dwType == REG_DWORD);
        }
        RegCloseKey(hKey);
    }

    return value;
}

bool regWriteProfileInt(HKEY hRoot, cstr_t szSubKey, cstr_t szKeyName, int value) {
    HKEY hKey;
    uint32_t dwDispositon;
    bool bRet = false;

    if (RegCreateKeyEx(hRoot, szSubKey,
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hKey, &dwDispositon) == ERROR_SUCCESS) {
        uint32_t dwType = REG_DWORD;
        if (RegSetValueEx(hKey, szKeyName, 0, dwType, (uint8_t *)&value, sizeof(value)) == ERROR_SUCCESS) {
            bRet = true;
        }
        RegCloseKey(hKey);
    }

    return bRet;
}


void regSetKeyDefValueIfNotExist(HKEY hRoot, cstr_t szItem, cstr_t szKeyName, cstr_t szDefault) {
    HKEY hKey;

    if (RegOpenKeyEx(hRoot, szItem,
        0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        uint32_t nSizeRet = 0;
        uint32_t dwType = REG_SZ;

        if (RegQueryValueEx(hKey, szKeyName, nullptr, &dwType, nullptr, &nSizeRet) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            goto R_FAILED;
        }
        RegCloseKey(hKey);
    } else {
        goto R_FAILED;
    }

R_FAILED:
    regWriteProfileString(hRoot, szItem, szKeyName, szDefault);
}

void regSetKeyDefValueIfNotExist(HKEY hRoot, cstr_t szItem, cstr_t szKeyName, int nDefault) {
    HKEY hKey;

    if (RegOpenKeyEx(hRoot, szItem,
        0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        uint32_t nSizeRet = 0;
        uint32_t dwType = REG_DWORD;

        if (RegQueryValueEx(hKey, szKeyName, nullptr, &dwType, nullptr, &nSizeRet) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            goto R_FAILED;
        }
        RegCloseKey(hKey);
    } else {
        goto R_FAILED;
    }

R_FAILED:
    regWriteProfileInt(hRoot, szItem, szKeyName, nDefault);
}


CProfile::CProfile() {
    for (int i = 0; i < RET_STR_MAX; i++) {
        m_szSetStrings[i] = nullptr;
    }

    m_hKeyRoot = nullptr;
    m_nCurRetStr = 0;
}

CProfile::~CProfile() {
    close();
}

//
// 区分大小写，将szAppName, szKey添加到缓存中
void CProfile::addKeys(cstr_t szAppName, cstr_t szKeyName, cstr_t szValue) {
    ListSections::iterator itApp;
    APP_SECT *appSec = nullptr;
    string strKeyNameLower;

    // find AppName
    for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++) {
        appSec = *itApp;
        if (strcasecmp(appSec->strAppName.c_str(), szAppName) == 0) {
            break;
        }
    }
    if (appSec == nullptr || itApp == m_listAppSect.end()) {
        // append new app secton
        appSec = new APP_SECT;
        appSec->strAppName = szAppName;
        m_listAppSect.push_back(appSec);
    }

    strKeyNameLower = szKeyName;
    lowerString((char *)strKeyNameLower.c_str());
    appSec->mapKeys[strKeyNameLower] = szValue;
}

bool CProfile::getKey(cstr_t szAppName, cstr_t szKeyName, MapStrings::iterator &it) {
    ListSections::iterator itApp;
    APP_SECT *appSec = nullptr;
    string strKeyNameLower;

    strKeyNameLower = szKeyName;
    lowerString((char *)strKeyNameLower.c_str());

    // 查找AppName
    for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++) {
        appSec = *itApp;
        if (strcasecmp(appSec->strAppName.c_str(), szAppName) == 0) {
            break;
        }
    }
    if (appSec == nullptr || itApp == m_listAppSect.end()) {
        return false;
    }

    //
    // 查找KeyName
    it = appSec->mapKeys.find(strKeyNameLower);
    if (it == appSec->mapKeys.end()) {
        return false;
    }

    return true;
}

void CProfile::setKeyDefaultIfNotExist(cstr_t szAppName, cstr_t szKeyName, cstr_t szValue) {
    ListSections::iterator itApp;
    APP_SECT *appSec = nullptr;
    string strKeyNameLower;
    MapStrings::iterator it;

    strKeyNameLower = szKeyName;
    lowerString((char *)strKeyNameLower.c_str());

    // 查找AppName
    for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++) {
        appSec = *itApp;
        if (strcasecmp(appSec->strAppName.c_str(), szAppName) == 0) {
            break;
        }
    }
    if (appSec == nullptr || itApp == m_listAppSect.end()) {
        // append new app secton
        appSec = new APP_SECT;
        appSec->strAppName = szAppName;
        m_listAppSect.push_back(appSec);
        appSec->mapKeys[strKeyNameLower] = szValue;
        return;
    }

    it = appSec->mapKeys.find(strKeyNameLower);
    if (it == appSec->mapKeys.end()) {
        appSec->mapKeys[strKeyNameLower] = szValue;
    }
}

void CProfile::init(cstr_t szProfile, cstr_t szDefKey) {
    close();

    if (szDefKey == nullptr) {
        m_strDefAppName = "";
    } else {
        m_strDefAppName = szDefKey;
    }

#define HKEY_LEN            5
    if (strncmp(szProfile, "HKEY_", HKEY_LEN) == 0) {
        /*HKEY_CLASSES_ROOT
        HKEY_CURRENT_CONFIG
        HKEY_CURRENT_USER
        HKEY_LOCAL_MACHINE
        HKEY_USERS*/
        int nLenRootKey;

        // uses registry
        if (strncmp(szProfile + HKEY_LEN, "LOCAL_MACHINE",
            nLenRootKey = strlen("LOCAL_MACHINE")) == 0) {
            m_hKeyRoot = HKEY_LOCAL_MACHINE;
        } else if (strncmp(szProfile + HKEY_LEN, "CURRENT_USER",
            nLenRootKey = strlen("CURRENT_USER")) == 0) {
            m_hKeyRoot = HKEY_CURRENT_USER;
        } else if (strncmp(szProfile + HKEY_LEN, "USERS",
            nLenRootKey = strlen("USERS")) == 0) {
            m_hKeyRoot = HKEY_USERS;
        } else if (strncmp(szProfile + HKEY_LEN, "CLASSES_ROOT",
            nLenRootKey = strlen("CLASSES_ROOT")) == 0) {
            m_hKeyRoot = HKEY_CLASSES_ROOT;
        } else if (strncmp(szProfile + HKEY_LEN, "CURRENT_CONFIG",
            nLenRootKey = strlen("CURRENT_CONFIG")) == 0) {
            m_hKeyRoot = HKEY_CURRENT_CONFIG;
        } else {
            m_hKeyRoot = nullptr;
        }

        if (m_hKeyRoot) {
            m_nCurRetStr = 0;

            if (szProfile[HKEY_LEN + nLenRootKey] == '\\') {
                nLenRootKey++;
            }

            m_strProfile = szProfile + HKEY_LEN + nLenRootKey;
            dirStringAddSep(m_strProfile);

            for (int i = 0; i < RET_STR_MAX; i++) {
                m_szSetStrings[i] = new char[RET_STR_LEN];
            }
            return;
        }
    }

    if (strchr(szProfile, PATH_SEP_CHAR) != nullptr) {
        // It might be full file path
        m_strProfile = szProfile;
        return;
    }

    m_strProfile = getAppDataDir();
    m_strProfile += szProfile;
}

void CProfile::close() {
    m_hKeyRoot = nullptr;

    m_strDefAppName = "";
    m_strProfile = "";

    ListSections::iterator itApp;
    APP_SECT *appSec;

    // 查找AppName
    for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++) {
        appSec = *itApp;
        delete appSec;
    }
    m_listAppSect.clear();

    for (int i = 0; i < RET_STR_MAX; i++) {
        if (m_szSetStrings[i]) {
            delete []m_szSetStrings[i];
            m_szSetStrings[i] = nullptr;
        }
    }
}

static void profileKeyValueMultiStrToVStr(cstr_t szText, vector<string> &vStr) {
    while (*szText) {
        cstr_t szBeg;
        szBeg = szText;
        while (*szText && *szText != '=') {
            szText++;
        }
        if (*szText == '=') {
            string str;
            str.append(szBeg, szText);

            szText++;
            vStr.push_back(str);
            vStr.push_back(szText);
            while (*szText != '\0') {
                szText++;
            }
            szText++;
        } else {
            szText++;
        }
    }
}

void CProfile::doCache() {
    if (m_hKeyRoot) {
        return;
    }

    string str;
    cstr_t szBeg, szEnd;
    string strAppName, strKey, strValue;

    if (!readFileByBom(m_strProfile.c_str(), str)) {
        return;
    }

    //
    // 查找 AppName
    szBeg = str.c_str();
    while (*szBeg) {
        while (*szBeg == ' ' || *szBeg == '\t' || *szBeg == '\n') {
            szBeg++;
        }

        if (*szBeg != '[') {
            while (*szBeg && *szBeg != '\n') {
                szBeg++;
            }
            continue;
        }
        szBeg++;

        while (*szBeg == ' ' || *szBeg == '\t') {
            szBeg++;
        }

        szEnd = szBeg;
        while (*szEnd && *szEnd != ']' && *szEnd != '\n') {
            szEnd++;
        }

        if (*szEnd != ']') {
            while (*szBeg && *szBeg != '\n') {
                szBeg++;
            }
            continue;
        }

        // found appname now
        strAppName = "";
        strAppName.append(szBeg, szEnd);
        trimStr(strAppName);

        szBeg = szEnd + 1;

        while (*szBeg) {
            // get key and values
            while (*szBeg == ' ' || *szBeg == '\t') {
                szBeg++;
            }

            while (*szBeg == '\r' || *szBeg == '\n') {
                szBeg++;
            }

            if (*szBeg == '[') {
                break;
            }

            szEnd = szBeg;
            while (*szEnd && *szEnd != '=' && *szEnd != '\n') {
                szEnd++;
            }

            if (*szEnd != '=') {
                szBeg = szEnd;
                while (*szBeg && *szBeg != '\n') {
                    szBeg++;
                }
                continue;
            }

            // found key
            strKey = "";
            strKey.append(szBeg, szEnd);

            szBeg = szEnd + 1;
            while (*szEnd && *szEnd != '\n') {
                szEnd++;
            }

            while (*szEnd == '\r' || *szEnd == '\n') {
                szEnd--;
            }
            if (*szEnd) {
                szEnd++;
            }
            strValue = "";
            strValue.append(szBeg, szEnd);

            addKeys(strAppName.c_str(), strKey.c_str(), strValue.c_str());

            szBeg = szEnd;
        }
    }
}

void CProfile::setDefaultIfNotExist(cstr_t szAppName, cstr_t szKeyName, cstr_t szDefault) {
    if (m_hKeyRoot) {
        string strSubKey;

        strSubKey = m_strProfile + szAppName;

        regSetKeyDefValueIfNotExist(m_hKeyRoot, strSubKey.c_str(), szKeyName, szDefault);
    } else {
        setKeyDefaultIfNotExist(szAppName, szKeyName, szDefault);
    }
}

void CProfile::setDefaultIfNotExist(cstr_t szAppName, cstr_t szKeyName, int nDefault) {
    if (m_hKeyRoot) {
        string strSubKey;

        strSubKey = m_strProfile + szAppName;

        regSetKeyDefValueIfNotExist(m_hKeyRoot, strSubKey.c_str(), szKeyName, nDefault);
    } else {
        char szDefault[64];
        _itot_s(nDefault, szDefault, CountOf(szDefault), 10);

        setKeyDefaultIfNotExist(szAppName, szKeyName, szDefault);
    }
}

uint32_t CProfile::getString(cstr_t szAppName, cstr_t szKeyName, cstr_t szDefault, char * szReturnedString, uint32_t nSize) {
    assert(!m_strProfile.empty());

    if (m_hKeyRoot) {
        string strSubKey;

        strSubKey = m_strProfile + szAppName;

        return regGetProfileString(m_hKeyRoot, strSubKey.c_str(), szKeyName, szDefault, szReturnedString, nSize);
    }

    uint32_t dwRet;

    if (szKeyName[0] == '\0') {
        emptyStr(szReturnedString);
        return 0;
    }

    MapStrings::iterator itKey;

    if (!getKey(szAppName, szKeyName, itKey)) {
        dwRet = getPrivateProfileString(szAppName, szKeyName, szDefault, szReturnedString, nSize, m_strProfile.c_str());
        addKeys(szAppName, szKeyName, szReturnedString);
        return dwRet;
    } else {
        strcpy_safe(szReturnedString, nSize, (*itKey).second.c_str());

        return (*itKey).second.size();
    }
}

cstr_t CProfile::getString(cstr_t szAppName, cstr_t szKeyName, cstr_t szDefault) {
    assert(!m_strProfile.empty());

    if (m_hKeyRoot) {
        string strSubKey;

        strSubKey = m_strProfile + szAppName;

        m_nCurRetStr++;
        if (m_nCurRetStr >= RET_STR_MAX) {
            m_nCurRetStr = 0;
        }

        regGetProfileString(m_hKeyRoot, strSubKey.c_str(), szKeyName, szDefault, m_szSetStrings[m_nCurRetStr], RET_STR_LEN);
        return m_szSetStrings[m_nCurRetStr];
    }

    uint32_t dwRet;

    if (szKeyName[0] == '\0') {
        return "";
    }

    MapStrings::iterator itKey;

    if (!getKey(szAppName, szKeyName, itKey)) {
        char szReturnedString[1024];
        dwRet = getPrivateProfileString(szAppName, szKeyName, szDefault, szReturnedString, 1024, m_strProfile.c_str());
        addKeys(szAppName, szKeyName, szReturnedString);
        if (!getKey(szAppName, szKeyName, itKey)) {
            assert(0 && "getKey Can't be FAILED!");
            return "";
        }
    }

    return (*itKey).second.c_str();
}

bool CProfile::writeString(cstr_t szAppName, cstr_t szKeyName, cstr_t szValue) {
    assert(!m_strProfile.empty());

    if (m_hKeyRoot) {
        string strSubKey;

        strSubKey = m_strProfile + szAppName;

        return regWriteProfileString(m_hKeyRoot, strSubKey.c_str(), szKeyName, szValue);
    }

    uint32_t dwRet;
    MapStrings::iterator itKey;

    dwRet = writePrivateProfileString(szAppName, szKeyName, szValue, m_strProfile.c_str());

    if (!getKey(szAppName, szKeyName, itKey)) {
        addKeys(szAppName, szKeyName, szValue);
    } else {
        (*itKey).second = szValue;
    }

    return dwRet;
}


uint32_t CProfile::getInt(cstr_t szAppName, cstr_t szKeyName, int nDefault) {
    assert(!m_strProfile.empty());

    if (m_hKeyRoot) {
        string strSubKey;

        strSubKey = m_strProfile + szAppName;

        return regGetProfileInt(m_hKeyRoot, strSubKey.c_str(), szKeyName, nDefault);
    }

    if (szKeyName[0] == '\0') {
        return nDefault;
    }

    char szValue[64];
    char szDefault[64];
    _itot_s(nDefault, szDefault, CountOf(szDefault), 10);

    getString(szAppName, szKeyName, szDefault, szValue, 64);

    return atoi(szValue);
}

bool CProfile::writeInt(cstr_t szAppName, cstr_t szKeyName, int value) {
    assert(!m_strProfile.empty());

    if (m_hKeyRoot) {
        string strSubKey;

        strSubKey = m_strProfile + szAppName;

        return regWriteProfileInt(m_hKeyRoot, strSubKey.c_str(), szKeyName, value);
    }

    char szValue[64];

    _itot_s(value, szValue, CountOf(szValue), 10);

    return writeString(szAppName, szKeyName, szValue);
}


cstr_t CProfile::getFile() {
    return m_strProfile.c_str();
}

cstr_t CProfile::getDefAppName() {
    return m_strDefAppName.c_str();
}

//
// support encrypt keyname and value
//
void encryptStr(cstr_t szStr, string &strOut) {
    strOut = base64Encode(szStr, strlen(szStr));
}

void decryptStr(cstr_t szStr, string &strOut) {
    string str = szStr;
    strrep(str, '?', '=');
    strOut = base64Decode(str.c_str(), str.size());
}
/*
void testEnDecrypt()
{
    char        szString[] = "012345adjkl;ge;lk561./zx/,.ckl";
    string    str, strOut, strOut2;

    for (int i = 0; i < 20; i++)
    {
        str.clear();
        str.append(szString, i);
        encryptStr(str.c_str(), strOut);
        decryptStr(strOut.c_str(), strOut2);
        assert(strcmp(str.c_str(), strOut2.c_str()) == 0);
    }
}
*/

string CProfile::encryptGetString(cstr_t szAppName, cstr_t szKeyName, cstr_t szDefault) {
    assert(!m_strProfile.empty());
    assert(szDefault);

    uint32_t dwRet;

    if (szKeyName[0] == '\0') {
        return szDefault;
    }

    string strKeyNameEncrypt, strValue;
    encryptStr(szKeyName, strKeyNameEncrypt);

    if (m_hKeyRoot) {
        string strSubKey;

        strSubKey = m_strProfile + szAppName;

        encryptStr(szDefault, strValue);

        m_nCurRetStr++;
        if (m_nCurRetStr >= RET_STR_MAX) {
            m_nCurRetStr = 0;
        }
        dwRet = regGetProfileString(m_hKeyRoot, strSubKey.c_str(), strKeyNameEncrypt.c_str(), strValue.c_str(), m_szSetStrings[m_nCurRetStr], RET_STR_LEN);

        decryptStr(m_szSetStrings[m_nCurRetStr], strValue);

        return strValue;
    }

    MapStrings::iterator itKey;

    char szReturnedString[1024];
    if (!getKey(szAppName, strKeyNameEncrypt.c_str(), itKey)) {
        encryptStr(szDefault, strValue);
        dwRet = getPrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str(), szReturnedString, 1024, m_strProfile.c_str());
        addKeys(szAppName, strKeyNameEncrypt.c_str(), szReturnedString);
    } else {
        strcpy_safe(szReturnedString, 1024, (*itKey).second.c_str());
    }
    decryptStr(szReturnedString, strValue);

    return strValue;
}

bool CProfile::encryptWriteString(cstr_t szAppName, cstr_t szKeyName, cstr_t szValue) {
    assert(!m_strProfile.empty());

    uint32_t dwRet;
    MapStrings::iterator itKey;
    string strKeyNameEncrypt, strValueEncrypt;

    encryptStr(szKeyName, strKeyNameEncrypt);
    encryptStr(szValue, strValueEncrypt);

    if (m_hKeyRoot) {
        string strSubKey;

        strSubKey = m_strProfile + szAppName;

        return regWriteProfileString(m_hKeyRoot, strSubKey.c_str(), strKeyNameEncrypt.c_str(), strValueEncrypt.c_str());
    }

    dwRet = writePrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValueEncrypt.c_str(), m_strProfile.c_str());

    if (!getKey(szAppName, strKeyNameEncrypt.c_str(), itKey)) {
        addKeys(szAppName, strKeyNameEncrypt.c_str(), szValue);
    } else {
        (*itKey).second = strValueEncrypt;
    }

    return dwRet;
}

int CProfile::encryptGetInt(cstr_t szKeyName, int value) {
    string str = encryptGetString(szKeyName, "");
    if (str.empty()) {
        return value;
    } else {
        return atoi(str.c_str());
    }
}

void CProfile::encryptWriteInt(cstr_t szKeyName, int value) {
    encryptWriteString(szKeyName, stringPrintf("%d", value).c_str());
}
