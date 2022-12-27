#pragma once

class CFileDlgExtFilter : public string {
public:
    void addExtention(cstr_t szDesc, cstr_t szExt);

};

class CFileOpenDlg {
public:
    CFileOpenDlg(cstr_t szTitle, cstr_t szFile, cstr_t extFilter, int nDefFileType, bool bAllowMultiSel = false);
    virtual ~CFileOpenDlg();

#ifdef _NO_WIGET_LIB
    int doModal(HWND hWndParent);
#else
    int doModal(Window *pWndParent);
#endif

    cstr_t getOpenFile();

    void getOpenFile(vector<string> &vFiles);

public:
    OPENFILENAME                m_openfile;
    char                        *            m_szFile;
    int                         m_nBufLen;
    bool                        m_bAllowMultiSel;

};
