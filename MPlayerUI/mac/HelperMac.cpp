#include "../MPlayerApp.h"
#include "../Helper.h"


void analyseProxySetting(cstr_t szProxySetting, char szServer[], int nMaxSize, int &nPort);

void execute(Window *pWnd, cstr_t szExe, cstr_t szParam) {
    // TODO: TBD
}

bool setClipboardText(Window *pWnd, cstr_t szText) {
    return tobool(copyTextToClipboard(szText));
}

bool SHDeleteFile(cstr_t szFile, Window *pWndParent) {
    return deleteFile(szFile);
}

bool SHCopyFile(cstr_t szSrcFile, cstr_t szTargFile, Window *pWndParent) {
    return copyFile(szSrcFile, szTargFile, true);
}

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF
#endif

bool setFileNoReadOnly(cstr_t szFile) {
    return true;
}

bool loadProxySvrFromIE(bool &bUseProxy, string &strSvr, int &nPort) {
    return false;
}

void getNotepadEditor(string &strEditor) {
}
