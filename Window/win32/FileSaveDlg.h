#pragma once

class CFileSaveDlg
{
public:
    CFileSaveDlg(cstr_t szTitle, cstr_t szFile, cstr_t szFilter, int nDefFileType);
    virtual ~CFileSaveDlg(void);

    int doModal(Window *pWndParent);

    cstr_t getSaveFile();

    cstr_t getSelectedExt();

public:
    OPENFILENAME    m_openfile;
    char            m_szFile[MAX_PATH];

};
