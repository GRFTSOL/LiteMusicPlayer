// Semaphore.h: interface for the Semaphore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEMAPHORE_H__F793948B_D179_4D98_AA9E_BB65322B4D78__INCLUDED_)
#define AFX_SEMAPHORE_H__F793948B_D179_4D98_AA9E_BB65322B4D78__INCLUDED_

#pragma once

#include "Mutex.h"

#define SEM_LOCKED   0
#define SEM_UNLOCKED 1

#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif

#ifndef WAIT_FOREVER
#define WAIT_FOREVER 0xFFFFFFFF
#endif

class _SemaphoreInternal;


class Semaphore  
{
public:
    Semaphore(int nCount = SEM_LOCKED);
    virtual ~Semaphore();

    bool wait(int ms = WAIT_FOREVER); // returns false if it times out
    void signal();
 
private:
    _SemaphoreInternal    *m_internal;
    Mutex                *m_mutex;
    int                    m_nCount;
    int                    m_nLockCount;

};

#endif // !defined(AFX_SEMAPHORE_H__F793948B_D179_4D98_AA9E_BB65322B4D78__INCLUDED_)
