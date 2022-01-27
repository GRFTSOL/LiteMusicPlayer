#include "UtilsTypes.h"
#include "StringEx.h"
#include "FileApi.h"


uint32_t GetPrivateProfileString(cstr_t lpAppName, cstr_t lpKeyName, cstr_t lpDefault, char * lpReturnedString, uint32_t nSize, cstr_t lpFileName)
{
    char        szBuff[1024];
    size_t            nAppName;
    size_t            nKeyName;
    bool        bFoundAppName = false;
    int            n;

    FILE *fp = fopen(lpFileName, "r");
    if (!fp)
        goto R_FAILED;

    nAppName = strlen(lpAppName);
    nKeyName = strlen(lpKeyName);

    //
    // 查找 AppName
    while (fgets(szBuff, CountOf(szBuff), fp))
    {
        cstr_t        szBeg = szBuff;
        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        if (*szBeg != '[')
            continue;
        szBeg++;

        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        if (strncasecmp(lpAppName, szBeg, nAppName) != 0)
            continue;
        szBeg += nAppName;

        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        if (*szBeg != ']')
            continue;
        szBeg++;

        bFoundAppName = true;

        break;
    }

    if (!bFoundAppName)
        goto R_FAILED;

    //
    // 查找 KeyName
    while (fgets(szBuff, CountOf(szBuff), fp))
    {
        cstr_t        szBeg = szBuff;
        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        if (*szBeg == '[')
            goto R_FAILED;

        if (strncasecmp(lpKeyName, szBeg, nKeyName) != 0)
            continue;
        szBeg += nKeyName;

        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        if (*szBeg != '=')
            continue;
        szBeg++;

        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        strcpy_safe(lpReturnedString, nSize, szBeg);

        n = strlen(lpReturnedString);

        while (n > 0 && (lpReturnedString[n - 1] == '\r' || lpReturnedString[n - 1] == '\n'))
            n--;

        lpReturnedString[n] = '\0';

        if (fp)
            fclose(fp);

        return n;
    }

R_FAILED:
    if (fp)
        fclose(fp);

    strcpy_safe(lpReturnedString, nSize, lpDefault);

    return strlen(lpReturnedString);
}


uint32_t GetPrivateProfileInt(cstr_t lpAppName, cstr_t lpKeyName, int nDefault, cstr_t lpFileName)
{
    char    szValue[64];
    char    szDefault[64];
    sprintf(szDefault, "%d", nDefault);

    GetPrivateProfileString(lpAppName, lpKeyName, szDefault, szValue, 64, lpFileName);

    return atoi(szValue);
}


bool replacePrivateProfileStr(string &buff, const char *lpAppName, const char *lpKeyName, const char *lpString)
{
    const char    *szBeg;
    size_t            nAppName;
    size_t            nKeyName;
    bool        bFoundAppName = false;
    char    szBuff[512];

    nAppName = strlen(lpAppName);
    nKeyName = strlen(lpKeyName);

    szBeg = buff.c_str();

    for (szBeg = buff.c_str(); szBeg != nullptr; szBeg = strchr(szBeg, '\n'))
    {
        while (*szBeg == '\r' || *szBeg == '\n')
            szBeg++;

        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        if (*szBeg != '[')
            continue;
        szBeg++;

        if (strncasecmp(lpAppName, szBeg, nAppName) != 0)
            continue;
        szBeg += nAppName;

        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        if (*szBeg != ']')
            continue;
        szBeg++;

        bFoundAppName = true;

        break;
    }

    if (!bFoundAppName)
    {
        if (buff.size())
        {
            if (buff[int(buff.size() -1)] != '\n')
            {
                buff.append("\r\n", 2);
            }
        }
        // append at end.
        sprintf(szBuff, "[%s]\r\n", lpAppName);
        buff += szBuff;
        sprintf(szBuff, "%s=%s\r\n", lpKeyName, lpString);
        buff += szBuff;

        return true;
    }

    //
    // 查找 KeyName
    for (; szBeg != nullptr; szBeg = strchr(szBeg, '\n'))
    {
        szBeg = strchr(szBeg, '\n');
        while (*szBeg == '\r' || *szBeg == '\n')
            szBeg++;

        const char*        szInsert = szBeg;
        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        if (*szBeg == '[')
        {
            sprintf(szBuff, "%s=%s\r\n", lpKeyName, lpString);
            buff.insert(int(szInsert - buff.c_str()), szBuff, strlen(szBuff));
            return true;
        }

        if (strncasecmp(lpKeyName, szBeg, nKeyName) != 0)
            continue;
        szBeg += nKeyName;

        while (*szBeg == ' ' || *szBeg == '\t')
            szBeg++;

        if (*szBeg != '=')
            continue;
        szBeg++;
        szInsert = szBeg;

        while (*szBeg && *szBeg != '\r' && *szBeg != '\n')
            szBeg++;

        buff.replace(int(szInsert - buff.c_str()), int(szBeg - szInsert), lpString, strlen(lpString));

        return true;
    }

    // the key name doesn't exist in this section.
    if (buff[int(buff.size() -1)] != '\n')
    {
        buff.append("\r\n", 2);
    }
    sprintf(szBuff, "%s=%s\r\n", lpKeyName, lpString);
    buff += szBuff;

    return true;
}

bool WritePrivateProfileString(cstr_t appName, cstr_t keyName, cstr_t value, cstr_t fileName)
{
    // Unicode isn't being supported.
    string buff;

    if (isFileExist(fileName) && !readFile(fileName, buff))
        return false;

    replacePrivateProfileStr(buff, appName, keyName, value);

    return saveDataAsFile(fileName, buff.c_str(), (int)buff.size());
}
