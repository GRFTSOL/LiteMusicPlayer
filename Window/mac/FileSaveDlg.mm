#import <Cocoa/Cocoa.h>
#include "WindowTypes.h"
#include "Window.h"
#include "FileSaveDlg.h"


NSArray *toNsStringArray(VecStrings &setExt);

void extractExtFilters(cstr_t extFilter, VecStrings &vExts);

// COMMENT:
//        取得 GetSaveFileName 中lpstrFilter 的第nIndex个扩展名。
// INPUT:
//        szFilters    :    "All supported files (*.lrc; *.txt; *.snc)\0*.lrc;*.txt;*.snc\0LRC Lyrics File (*.lrc)\0*.LRC\0Text File (*.txt)\0*.TXT\0Snc File (*.snc)\0*.snc\0\0"
cstr_t GetFileFilterExtByIndex(cstr_t szFilters, int nIndex) {
    assert(nIndex >= 1);
    cstr_t szReturn;

    szReturn = szFilters;

    //
    // 找到第nIndex个Filter
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

CFileSaveDlg::CFileSaveDlg(cstr_t title, cstr_t file, cstr_t extFilter, int nDefFileType) {
    m_title = title;
    m_file = file;
    m_extFilter = extFilter;
    m_nDefFileType = nDefFileType;
    if (extFilter) {
        extractExtFilters(extFilter, m_vExts);
    }
}

CFileSaveDlg::~CFileSaveDlg(void) {
}

int CFileSaveDlg::doModal(Window *pWndParent) {
    NSSavePanel* saveDlg = [NSSavePanel savePanel];

    // Enable the selection of files in the dialog.
    [saveDlg setDirectoryURL:[NSURL URLWithString:[NSString stringWithUTF8String:m_file._cstr()]]];

    [saveDlg setAllowedFileTypes:toNsStringArray(m_vExts)];

    //[saveDlg set:YES];

    // Enable the selection of directories in the dialog.
    //[saveDlg setCanChooseDirectories:NO];

    if ([saveDlg runModal] == NSModalResponseOK) {
        NSString *fileName = [saveDlg filename];

        m_file.assign([fileName UTF8String]);

        return IDOK;
    }

    return IDCANCEL;
}
