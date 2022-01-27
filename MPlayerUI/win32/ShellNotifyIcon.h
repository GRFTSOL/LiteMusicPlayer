// ShellNotifyIcon.h: interface for the CShellNotifyIcon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHELLNOTIFYICON_H__237180DB_EC4C_465C_8833_0476272F3281__INCLUDED_)
#define AFX_SHELLNOTIFYICON_H__237180DB_EC4C_465C_8833_0476272F3281__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CShellNotifyIcon
{
public:
    CShellNotifyIcon();
    ~CShellNotifyIcon();

    static bool addIcon(Window *pWnd, uint32_t nID, cstr_t szTip, HICON hIcon, uint32_t nCallbackMessage);
    static bool delIcon(Window *pWnd, uint32_t nID);
    static bool modifyIcon(Window *pWnd, uint32_t nID, cstr_t szTip, HICON hIcon = nullptr);

};

#endif // !defined(AFX_SHELLNOTIFYICON_H__237180DB_EC4C_465C_8833_0476272F3281__INCLUDED_)
