#include "MPlayerApp.h"
#include "WaitingDlg.h"
#include "DownloadMgr.h"
#include "Helper.h"


CWorkObjBase::CWorkObjBase() {
    m_hWndNotify = nullptr;
    m_bEnableCancel = false;
    m_bJobCanceled = false;
}

CWorkObjBase::~CWorkObjBase() {
    m_threadWork.join();
}

void CWorkObjBase::createWorkThread(Window *hWndNotify) {
    m_hWndNotify = hWndNotify;
    m_bJobCanceled = false;
    m_threadWork.create(workThread, this);
}

uint32_t CWorkObjBase::doTheJob() {
    return 0;
}

void CWorkObjBase::workThread(void *lpParam) {
    CWorkObjBase *pThis;

    pThis = (CWorkObjBase *)lpParam;
    pThis->doTheJob();

    pThis->m_hWndNotify->postUserMessage(UMSG_WORK_END, 0);
}

//////////////////////////////////////////////////////////////////////
UIOBJECT_CLASS_NAME_IMP(CPageWaitMessage, "Container.WaitMsg")


CPageWaitMessage::CPageWaitMessage() {
    m_pWorkObj = nullptr;
}

void CPageWaitMessage::startWork(cstr_t szCaption, cstr_t szInfo, CWorkObjBase *pWorkObj) {
    m_pSkin->setCaptionText(szCaption);
    setUIObjectText("CID_WAIT_INFO", szInfo, true);

    m_pWorkObj = pWorkObj;
    assert(pWorkObj);

    m_pContainer->switchToPage(getClassName(), false, 0, true);
}

void CPageWaitMessage::onSwitchTo() {
    CSkinContainer::onSwitchTo();

    if (m_pWorkObj) {
        m_pWorkObj->createWorkThread(m_pSkin);
        if (m_pWorkObj->isCancelEnabled()) {
            enableUIObject(ID_CANCEL, true);
        } else {
            enableUIObject(ID_CANCEL, false);
        }
    }
}

bool CPageWaitMessage::onOK() {
    return false;
}

bool CPageWaitMessage::onCancel() {
    if (m_pWorkObj && m_pWorkObj->isCancelEnabled()) {
        m_pWorkObj->cancelJob();
    }

    return false;
}

void CPageWaitMessage::onWorkEnd() {
    m_pWorkObj = nullptr;
    m_pContainer->switchToLastPage(0, true);
}

bool CPageWaitMessage::onUserMessage(int nMessageID, LPARAM param) {
    if (nMessageID == UMSG_WORK_END) {
        onWorkEnd();
        return true;
    } else {
        return CSkinContainer::onUserMessage(nMessageID, param);
    }
}
