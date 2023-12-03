#pragma once

#ifndef Utils_Thread_h
#define Utils_Thread_h

#ifdef _WIN32
#else
#include <pthread.h>
#endif


typedef void (*FUNThread)(void * lpParam);


class CThread {
public:
    CThread();
    virtual ~CThread();

    bool create(FUNThread function, void* lpData);
    void destroy();
    void join();

    bool isRunning();

protected:
    FUNThread                   m_funThead = nullptr;
    void                        *m_lpData = nullptr;

#ifdef _WIN32
    static DWORD WINAPI threadFunction(void *lpParam);

    HANDLE                      m_hThread = NULL;
#else
    static void *threadFunction(void *lpParam);

    pthread_t                   m_threadID = 0;
#endif

};

#endif // !defined(Utils_Thread_h)
