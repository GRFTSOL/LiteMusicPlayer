/********************************************************************
    Created  :    2002/02/04    23:08
    FileName :    FolderDialog.cpp
    Author   :    xhy
    
    Purpose  :    浏览选择文件夹的类
*********************************************************************/

#import <Cocoa/Cocoa.h>
#include "WindowTypes.h"
#include "FolderDialog.h"

CFolderDialog::CFolderDialog()
{
    m_strInitFolder = "";
    m_strRootFolder = "";
    m_strTitle = "";
    m_szPath[0] = '\0';
}

CFolderDialog::~CFolderDialog()
{
}

bool CFolderDialog::doBrowse(Window *pWndParent)
{
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    
    // Enable the selection of files in the dialog.
    [openDlg setCanChooseFiles:NO];
    
    // Enable the selection of directories in the dialog.
    [openDlg setCanChooseDirectories:YES];
    
    if ([openDlg runModal] == NSModalResponseOK)
    {
        NSArray* files = [openDlg filenames];
        
        // Loop through all the files and process them.
        for(int i = 0; i < [files count]; i++ )
        {
            NSString* fileName = [files objectAtIndex:i];
            
            strcpy_safe(m_szPath, CountOf(m_szPath), [fileName UTF8String]);
            break;
        }
        
        return true;
    }

    return false;
}

cstr_t CFolderDialog::getFolder()
{
    return m_szPath;
}

void CFolderDialog::setInitFolder(cstr_t szInitDir)
{
    m_strInitFolder = szInitDir;

    // 去掉目录后面的'\'
    if (m_strInitFolder.size() > 0 && m_strInitFolder[m_strInitFolder.size() - 1] == PATH_SEP_CHAR)
        m_strInitFolder[m_strInitFolder.size() - 1] = '\0';
}

void CFolderDialog::setTitle(cstr_t szTitle)
{
    m_strTitle = szTitle;
}

void CFolderDialog::setRootFolder(cstr_t szRootFolder)
{
    m_strRootFolder = szRootFolder;
}
