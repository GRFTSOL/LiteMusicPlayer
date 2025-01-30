#import <Cocoa/Cocoa.h>
#include "../WindowTypes.h"
#include "Window.h"
#include "../FileSaveDlg.h"


NSArray *toNsStringArray(VecStrings &setExt);

void extractExtFilters(cstr_t extFilter, VecStrings &vExts);

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
    [saveDlg setDirectoryURL:[NSURL URLWithString:[NSString stringWithUTF8String:m_file.c_str()]]];

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
