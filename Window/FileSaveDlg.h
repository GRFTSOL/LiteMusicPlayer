#pragma once

class CFileSaveDlg {
public:
    CFileSaveDlg(cstr_t title, cstr_t file, cstr_t extFilter, int nDefFileType);
    virtual ~CFileSaveDlg(void);

    int doModal(Window *pWndParent);

    cstr_t getSaveFile() { return m_file.c_str(); }

public:
    string                      m_title;
    cstr_t                      m_extFilter;
    int                         m_nDefFileType;
    string                      m_file;
    VecStrings                  m_vExts;

};
