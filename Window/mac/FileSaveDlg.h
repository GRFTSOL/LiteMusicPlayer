#pragma once

#include "Window.h"


class CFileSaveDlg {
public:
    CFileSaveDlg(cstr_t szTitle, cstr_t szFile, cstr_t extFilter, int nDefFileType);
    virtual ~CFileSaveDlg(void);

    int doModal(Window *pWndParent);

    cstr_t getSaveFile();

    cstr_t getSelectedExt();

public:
    char                        m_szFile[MAX_PATH];
    VecStrings                  m_vExts;

};
