// ShellNotifyIcon.cpp: implementation of the CShellNotifyIcon class.
//
//////////////////////////////////////////////////////////////////////

#include "ShellNotifyIcon.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShellNotifyIcon::CShellNotifyIcon()
{

}

CShellNotifyIcon::~CShellNotifyIcon()
{

}

bool CShellNotifyIcon::addIcon(Window *pWnd, uint32_t nID, cstr_t szTip, HICON hIcon, uint32_t nCallbackMessage)
{
    NOTIFYICONDATA        nif;

    nif.cbSize = sizeof(nif);
    nif.hWnd = pWnd->getHandle();
    nif.hIcon = hIcon;
    strcpy_safe(nif.szTip, CountOf(nif.szTip), szTip);
    nif.uID = nID;
    nif.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nif.uCallbackMessage = nCallbackMessage;

    return tobool(::Shell_NotifyIcon(NIM_ADD, &nif));
}

bool CShellNotifyIcon::delIcon(Window *pWnd, uint32_t nID)
{
    NOTIFYICONDATA        nif;

    nif.cbSize = sizeof(nif);
    nif.uID = nID;
    nif.hWnd = pWnd->getHandle();
    nif.uFlags = 0;

    return tobool(::Shell_NotifyIcon(NIM_DELETE, &nif));
}

bool CShellNotifyIcon::modifyIcon(Window *pWnd, uint32_t nID, cstr_t szTip, HICON hIcon)
{
    NOTIFYICONDATA        nif;

    nif.cbSize = sizeof(nif);
    nif.hWnd = pWnd->getHandle();
    if (szTip)
        strcpy_safe(nif.szTip, CountOf(nif.szTip), szTip);
    nif.uID = nID;
    nif.hIcon = hIcon;
    nif.uFlags = 0;
    if (szTip)
        nif.uFlags |= NIF_TIP;
    if (hIcon)
        nif.uFlags |= NIF_ICON;

    return tobool(::Shell_NotifyIcon(NIM_MODIFY, &nif));
}
