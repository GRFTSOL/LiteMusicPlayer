#include "Thread.h"


CThread::CThread() {
    m_funThead = nullptr;
    m_hThread = nullptr;
    m_lpData = nullptr;
}

CThread::~CThread() {
    if(m_hThread) {
        assert(!isRunning());
        CloseHandle(m_hThread);
    }
}

bool CThread::create(FUNThread function, void* lpData) {
    uint32_t dwThreadId;
    m_funThead = function;
    m_lpData = lpData;

    if (m_hThread) {
        assert(!isRunning());
        CloseHandle(m_hThread);
    }

    m_hThread = ::CreateThread(nullptr,
        0,
        threadFunction,
        this,
        0,
        &dwThreadId);
    if(m_hThread) {
        return true;
    }

    return false;
}

void CThread::destroy() {
    if (m_hThread) {
        TerminateThread(m_hThread, 0);
    }
}

void CThread::suspend() {
    if (m_hThread) {
        SuspendThread(m_hThread);
    }
}

void CThread::resume() {
    if (m_hThread) {
        ResumeThread(m_hThread);
    }
}

void CThread::join() {
    if (m_hThread) {
        WaitForSingleObject(m_hThread, INFINITE);
    }
}

uint32_t CThread::getPriority() const {
    if (m_hThread) {
        return getThreadPriority(m_hThread);
    } else {
        return 0;
    }
}

uint32_t CThread::setPriority(uint32_t priority) {
    if (!m_hThread) {
        return 0;
    }

    uint32_t nOld;

    nOld = getThreadPriority(m_hThread);

    SetThreadPriority(m_hThread, priority);

    return nOld;
}

bool CThread::isRunning() {
    if (m_hThread) {
        if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
            return true;
        }
    }

    return false;
}

uint32_t WINAPI CThread::threadFunction(void *lpParam) {
    CThread *pwin32Thread;
    pwin32Thread = (CThread *)lpParam;

    pwin32Thread->m_funThead(pwin32Thread->m_lpData);

    return 0;
}
