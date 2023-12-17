/********************************************************************
    Created  :    2002/02/04    23:08
    FileName :    FolderDialog.cpp
    Author   :    xhy

    Purpose  :    浏览选择文件夹的类
*********************************************************************/

#import <Cocoa/Cocoa.h>
#include "../WindowTypes.h"
#include "Window.h"
#include "../FolderDialog.h"


bool CFolderDialog::doBrowse(Window *pWndParent) {
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

    // Enable the selection of files in the dialog.
    [openDlg setCanChooseFiles:NO];

    // Enable the selection of directories in the dialog.
    [openDlg setCanChooseDirectories:YES];

    if ([openDlg runModal] == NSModalResponseOK) {
        NSArray* files = [openDlg filenames];

        // Loop through all the files and process them.
        for(int i = 0; i < [files count]; i++ ) {
            NSString* fileName = [files objectAtIndex:i];

            m_path = [fileName UTF8String];
            break;
        }

        return true;
    }

    return false;
}
