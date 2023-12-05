#include "../WindowLib.h"
#include <commdlg.h>
#include <array>
#include <cderr.h>
#include <Dlgs.h>


// COMMENT:
//        取得 GetSaveFileName 中lpstrFilter 的第nIndex个扩展名。
// INPUT:
//        szFilters    :    "All supported files (*.lrc; *.txt; *.snc)\0*.lrc;*.txt;*.snc\0LRC Lyrics File (*.lrc)\0*.LRC\0Text File (*.txt)\0*.TXT\0Snc File (*.snc)\0*.snc\0\0"
cstr_t getFileFilterExtByIndex(cstr_t szFilters, int nIndex) {
    assert(nIndex >= 1);
    cstr_t szReturn;

    szReturn = szFilters;

    //
    // 找到第nIndex个filter
    for (int i = 0; i < nIndex * 2 - 1; i++) {
        while (*szReturn != '\0') {
            szReturn++;
        }
        szReturn++;
        if (*szReturn == '\0') {
            return nullptr;
        }
    }

    // 移动直到扩展名
    while (*szReturn != '.' && *szReturn != '\0') {
        szReturn++;
    }

    return szReturn;
}

UINT_PTR CALLBACK MyOFNHookProc(
    HWND hdlg,      // handle to dialog box
    uint32_t uiMsg,      // message identifier
    WPARAM wParam,  // message parameter
    LPARAM lParam   // message parameter
    ) {
    switch (uiMsg) {
    case WM_NOTIFY:
        LPOFNOTIFY ofnotify = (LPOFNOTIFY)lParam;
        if (ofnotify) {
            if (ofnotify->hdr.code == CDN_TYPECHANGE) {
                auto ofn = ofnotify->lpOFN;
                cstr_t newExt = getFileFilterExtByIndex(ofn->lpstrFilter, ofn->nFilterIndex);
                if (ofn->nFilterIndex > 1 && newExt) {
                    char filename[MAX_PATH] = { 0 };

                    GetDlgItemText(ofnotify->hdr.hwndFrom, cmb13, filename, MAX_PATH);
                    string name = filename;
                    fileSetExt(name, newExt);
                    SetDlgItemText(ofnotify->hdr.hwndFrom, cmb13, name.c_str());
                }
            }
            return 1;
        }
        break;
    }
    return 0;
}

CFileSaveDlg::CFileSaveDlg(cstr_t title, cstr_t file, cstr_t extFilter, int nDefFileType) {
    m_title = title;
    m_file = file;
    m_extFilter = extFilter;
    m_nDefFileType = nDefFileType;
}

CFileSaveDlg::~CFileSaveDlg(void) {
}

int CFileSaveDlg::doModal(Window *pWndParent) {
    OPENFILENAME openfile;
    m_file.resize(MAX_PATH);

    memset(&openfile, 0, sizeof(openfile));
    openfile.lpstrTitle = m_title.c_str();
    openfile.lpstrFilter = m_extFilter;
    openfile.lpstrFile = (char *)m_file.data();
    openfile.nMaxFile = m_file.size();
    openfile.nFilterIndex = m_nDefFileType;

    openfile.lStructSize = sizeof(openfile);
    openfile.hInstance = getAppInstance();
    openfile.Flags = OFN_OVERWRITEPROMPT | OFN_ENABLEHOOK | OFN_EXPLORER;
    openfile.lpfnHook = (LPOFNHOOKPROC)MyOFNHookProc;

    openfile.hwndOwner = pWndParent->getWndHandle();
    if (GetSaveFileName(&openfile)) {
        m_file.resize(strlen(m_file.c_str()));

        cstr_t ext = getFileFilterExtByIndex(m_extFilter, openfile.nFilterIndex);
        if (ext) {
            m_selectedExt = ext;
        } else {
            m_selectedExt.clear();
        }

        return IDOK;
    } else {
        return IDCANCEL;
    }
}
