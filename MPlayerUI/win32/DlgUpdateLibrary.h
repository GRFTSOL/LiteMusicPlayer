// DlgUpdateLibrary.h: interface for the CDlgUpdateLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGUPDATELIBRARY_H__9CCD0FD3_64C4_44E1_A22A_3E61F66E61C0__INCLUDED_)
#define AFX_DLGUPDATELIBRARY_H__9CCD0FD3_64C4_44E1_A22A_3E61F66E61C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUpdateLibraryObj
{
public:
    CUpdateLibraryObj();
    ~CUpdateLibraryObj();

    virtual void doUpdating() = 0;

    void createWorkThread(Window *hWndNotify);

    static void workThread(void *lpParam);

    void setMsg1(cstr_t szMsg);
    void setMsg2(cstr_t szMsg);

protected:
    CThread            m_thread;
    Window        *m_pWnd;

};

class CAddMediaObj : public CUpdateLibraryObj
{
public:
    CAddMediaObj()
    {
        m_bAddFiles = false;
    }
    virtual void doUpdating();

    bool                m_bAddFiles;
    string                m_strDir;
    vector<string>        m_vFiles;

protected:
    void listMedia(cstr_t szDir);

};


class CDlgUpdateLibrary : public CBaseDialog  
{
public:
    CDlgUpdateLibrary();
    virtual ~CDlgUpdateLibrary();

    bool onInitDialog();

    virtual void onCancel();
    virtual void onOK();

    bool doModal(Window *pWndParent, CUpdateLibraryObj *pWorkObj);

#ifdef _WIN32
    LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);
#endif

protected:
    CUpdateLibraryObj            *m_pWorkObj;
    bool                        m_bCanQuit;

};

#endif // !defined(AFX_DLGUPDATELIBRARY_H__9CCD0FD3_64C4_44E1_A22A_3E61F66E61C0__INCLUDED_)
