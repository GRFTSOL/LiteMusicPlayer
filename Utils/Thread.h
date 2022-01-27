// Thread.h: interface for the CThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_)
#define AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <pthread.h>


typedef void (*FUNThread)(void * lpParam);


class CThread
{
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
    FUNThread       m_funThead;
    void            *m_lpData;
    pthread_t       m_threadID;

};

#endif // !defined(AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_)
