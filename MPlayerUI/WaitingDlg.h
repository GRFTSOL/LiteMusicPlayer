// WaitingDlg.h: interface for the CWaitingDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAITINGDLG_H__8E8BBE95_0587_4DEB_9DAF_7B3769589439__INCLUDED_)
#define AFX_WAITINGDLG_H__8E8BBE95_0587_4DEB_9DAF_7B3769589439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define UMSG_WORK_END        1

class CWorkObjBase
{
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
    Window        *m_hWndNotify;
    CThread            m_threadWork;
    bool            m_bEnableCancel;
    bool            m_bJobCanceled;

};


class CPageWaitMessage : public CSkinContainer
{
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
    CWorkObjBase    *m_pWorkObj;

};

#endif // !defined(AFX_WAITINGDLG_H__8E8BBE95_0587_4DEB_9DAF_7B3769589439__INCLUDED_)
