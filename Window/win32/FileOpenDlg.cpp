#ifndef _NO_WIGET_LIB
#include "BaseWnd.h"
#endif
#include "FileOpenDlg.h"
#include <CdErr.h>


void CFileDlgExtFilter::addExtention(cstr_t szDesc, cstr_t szExt) {
    append(szDesc);
    append(1, char('\0'));

    append(szExt);
    append(1, char('\0'));
}

//////////////////////////////////////////////////////////////////////

CFileOpenDlg::CFileOpenDlg(cstr_t szTitle, cstr_t szFile, cstr_t extFilter, int nDefFileType, bool bAllowMultiSel) {
    if (bAllowMultiSel) {
        m_nBufLen = 1024 * 10;
    } else {
        m_nBufLen = MAX_PATH;
    }

    m_bAllowMultiSel = bAllowMultiSel;
    m_szFile = new char[m_nBufLen];
    emptyStr(m_szFile);

    if (szFile) {
        strcpy_safe(m_szFile, m_nBufLen, szFile);
    }

    memset(&m_openfile, 0, sizeof(m_openfile));
    m_openfile.lpstrTitle = szTitle;
    m_openfile.lpstrFilter = extFilter;
    m_openfile.lpstrFile = m_szFile;
    m_openfile.nMaxFile = m_nBufLen;
    m_openfile.nFilterIndex = nDefFileType;

    m_openfile.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
    if (bAllowMultiSel) {
        m_openfile.Flags |= OFN_ALLOWMULTISELECT;
    }
    m_openfile.lStructSize = sizeof(m_openfile);
    m_openfile.hInstance = getAppInstance();
}

CFileOpenDlg::~CFileOpenDlg() {
    delete[] m_szFile;
}

int CFileOpenDlg::doModal(Window *pWndParent) {
    m_openfile.hwndOwner = pWndParent->getHandle();

    if (getOpenFileName(&m_openfile)) {
        return IDOK;
    } else {
        uint32_t dwErr = CommDlgExtendedError();
        if (dwErr == FNERR_INVALIDFILENAME) {
            emptyStr(m_openfile.lpstrFile);
            if (getOpenFileName(&m_openfile)) {
                return IDOK;
            }
        }
    }
    return IDCANCEL;
}

cstr_t CFileOpenDlg::getOpenFile() {
    return m_szFile;
}

void CFileOpenDlg::getOpenFile(vector<string> &vFiles) {
    if (m_bAllowMultiSel) {
        cstr_t szText = m_szFile;
        string strDir;

        strDir = m_szFile;
        dirStringAddSep(strDir);

        while (*szText != '\0') {
            szText++;
        }
        szText++;

        while (*szText) {
            vFiles.push_back(strDir + szText);
            while (*szText != '\0') {
                szText++;
            }
            szText++;
        }

        if (vFiles.empty()) {
            vFiles.push_back(m_szFile);
        }
    } else {
        vFiles.push_back(m_szFile);
    }
}
