#pragma once

class IRunnable {
public:
    IRunnable() { m_bFreeAfterRun = false; }
    virtual ~IRunnable() { }

    virtual void run() = 0;

    // inline bool IsFreeAfterRun() const { return m_bFreeAfterRun; }
private:
    friend class CLooper;
    bool                        m_bFreeAfterRun;

};

class CRunnableQueue {
public:
    CRunnableQueue();

    void add(IRunnable *pCallback);

    IRunnable *next();

protected:
    typedef list<IRunnable *>        ListRunnable;

    Event                       m_event;
    std::mutex                  m_mutex;

    ListRunnable                m_listRunnable;

};

class CLooper {
public:
    CLooper();
    virtual ~CLooper() { }

    void loop();

    void post(IRunnable *pCallback);
    void postFreeAfterRun(IRunnable *pCallback);

    void quit();

    bool isIdle() const { return m_bIdle; }

    bool                        m_bLog;
protected:
    CRunnableQueue              m_queue;
    volatile bool               m_bIdle;

};

class CLooperThread {
public:
    bool start();

protected:
    static void threadLoop(void *lpParam);

private:
    CThread                     m_thread;
    CLooper                     m_looper;

};
