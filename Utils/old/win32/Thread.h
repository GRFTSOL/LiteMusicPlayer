// Thread.h: interface for the CThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_)
#define AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef void (*FUNThread)(void * lpParam);

class CThread
{
public:
    CThread();
    virtual ~CThread();

    bool create(FUNThread function, void* lpData);
    void destroy();
    void suspend();
    void resume();
    void join();

    uint32_t getPriority() const;
    uint32_t setPriority(uint32_t priority);

    bool isRunning();

protected:
    FUNThread        m_funThead;
    void            *m_lpData;

protected:
    static uint32_t WINAPI threadFunction(void *lpParam);

protected:
    HANDLE            m_hThread;

};

#endif // !defined(AFX_THREAD_H__5ECE071C_BE98_469F_808C_6C8AFCB2A55A__INCLUDED_)
