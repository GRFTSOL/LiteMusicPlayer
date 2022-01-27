// Helper.cpp: implementation of the Helper class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "Helper.h"

void analyseProxySetting(cstr_t szProxySetting, char szServer[], int nMaxSize, int &nPort);

void execute(Window *pWnd, cstr_t szExe, cstr_t szParam)
{
    // TODO: TBD
}

bool setClipboardText(Window *pWnd, cstr_t szText)
{
    return tobool(copyTextToClipboard(szText));
}

bool SHDeleteFile(cstr_t szFile, Window *pWndParent)
{
    return deleteFile(szFile);
}

bool SHCopyFile(cstr_t szSrcFile, cstr_t szTargFile, Window *pWndParent)
{
    return copyFile(szSrcFile, szTargFile, true);
}

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES    0xFFFFFFFF
#endif

bool setFileNoReadOnly(cstr_t szFile)
{
    return true;
}

bool loadProxySvrFromIE(bool &bUseProxy, string &strSvr, int &nPort)
{
    return false;
}

static bool findStringValue(cstr_t szBuff, cstr_t szName, cstr_t szEndTag, string &strValue)
{
    cstr_t        szBeg, szEnd;

    strValue.resize(0);

    szBeg = strstr(szBuff, szName);
    if (!szBeg)
        return false;

    szBeg += strlen(szName);

    szEnd = strstr(szBeg, szEndTag);
    if (!szEnd)
        return false;

    strValue.append(szBeg, szEnd);

    return true;
}

bool loadProxySvrFromFireFox(bool &bUseProxy, string &strSvr, int &nPort)
{
    return false;
}


void getNotepadEditor(string &strEditor)
{
}

uint32_t getSecCount()
{
    return getTickCount() / 1000;
}
