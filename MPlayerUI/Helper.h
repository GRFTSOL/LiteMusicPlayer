#pragma once

#ifndef MPlayerUI_Helper_h
#define MPlayerUI_Helper_h


#include "../Window/WindowLib.h"


void execute(Window *pWnd, cstr_t szExe, cstr_t szParam);

bool setClipboardText(Window *pWnd, cstr_t szText);

cstr_t httpErrorCodeToStr(int nCode);

bool SHDeleteFile(cstr_t szFile, Window *pWndParent);

bool SHCopyFile(cstr_t szSrcFile, cstr_t szTargFile, Window *pWndParent);

bool setFileNoReadOnly(cstr_t szFile);

bool loadProxySvrFromIE(bool &bUseProxy, string &strSvr, int &nPort);

void getNotepadEditor(string &strEditor);

void profileGetColorValue(COLORREF &clr, cstr_t szSectName, cstr_t szKeyName);

void profileGetColorValue(CColor &clr, cstr_t szSectName, cstr_t szKeyName);

#endif // !defined(MPlayerUI_Helper_h)
