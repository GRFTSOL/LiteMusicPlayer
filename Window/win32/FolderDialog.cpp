/********************************************************************
    Created  :    2002/02/04    23:08
    FileName :    FolderDialog.cpp
    Author   :    xhy

    Purpose  :    浏览选择文件夹的类
*********************************************************************/

#include "shlobj.h"
#include "BaseWnd.h"
#include "FolderDialog.h"


int CALLBACK BrowseCallbackProc(HWND hwnd,uint32_t uMsg,LPARAM lp, LPARAM pData) {
    char szDir[MAX_PATH];

    switch (uMsg) {
    case BFFM_INITIALIZED:
        {
            CFolderDialog *pDlg;

            pDlg = (CFolderDialog *)pData;
            assert(pDlg);

            if (pDlg && (!pDlg->m_strInitFolder.empty())) {
                // WParam is true since you are passing a path.
                // It would be false if you were passing a pidl.
                sendMessage(hwnd,BFFM_SETSELECTION,true,(LPARAM)pDlg->m_strInitFolder.c_str());
            }
            break;
        }
    case BFFM_SELCHANGED:
        {
            // set the status window to the currently selected path.
            if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir)) {
                sendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
            }
            break;
        }
    default:
        break;
    }
    return 0;
}



CFolderDialog::CFolderDialog() {
    m_strInitFolder = "";
    m_strRootFolder = "";
    m_strTitle = "";
    m_szPath[0] = '\0';
}

CFolderDialog::~CFolderDialog() {
}

bool CFolderDialog::doBrowse(Window *pWndParent) {
    LPMALLOC pMalloc;

    if (SHGetMalloc(&pMalloc) != NOERROR) {
        return false;
    }

    BROWSEINFO bInfo;
    LPITEMIDLIST pidl;

    ZeroMemory((PVOID)&bInfo, sizeof(bInfo));

    if (!m_strRootFolder.empty()) {
        OLECHAR olePath[MAX_PATH];
        ULONG chEaten;
        ULONG dwAttributes;
        HRESULT hr;
        LPSHELLFOLDER pDesktopFolder;

        // get a pointer to the Desktop's IShellFolder interface.
        if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder))) {
            // IShellFolder::ParseDisplayName requires the file name be in Unicode
#ifndef _UNICODE
            MultiByteToWideChar(CP_ACP,
                MB_PRECOMPOSED,
                m_strRootFolder.c_str(),
                -1,
                olePath,
                MAX_PATH
                );
#else
            wcscpy_safe(olePath, CountOf(olePath), m_strRootFolder.c_str());
#endif

            // Convert the path to ITEMIDLIST
            hr = pDesktopFolder->ParseDisplayName(nullptr,
                nullptr,
                olePath,
                &chEaten,
                &pidl,
                &dwAttributes
                );
            if (FAILED(hr)) {
                goto BR_FAILED;
            }
            bInfo.pidlRoot = pidl;
        }
    }

    bInfo.hwndOwner = pWndParent->getHandle();

    bInfo.pszDisplayName = m_szPath;
    if (m_strTitle.empty()) {
        bInfo.lpszTitle = _TLT("Choose Folder"); // STRING
    }

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE  0x0040
#endif
    bInfo.ulFlags = /*BIF_EDITBOX | */BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;//BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;

    bInfo.lpfn = BrowseCallbackProc; // address of callback function.
    bInfo.lParam = (LPARAM)this; // pass address of object to callback function

    pidl = ::SHBrowseForFolder(&bInfo);
    if (pidl == nullptr) {
        goto BR_FAILED;
    }

    //    m_iImageIndex = bInfo.iImage;

    if (::SHGetPathFromIDList(pidl, m_szPath) == false) {
        goto BR_FAILED;
    }

    pMalloc->free(pidl);
    pMalloc->release();
    return true;

BR_FAILED:
    if (pMalloc) {
        pMalloc->free(pidl);
        pMalloc->release();
    }
    return false;
}

cstr_t CFolderDialog::getFolder() {
    return m_szPath;
}

void CFolderDialog::setInitFolder(cstr_t szInitDir) {
    m_strInitFolder = szInitDir;

    // 去掉目录后面的\
    if (m_strInitFolder.size() > 0 && m_strInitFolder[m_strInitFolder.size() - 1] == PATH_SEP_CHAR) {
        m_strInitFolder[m_strInitFolder.size() - 1] = '\0';
    }
}

void CFolderDialog::setTitle(cstr_t szTitle) {
    m_strTitle = szTitle;
}

void CFolderDialog::setRootFolder(cstr_t szRootFolder) {
    m_strRootFolder = szRootFolder;
}
