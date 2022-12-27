#pragma once

#ifndef Utils_Thread_h
#define Utils_Thread_h


#include <pthread.h>


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
    static void *threadFunction(void *lpParam);

protected:
    FUNThread                   m_funThead;
    void                        *m_lpData;
    pthread_t                   m_threadID;

};

#endif // !defined(Utils_Thread_h)
