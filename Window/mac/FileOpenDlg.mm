#import <Cocoa/Cocoa.h>
#include "WindowTypes.h"
#include "Window.h"
#include "FileOpenDlg.h"


void CFileDlgExtFilter::addExtention(cstr_t szDesc, cstr_t szExt) {
    append(szDesc);
    append(1, char('\0'));

    append(szExt);
    append(1, char('\0'));
}

void extractExtFilters(cstr_t extFilter, VecStrings &vExts) {
    VecStrings vStr;
    multiStrToVStr(extFilter, vStr);

    for (uint32_t i = 0; i < vStr.size(); i++) {
        cstr_t p = vStr[i].c_str();
        while (p != nullptr) {
            cstr_t beg = strchr(p, '.');
            if (beg == nullptr) {
                break;
            }
            beg++;
            cstr_t end = beg;
            while (isAlphaDigit(*end)) {
                end++;
            }

            string str(beg, int(end - beg));
            str = toLower(str.c_str());
            if (!isInArray(str, vExts)) {
                vExts.push_back(str);
            }
            p = end;
        }
    }
}

NSArray *toNsStringArray(VecStrings &vExts) {
    if (vExts.empty()) {
        return nil;
    }

    NSMutableArray *ext = [[NSMutableArray alloc] init];
    for (auto &name : vExts) {
        NSString *s = [NSString stringWithUTF8String:name.c_str()];
        [ext addObject:s];
    }

    return ext;
}

//////////////////////////////////////////////////////////////////////

CFileOpenDlg::CFileOpenDlg(cstr_t szTitle, cstr_t szFile, cstr_t extFilter, int nDefFileType, bool bAllowMultiSel) {
    m_bAllowMultiSel = bAllowMultiSel;

    if (szFile) {
        m_vFiles.push_back(szFile);
    }

    if (extFilter) {
        extractExtFilters(extFilter, m_vExts);
    }
}

CFileOpenDlg::~CFileOpenDlg() {
}

int CFileOpenDlg::doModal(Window *pWndParent) {
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

    // Enable the selection of files in the dialog.
    [openDlg setCanChooseFiles:YES];

    // Enable the selection of directories in the dialog.
    [openDlg setCanChooseDirectories:NO];

    [openDlg setAllowsMultipleSelection: m_bAllowMultiSel ? YES : NO];

    // [openDlg setDirectoryURL:[NSURL URLWithString:[NSString stringWithUTF8String:m_szFile]]];

    [openDlg setAllowedFileTypes:toNsStringArray(m_vExts)];

    m_vFiles.clear();

    if ([openDlg runModal] == NSModalResponseOK) {
        NSArray* files = [openDlg filenames];

        // Loop through all the files and process them.
        for(int i = 0; i < [files count]; i++ ) {
            NSString* fileName = [files objectAtIndex:i];
            m_vFiles.push_back([fileName UTF8String]);
        }

        return IDOK;
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
