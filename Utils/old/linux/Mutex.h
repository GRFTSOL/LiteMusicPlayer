// Mutex.h: interface for the Mutex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MUTEX_H__F62F5D53_F816_4868_BCE8_9951B1E49EC5__INCLUDED_)
#define AFX_MUTEX_H__F62F5D53_F816_4868_BCE8_9951B1E49EC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WAIT_FOREVER
#define WAIT_FOREVER 0xFFFFFFFF
#endif

class Mutex  
{
public:
    Mutex();
    virtual ~Mutex();

    bool acquire(uint32_t nTimeOut = WAIT_FOREVER);
    void release();

private:

};

class MutexAutolock
{
public:
    MutexAutolock(Mutex &mutex) : m_pMutex(&mutex) { m_pMutex->acquire(); }
    ~MutexAutolock() { if (m_pMutex) m_pMutex->release(); }

    void unlock() { if (m_pMutex) { m_pMutex->release(); m_pMutex = nullptr; } }

    Mutex        *m_pMutex;
};

#endif // !defined(AFX_MUTEX_H__F62F5D53_F816_4868_BCE8_9951B1E49EC5__INCLUDED_)
