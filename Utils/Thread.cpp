#include "UtilsTypes.h"
#include "Thread.h"
#include <signal.h>


CThread::CThread() {
}

CThread::~CThread() {
}

#ifdef _WIN32

bool CThread::create(FUNThread function, void* lpData) {
    m_funThead = function;
    m_lpData = lpData;

    if (m_hThread) {
        assert(!isRunning());
        CloseHandle(m_hThread);
    }

    DWORD dwThreadId = 0;
    m_hThread = ::CreateThread(nullptr, 0, threadFunction, this, 0, &dwThreadId);
    if(m_hThread) {
        return true;
    }

    return false;
}

void CThread::destroy() {
    if (m_hThread) {
        TerminateThread(m_hThread, 0);
        m_hThread = NULL;
    }
}

void CThread::join() {
    if (m_hThread) {
        WaitForSingleObject(m_hThread, INFINITE);
    }
}

bool CThread::isRunning() {
    if (m_hThread) {
        if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
            return true;
        }
    }

    return false;
}

DWORD WINAPI CThread::threadFunction(void *lpParam) {
    CThread *thiz = (CThread *)lpParam;
    thiz->m_funThead(thiz->m_lpData);
    return 0;
}

#else // #ifdef _WIN32

bool CThread::create(FUNThread function, void* lpData) {
    m_funThead = function;
    m_lpData = lpData;

    return pthread_create(&m_threadID, nullptr, threadFunction, (void *)this) == 0;
}

void CThread::destroy() {
    if (m_threadID) {
        pthread_kill(m_threadID, 0);
        m_threadID = 0;
    }
}

void CThread::join() {
    if (m_threadID) {
        pthread_join(m_threadID, nullptr);
    }
}

bool CThread::isRunning() {
    return m_threadID != 0;
}

void *CThread::threadFunction(void *lpParam) {
    CThread *pThis = (CThread *)lpParam;

    pThis->m_funThead(pThis->m_lpData);
    pThis->m_threadID = 0;

    return nullptr;
}

#endif // #ifdef _WIN32
