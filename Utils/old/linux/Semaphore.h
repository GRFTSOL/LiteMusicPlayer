#ifndef Utils_old_linux_Semaphore_h
#define Utils_old_linux_Semaphore_h

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


class Semaphore {
public:
    Semaphore(int nCount = SEM_LOCKED);
    virtual ~Semaphore();

    bool wait(int ms = WAIT_FOREVER); // returns false if it times out
    void signal();

private:

};

#endif // !defined(Utils_old_linux_Semaphore_h)
