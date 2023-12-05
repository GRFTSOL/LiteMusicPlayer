#include "../WindowLib.h"
#include <commdlg.h>
#include <array>
#include <cderr.h>


void CFileDlgExtFilter::addExtention(cstr_t szDesc, cstr_t szExt) {
    append(szDesc);
    append(1, char('\0'));

    append(szExt);
    append(1, char('\0'));
}

VecStrings extractOpenFiles(bool isMulSel, const char *text) {
    VecStrings files;

    if (!isMulSel) {
        files.push_back(text);
        return files;
    }

    string strDir = text;
    dirStringAddSep(strDir);

    while (*text != '\0') {
        text++;
    }
    text++;

    while (*text) {
        files.push_back(strDir + text);
        while (*text != '\0') {
            text++;
        }
        text++;
    }

    return files;
}
//////////////////////////////////////////////////////////////////////

CFileOpenDlg::CFileOpenDlg(cstr_t title, cstr_t file, cstr_t extFilter, int nDefFileType, bool bAllowMultiSel) {
    m_title = title;
    m_extFilter = extFilter;
    m_nDefFileType = nDefFileType;
    m_bAllowMultiSel = bAllowMultiSel;

    if (file) {
        m_vFiles.push_back(file);
    }
}

CFileOpenDlg::~CFileOpenDlg() {
}

int CFileOpenDlg::doModal(Window *pWndParent) {
    OPENFILENAME openfile;
    std::array<char, 1024 * 16> buf;

    if (!m_vFiles.empty()) {
        memcpy(buf.data(), m_vFiles[0].c_str(), m_vFiles[0].size() + 1);
    }

    memset(&openfile, 0, sizeof(openfile));
    openfile.lStructSize = sizeof(openfile);
    openfile.lpstrTitle = m_title.c_str();
    openfile.lpstrFilter = m_extFilter;
    openfile.lpstrFile = buf.data();
    openfile.nMaxFile = buf.size();
    openfile.nFilterIndex = m_nDefFileType;

    openfile.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
    if (m_bAllowMultiSel) {
        openfile.Flags |= OFN_ALLOWMULTISELECT;
    }
    openfile.hInstance = getAppInstance();
    openfile.hwndOwner = pWndParent->getWndHandle();

    if (GetOpenFileName(&openfile)) {
        extractOpenFiles(m_bAllowMultiSel, buf.data());
        return IDOK;
    } else {
        uint32_t dwErr = CommDlgExtendedError();
        if (dwErr == FNERR_INVALIDFILENAME) {
            emptyStr(openfile.lpstrFile);
            if (GetOpenFileName(&openfile)) {
                extractOpenFiles(m_bAllowMultiSel, buf.data());
                return IDOK;
            }
        }
    }

    return IDCANCEL;
}

cstr_t CFileOpenDlg::getOpenFile() {
    if (m_vFiles.size() > 0) {
        return m_vFiles[0].c_str();
    } else {
        return "";
    }
}

void CFileOpenDlg::getOpenFile(vector<string> &vFiles) {
    vFiles = m_vFiles;
}
