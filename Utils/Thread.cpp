// Thread.cpp: implementation of the CThread class.
//
//////////////////////////////////////////////////////////////////////

#include "Thread.h"
#include <pthread.h>


CThread::CThread() {
    m_funThead = nullptr;
    m_lpData = nullptr;
    m_threadID = 0;
}

CThread::~CThread() {
}

bool CThread::create(FUNThread function, void* lpData) {
    m_funThead = function;
    m_lpData = lpData;

    return pthread_create(&m_threadID, nullptr, threadFunction, (void *)this) == 0;
}

void CThread::destroy() {
    if (m_threadID) {
        pthread_kill(m_threadID, 0);
    }
}

void CThread::join() {
    if (m_threadID) {
        pthread_join(m_threadID, nullptr);
    }
}

bool CThread::isRunning()
{
    return m_threadID != 0;
}

void *CThread::threadFunction(void *lpParam)
{
    CThread    *pThis = (CThread *)lpParam;

    pThis->m_funThead(pThis->m_lpData);
    pThis->m_threadID = 0;

    return nullptr;
}
