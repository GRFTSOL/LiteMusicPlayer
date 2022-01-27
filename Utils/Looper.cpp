#include "Utils.h"
#include "Looper.h"

CRunnableQueue::CRunnableQueue() : m_event(false, false)
{
}

void CRunnableQueue::add(IRunnable *pCallback)
{
    MutexAutolock lock(m_mutex);

    m_listRunnable.push_back(pCallback);
    m_event.set();
}

IRunnable *CRunnableQueue::next()
{
    // Is empty? If so, reset event
    m_mutex.lock();
    if (m_listRunnable.empty())
        m_event.reset();
    m_mutex.unlock();

    // wait for event
    m_event.acquire();

    // get one runnable.
    m_mutex.lock();
    assert(m_listRunnable.size() > 0);
    IRunnable *pCallback = m_listRunnable.front();
    m_listRunnable.pop_front();
    m_mutex.unlock();

    return pCallback;
}

//////////////////////////////////////////////////////////////////////////

CLooper::CLooper()
{
    m_bLog = true;
    m_bIdle = true;
}

void CLooper::loop()
{
    if (m_bLog)
        DBG_LOG0("CLooper: Enter loop");

    while (true)
    {
        IRunnable *pCallback = m_queue.next();
        if (m_bLog)
            DBG_LOG1("CLooper: retrieved a runnable: %x", pCallback);

        m_bIdle = false;
        if (pCallback == nullptr)
        {
            if (m_bLog)
                DBG_LOG0("CLooper: Leave loop");
            return;
        }

        pCallback->run();

        m_bIdle = true;
        if (pCallback->m_bFreeAfterRun)
            delete pCallback;
    }
}

void CLooper::post(IRunnable *pCallback)
{
    m_queue.add(pCallback);
}

void CLooper::postFreeAfterRun(IRunnable *pCallback)
{
    pCallback->m_bFreeAfterRun = true;
    m_queue.add(pCallback);
}

void CLooper::quit()
{
    m_queue.add(nullptr);
}

//////////////////////////////////////////////////////////////////////////

bool CLooperThread::start()
{
    return m_thread.create(threadLoop, this);
}

void CLooperThread::threadLoop(void *lpParam)
{
    CLooperThread *pThis = (CLooperThread *)lpParam;

    pThis->m_looper.loop();
}
