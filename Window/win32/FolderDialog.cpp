/********************************************************************
    Created  :    2002/02/04    23:08
    FileName :    FolderDialog.cpp
    Author   :    xhy

    Purpose  :    浏览选择文件夹的类
*********************************************************************/

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT	0x0500

#include <shlobj.h>
#include "../WindowLib.h"


int CALLBACK BrowseCallbackProc(HWND hwnd,uint32_t uMsg,LPARAM lp, LPARAM pData) {
    char szDir[MAX_PATH];

    switch (uMsg) {
    case BFFM_INITIALIZED:
        {
            CFolderDialog *pDlg = (CFolderDialog *)pData;
            assert(pDlg);

            if (pDlg && (!pDlg->m_initFolder.empty())) {
                // WParam is true since you are passing a path.
                // It would be false if you were passing a pidl.
                SendMessage(hwnd, BFFM_SETSELECTION, true, (LPARAM)pDlg->m_initFolder.c_str());
            }
            break;
        }
    case BFFM_SELCHANGED:
        {
            // set the status window to the currently selected path.
            if (SHGetPathFromIDList((LPITEMIDLIST) lp, szDir)) {
                SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
            }
            break;
        }
    default:
        break;
    }
    return 0;
}


bool CFolderDialog::doBrowse(Window *pWndParent) {
    BROWSEINFO info;

    ZeroMemory((PVOID)&info, sizeof(info));
    info.hwndOwner = pWndParent->getWndHandle();

    char pathBuf[MAX_PATH] = {0};
    info.pszDisplayName = pathBuf;
    if (m_title.empty()) {
        info.lpszTitle = _TLT("Choose Folder");
    }

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE  0x0040
#endif

    info.ulFlags = /*BIF_EDITBOX | */BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;//BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;

    info.lpfn = BrowseCallbackProc; // address of callback function.
    info.lParam = (LPARAM)this; // pass address of object to callback function

    LPITEMIDLIST pidl = ::SHBrowseForFolder(&info);
    if (pidl == nullptr) {
        return false;
    }

    if (::SHGetPathFromIDList(pidl, pathBuf) == false) {
        return false;
    }
    m_path = pathBuf;

    return true;
}
