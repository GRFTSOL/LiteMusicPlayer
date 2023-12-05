#pragma once



class CFileDlgExtFilter : public string {
public:
    void addExtention(cstr_t szDesc, cstr_t szExt);

};

class CFileOpenDlg {
public:
    CFileOpenDlg(cstr_t title, cstr_t file, cstr_t extFilter, int nDefFileType, bool bAllowMultiSel = false);
    virtual ~CFileOpenDlg();

#ifdef _NO_WIGET_LIB
    int doModal(HWND hWndParent);
#else
    int doModal(Window *pWndParent);
#endif

    cstr_t getOpenFile();

    void getOpenFile(vector<string> &vFiles);

public:
    string                      m_title;
    cstr_t                      m_extFilter;
    int                         m_nDefFileType;
    VecStrings                  m_vFiles;
    bool                        m_bAllowMultiSel;
    VecStrings                  m_vExts;

};
