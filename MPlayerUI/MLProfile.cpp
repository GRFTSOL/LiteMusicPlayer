/********************************************************************
    Created  :    2002/01/04    21:30
    FileName :    MLProfile.cpp
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#include "../Skin/Skin.h"
#include "MLProfile.h"

#define CHAR_WORKING_FOLDER            '-'

void analyseProxySetting(cstr_t szProxySetting, char szServer[], int nMaxSize, int &nPort);

//
//取得IE 的代理设置
//    else if (nHttpProxyType == HTTP_PROXY_IE)
//    {
//        // 取得IE 的代理设置
//        // [HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings]
//        // REG_SZ: "ProxyServer"="127.1.1.1:80"
//        // ftp=localhost:80;gopher=localhost:80;http=localhost:80;https=localhost:80
//        HKEY    hKeyInet;
//
//        if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
//            0, KEY_QUERY_VALUE, &hKeyInet) == ERROR_SUCCESS)
//        {
//            uint32_t        nSizeProxy = sizeof(szProxy);
//            uint32_t        dwType = REG_SZ;
//            if (RegQueryValueEx(hKeyInet, "ProxyServer", nullptr, &dwType, (uint8_t *)szProxy, &nSizeProxy) != ERROR_SUCCESS)
//            {
//                ERR_LOG0("RegQueryValueEx: ProxyServer FAILED!");
//            }
//            else
//            {
//                // strstr(szProxy, "http=")
//            }
//            RegCloseKey(hKeyInet);
//        }
//        else
//            ERR_LOG0("RegOpenKeyEx: [HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings] FAILED!");
//    }


CMLProfile::CMLProfile()
{
}

CMLProfile::~CMLProfile()
{

}

bool CMLProfile::inetIsUseProxy()
{
    return (g_profile.getInt("ProxyType", HTTP_PROXY_NONE) != HTTP_PROXY_NONE);
}

bool CMLProfile::inetGetProxy(string &serverOut, int &nPort)
{
    int        nHttpProxyType;

    nPort = __HTTP_PORT;

    // 代理类型？
    nHttpProxyType = g_profile.getInt("ProxyType", HTTP_PROXY_NONE);
    if (nHttpProxyType == HTTP_PROXY_OURS) {
        // 我们自己的代理设置
        // 代理服务器地址
        nPort = g_profile.getInt("ProxyPort", __HTTP_PORT);
        if (nPort <= 0)
            nPort = __HTTP_PORT;
        serverOut = g_profile.getString("ProxyServer", "");

        return !serverOut.empty();
    }

    return false;
}

cstr_t CMLProfile::inetGetBase64ProxyUserPass()
{
    // return "";
    return g_profile.getString("Base64ProxyUserPass", "");
}


bool CMLProfile::writeDir(cstr_t szSectName, cstr_t szKeyName, cstr_t szDir)
{
    if (szDir[0] == getAppResourceDir()[0] && szDir[1] == ':')
    {
        // Lyrics folder is the same drive of Current working folder
        string        str = szDir;
        str[0] = CHAR_WORKING_FOLDER;
        return g_profile.writeString(szSectName, szKeyName, str.c_str());
    }
    else
        return g_profile.writeString(szSectName, szKeyName, szDir);
}


string CMLProfile::getDir(cstr_t szSectName, cstr_t szKeyName, cstr_t szDefDir)
{
    string        strDir;

    strDir = g_profile.getString(szSectName, szKeyName, szDefDir);

    if (strDir.size() >= 2 && strDir[0] == CHAR_WORKING_FOLDER && strDir[1] == ':')
        strDir[0] = getAppResourceDir()[0];

    return strDir;
}

