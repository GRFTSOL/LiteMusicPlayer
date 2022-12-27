#include "../stdafx.h"
#include "android.h"
#include "Thread.h"


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
    pthread_kill(m_threadID, 0);
}

void CThread::suspend() {
}

void CThread::resume() {
}

void CThread::join() {
    if (m_threadID) {
        pthread_join(m_threadID, nullptr);
    }
}

uint32_t CThread::getPriority() const {
    return 0;
}

uint32_t CThread::setPriority(uint32_t priority) {
    return 0;
}

bool CThread::isRunning() {
    return false;
}

void *CThread::threadFunction(void *lpParam) {
    CThread *pThis = (CThread *)lpParam;

    JNIEnv *env;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        // Error occurred.
    }

    pThis->m_funThead(pThis->m_lpData);

    g_jvm->DetachCurrentThread();

    return nullptr;
}
