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

VecStrings extractOpenFiles(bool isMulSel, const utf16_t *text) {
    VecStrings files;

    if (!isMulSel) {
        files.push_back(ucs2ToUtf8(text));
        return files;
    }

    string strDir = ucs2ToUtf8(text);
    dirStringAddSep(strDir);

    while (*text != '\0') {
        text++;
    }
    text++;

    while (*text) {
        files.push_back(strDir + ucs2ToUtf8(text));
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
    OPENFILENAMEW openfile;
    std::array<utf16_t, 1024 * 16> buf;

    if (!m_vFiles.empty()) {
        memcpy(buf.data(), m_vFiles[0].c_str(), m_vFiles[0].size() + 1);
    }

    utf16string u16Title = utf8ToUCS2(m_title.c_str(), m_title.size());
    utf16string u16ExtFilter = utf8ToUCS2(m_extFilter, getMultiStrLength(m_extFilter));

    memset(&openfile, 0, sizeof(openfile));
    openfile.lStructSize = sizeof(openfile);
    openfile.lpstrTitle = u16Title.c_str();
    openfile.lpstrFilter = u16ExtFilter.c_str();
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
            openfile.lpstrFile[0] = '\0';
            openfile.lpstrFile[1] = '\0';
            if (GetOpenFileName(&openfile)) {
                m_vFiles = extractOpenFiles(m_bAllowMultiSel, buf.data());
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
