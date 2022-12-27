#ifndef Utils_old_Mac_Semaphore_h
#define Utils_old_Mac_Semaphore_h

#pragma once

#include "Mutex.h"


#define SEM_LOCKED          0
#define SEM_UNLOCKED        1

#ifndef INFINITE
#define INFINITE            0xFFFFFFFF
#endif

#ifndef WAIT_FOREVER
#define WAIT_FOREVER        0xFFFFFFFF
#endif

class _SemaphoreInternal;


class Semaphore {
public:
    Semaphore(int nCount = SEM_LOCKED);
    virtual ~Semaphore();

    bool wait(int ms = WAIT_FOREVER); // returns false if it times out
    void signal();

private:
    _SemaphoreInternal          *m_internal;
    Mutex                       *m_mutex;
    int                         m_nCount;
    int                         m_nLockCount;

};

#endif // !defined(Utils_old_Mac_Semaphore_h)
