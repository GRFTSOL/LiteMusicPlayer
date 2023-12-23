#include "../WindowLib.h"
#include <commdlg.h>
#include <array>
#include <cderr.h>
#include <Dlgs.h>


// COMMENT:
//        取得 GetSaveFileName 中lpstrFilter 的第nIndex个扩展名。
// INPUT:
//        szFilters    :    "All supported files (*.lrc; *.txt; *.snc)\0*.lrc;*.txt;*.snc\0LRC Lyrics File (*.lrc)\0*.LRC\0Text File (*.txt)\0*.TXT\0Snc File (*.snc)\0*.snc\0\0"
cwstr_t getFileFilterExtByIndex(cwstr_t szFilters, int nIndex) {
    assert(nIndex >= 1);
    auto szReturn = szFilters;

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
        LPOFNOTIFYW ofnotify = (LPOFNOTIFY)lParam;
        if (ofnotify) {
            if (ofnotify->hdr.code == CDN_TYPECHANGE) {
                auto ofn = ofnotify->lpOFN;
                cwstr_t newExt = getFileFilterExtByIndex(ofn->lpstrFilter, ofn->nFilterIndex);
                if (ofn->nFilterIndex > 1 && newExt) {
                    utf16_t filename[MAX_PATH] = { 0 };

                    GetDlgItemTextW(ofnotify->hdr.hwndFrom, cmb13, filename, MAX_PATH);
                    string name = ucs2ToUtf8(filename);
                    fileSetExt(name, ucs2ToUtf8(newExt).c_str());
                    SetDlgItemTextW(ofnotify->hdr.hwndFrom, cmb13, 
                        utf8ToUCS2(name.c_str()).c_str());
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
    OPENFILENAMEW openfile;

    utf16string u16ExtFilter = utf8ToUCS2(m_extFilter, getMultiStrLength(m_extFilter));
    utf16string title = utf8ToUCS2(m_title.c_str(), m_title.size());
    utf16string u16Fn = utf8ToUCS2(m_title.c_str(), m_title.size());
    u16Fn.resize(MAX_PATH);

    memset(&openfile, 0, sizeof(openfile));
    openfile.lpstrTitle = title.c_str();
    openfile.lpstrFilter = u16ExtFilter.c_str();
    openfile.lpstrFile = (utf16_t *)u16Fn.data();
    openfile.nMaxFile = m_file.size();
    openfile.nFilterIndex = m_nDefFileType;

    openfile.lStructSize = sizeof(openfile);
    openfile.hInstance = getAppInstance();
    openfile.Flags = OFN_OVERWRITEPROMPT | OFN_ENABLEHOOK | OFN_EXPLORER;
    openfile.lpfnHook = (LPOFNHOOKPROC)MyOFNHookProc;

    openfile.hwndOwner = pWndParent->getWndHandle();
    if (GetSaveFileName(&openfile)) {
        m_file.resize(strlen(m_file.c_str()));

        auto ext = getFileFilterExtByIndex(u16ExtFilter.c_str(), openfile.nFilterIndex);
        if (ext) {
            m_selectedExt = ucs2ToUtf8(ext);
        } else {
            m_selectedExt.clear();
        }

        return IDOK;
    } else {
        return IDCANCEL;
    }
}
