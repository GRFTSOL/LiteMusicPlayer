#pragma once

#ifndef Utils_old_win32_Thread_h
#define Utils_old_win32_Thread_h



typedef void (*FUNThread)(void * lpParam);

class CThread {
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
    FUNThread                   m_funThead;
    void                        *m_lpData;

protected:
    static uint32_t WINAPI threadFunction(void *lpParam);

protected:
    HANDLE                      m_hThread;

};

#endif // !defined(Utils_old_win32_Thread_h)
