#include "../UtilsTypes.h"
#include "Event.h"
#import <pthread.h>
#import <sys/time.h>


class _NSConditionInternal {
public:
    _NSConditionInternal(bool isSingaled) {
        pthread_cond_init(&condition, nullptr);
        pthread_mutex_init(&mutex, nullptr);
        read_to_go = isSingaled;
    }

    pthread_cond_t              condition;
    pthread_mutex_t             mutex;
    volatile bool               read_to_go;

};

//////////////////////////////////////////////////////////////////////

Event::Event(bool bManualSet, bool isSingaled) {
    m_nsCondition = new _NSConditionInternal(isSingaled);

    m_bMaualset = bManualSet;
}

Event::~Event() {
}

bool Event::set() {
    pthread_mutex_lock(&m_nsCondition->mutex);
    m_nsCondition->read_to_go = true;
    pthread_cond_signal(&m_nsCondition->condition);
    pthread_mutex_unlock(&m_nsCondition->mutex);
    return true;
}

bool Event::reset() {
    m_nsCondition->read_to_go = false;
    return true;
}

bool Event::acquire(uint32_t nTimeOut) {
    if (nTimeOut == 0) {
        return m_nsCondition->read_to_go;
    }

    bool ret = false;
    pthread_mutex_lock(&m_nsCondition->mutex);
    if (nTimeOut == WAIT_FOREVER) {
        while (!m_nsCondition->read_to_go) {
            pthread_cond_wait(&m_nsCondition->condition, &m_nsCondition->mutex);
        }
        if (!m_bMaualset) {
            m_nsCondition->read_to_go = false;
        }
        pthread_mutex_unlock(&m_nsCondition->mutex);
        ret = true;
    } else {
        struct timeval tv;
        struct timespec ts;
        gettimeofday(&tv, nullptr);
        ts.tv_sec = time(nullptr) + nTimeOut / 1000;
        ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (nTimeOut % 1000);
        ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
        ts.tv_nsec %= (1000 * 1000 * 1000);

        int n = 0;
        if (!m_nsCondition->read_to_go) {
            n = pthread_cond_timedwait(&m_nsCondition->condition, &m_nsCondition->mutex, &ts);
        }

        if (n == 0) {
            ret = true;
            if (!m_bMaualset) {
                m_nsCondition->read_to_go = false;
            }
        }
        pthread_mutex_unlock(&m_nsCondition->mutex);
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CEvent)

class CTestCaseCEvent : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CTestCaseCEvent);
    CPPUNIT_TEST(TestAcquire);
    CPPUNIT_TEST(TestAll);
    CPPUNIT_TEST_SUITE_END();

protected:
    void TestAcquire() {
        Event e(false, false);

        {
            // Step 1
            uint32_t start = GetTickCount();
            CThread thread;
            thread.create(ThreadSetEvent, &e);
            assert(e.Acquire());
            uint32_t cost = GetTickCount() - start;
            assert(cost >= 1000 && cost < 1500);
        }

        {
            // Step 2
            uint32_t start = GetTickCount();
            CThread thread;
            thread.create(ThreadSetEvent, &e);
            assert(e.Acquire(0) == false);
            assert(e.Acquire(500) == false);
            assert(e.Acquire(1000));
            uint32_t cost = GetTickCount() - start;
            assert(cost >= 1000 && cost < 2000);
        }


        {
            // Step 3: Compete
            uint32_t start = GetTickCount();
            CThread thread;
            thread.create(ThreadSetEvent, &e);
            CThread thread2;
            thread2.create(ThreadCompete, &e);

            assert(e.Acquire(0) == false);
            assert(e.Acquire(500) == false);
            uint32_t cost = GetTickCount() - start;
            assert(cost >= 500 && cost < 1000);
            thread2.Join();
            cost = GetTickCount() - start;
            assert(cost >= 1000 && cost < 2000);
        }
    }

    static void ThreadSetEvent(void *param) {
        Event *e = (Event *)param;
        Sleep(1000);
        e->Set();
    }

    static void ThreadCompete(void *param) {
        Event *e = (Event *)param;
        assert(e->Acquire());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCEvent);

#endif // _CPPUNIT_TEST
