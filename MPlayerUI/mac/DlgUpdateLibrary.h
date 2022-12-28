#pragma once

class CUpdateLibraryObj {
public:
    CUpdateLibraryObj();
    ~CUpdateLibraryObj();

    virtual void doUpdating() = 0;

    void createWorkThread(Window *hWndNotify);

    static void workThread(void *lpParam);

    void setMsg1(cstr_t szMsg);
    void setMsg2(cstr_t szMsg);

protected:
    CThread                     m_thread;
    Window                      *m_pWnd;

};

class CAddMediaObj : public CUpdateLibraryObj {
public:
    CAddMediaObj() {
        m_bAddFiles = false;
    }
    virtual void doUpdating();

    bool                        m_bAddFiles;
    string                      m_strDir;
    vector<string>              m_vFiles;

protected:
    void listMedia(cstr_t szDir);

};


class CDlgUpdateLibrary {
public:
    CDlgUpdateLibrary();
    virtual ~CDlgUpdateLibrary();

    bool doModal(Window *pWndParent, CUpdateLibraryObj *pWorkObj);

protected:
    CUpdateLibraryObj           *m_pWorkObj;
    bool                        m_bCanQuit;

};
