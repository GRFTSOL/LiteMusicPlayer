/********************************************************************
    Created  :    2002/02/04    23:08
    FileName :    FolderDialog.h
    Author   :    xhy

    Purpose  :    浏览选择文件夹的类
*********************************************************************/

#pragma once



class CFolderDialog {
public:
    CFolderDialog(cstr_t initFolder, cstr_t title = "") : m_initFolder(initFolder), m_title(title) {
        if (m_initFolder.size() > 0 && m_initFolder[m_initFolder.size() - 1] == PATH_SEP_CHAR) {
            // Remove ending '/'
            m_initFolder.resize(m_initFolder.size() - 1);
        }
    }
    virtual ~CFolderDialog() { }

    bool doBrowse(Window *pWndParent);
    cstr_t getFolder() { return m_path.c_str(); }

public:
    string                      m_initFolder;
    string                      m_title;
    string                      m_path;

};

