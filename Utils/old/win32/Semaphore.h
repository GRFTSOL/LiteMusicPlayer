#pragma once

#ifndef Utils_old_win32_Semaphore_h
#define Utils_old_win32_Semaphore_h


#define SEM_LOCKED          0
#define SEM_UNLOCKED        1

#define WAIT_FOREVER        INFINITE

class Semaphore {
public:
    Semaphore(int nCount = SEM_LOCKED);
    virtual ~Semaphore();

    bool wait(int ms = WAIT_FOREVER); // returns false if it times out
    void signal();

private:
    int                         m_nCount;
    HANDLE                      m_hSemaphore;

};

#endif // !defined(Utils_old_win32_Semaphore_h)
