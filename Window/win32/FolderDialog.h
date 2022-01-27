/********************************************************************
    Created  :    2002/02/04    23:08
    FileName :    FolderDialog.h
    Author   :    xhy
    
    Purpose  :    浏览选择文件夹的类
*********************************************************************/

#if !defined(AFX_FOLDERDIALOG_H__4E319E02_19C3_11D6_B478_FFFFFF000000__INCLUDED_)
#define AFX_FOLDERDIALOG_H__4E319E02_19C3_11D6_B478_FFFFFF000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFolderDialog  
{
public:
    void setRootFolder(cstr_t szRootFolder);
    void setTitle(cstr_t szTitle);
    void setInitFolder(cstr_t szInitDir);
    cstr_t getFolder();
    bool doBrowse(Window *pWndParent);
    CFolderDialog();
    virtual ~CFolderDialog();

    string        m_strInitFolder;
    string        m_strRootFolder;
    string        m_strTitle;
    char        m_szPath[MAX_PATH];

};

#endif // !defined(AFX_FOLDERDIALOG_H__4E319E02_19C3_11D6_B478_FFFFFF000000__INCLUDED_)
