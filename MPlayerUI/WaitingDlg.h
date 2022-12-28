#pragma once

#ifndef MPlayerUI_WaitingDlg_h
#define MPlayerUI_WaitingDlg_h


#define UMSG_WORK_END       1

class CWorkObjBase {
public:
    CWorkObjBase();
    virtual ~CWorkObjBase();

    virtual uint32_t doTheJob();

    virtual void cancelJob() { m_bJobCanceled = true; }

    void createWorkThread(Window *hWndNotify);

    static void workThread(void *lpParam);

    bool isCancelEnabled() { return m_bEnableCancel; }

    bool isJobCanceled() { return m_bJobCanceled; }

protected:
    Window                      *m_hWndNotify;
    CThread                     m_threadWork;
    bool                        m_bEnableCancel;
    bool                        m_bJobCanceled;

};


class CPageWaitMessage : public CSkinContainer {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
    CPageWaitMessage();

    void startWork(cstr_t szCaption, cstr_t szInfo, CWorkObjBase *pWorkObj);

    void onSwitchTo() override;

    bool onOK() override;

    bool onCancel() override;

    virtual void onWorkEnd();

    bool onUserMessage(int nMessageID, LPARAM param) override;

protected:
    CWorkObjBase                *m_pWorkObj;

};

#endif // !defined(MPlayerUI_WaitingDlg_h)
