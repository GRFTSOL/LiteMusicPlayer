// Helper.h: interface for the Helper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPER_H__12EAEFEA_BAAC_49DE_8ABD_28FF06B85835__INCLUDED_)
#define AFX_HELPER_H__12EAEFEA_BAAC_49DE_8ABD_28FF06B85835__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#endif // !defined(AFX_HELPER_H__12EAEFEA_BAAC_49DE_8ABD_28FF06B85835__INCLUDED_)
