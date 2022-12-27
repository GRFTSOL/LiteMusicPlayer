/********************************************************************
    Created  :    2002/02/04    23:08
    FileName :    FolderDialog.h
    Author   :    xhy

    Purpose  :    浏览选择文件夹的类
*********************************************************************/

#pragma once

#ifndef Window_win32_FolderDialog_h
#define Window_win32_FolderDialog_h


class CFolderDialog {
public:
    void setRootFolder(cstr_t szRootFolder);
    void setTitle(cstr_t szTitle);
    void setInitFolder(cstr_t szInitDir);
    cstr_t getFolder();
    bool doBrowse(Window *pWndParent);
    CFolderDialog();
    virtual ~CFolderDialog();

    string                      m_strInitFolder;
    string                      m_strRootFolder;
    string                      m_strTitle;
    char                        m_szPath[MAX_PATH];

};

#endif // !defined(Window_win32_FolderDialog_h)
