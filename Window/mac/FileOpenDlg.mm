#import <Cocoa/Cocoa.h>
#include "WindowTypes.h"
#include "FileOpenDlg.h"


void CFileDlgExtFilter::addExtention(cstr_t szDesc, cstr_t szExt) {
    append(szDesc);
    append(1, char('\0'));

    append(szExt);
    append(1, char('\0'));
}

void ExtFiltersToSet(cstr_t extFilter, SetStrings &setExts) {
    VecStrings vStr;
    multiStrToVStr(extFilter, vStr);

    string str;
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
            str.clear();
            str.append(beg, int(end - beg));
            setExts.insert(toLower(str.c_str()));
            p = end;
        }
    }
}

NSArray *SetStrToExtArray(SetStrings &setExt) {
    if (setExt.size() == 0) {
        return nil;
    }

    NSMutableArray *ext = [[NSMutableArray alloc] init];
    for (SetStrings::iterator it = setExt.begin(); it != setExt.end(); ++it) {
        NSString *s = [NSString stringWithUTF8String:(*it).c_str()];
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
        ExtFiltersToSet(extFilter, m_setExt);
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

    [openDlg setAllowedFileTypes:SetStrToExtArray(m_setExt)];

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
