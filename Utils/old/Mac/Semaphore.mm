#include <limits.h>
#include "Semaphore.h"
#include <Foundation/Foundation.h>


class _SemaphoreInternal {
public:
    _SemaphoreInternal() {
        lock = [NSObject alloc];
    }

    ~_SemaphoreInternal() {
        [lock release];
    }

    NSObject                    *lock;
};


Semaphore::semaphore(int nCount) {
    m_internal = new _SemaphoreInternal();
    m_nCount = nCount;
    m_nLockCount = 0;
    m_mutex = new Mutex();
}

Semaphore::~Semaphore() {
    delete m_mutex;
    delete m_internal;
}

bool Semaphore::Wait(int ms) {
    bool bAcquire = false;
    @synchronized(m_internal->lock) {
        if (m_nLockCount < m_nCount) {
            m_nLockCount++;
        } else if (m_nLockCount == m_nCount) {
            bAcquire = true;
        }
    }
    while (bAcquire) {
        if (!m_mutex->Acquire(ms)) {
            return false;
        }
        @synchronized(m_internal->lock) {
            if (m_nLockCount < m_nCount) {
                m_nLockCount++;
                return true;
            }
        }
    }

    return true;
}

void Semaphore::signal() {
    @synchronized(m_internal->lock) {
        if (m_nLockCount > 0) {
            m_nLockCount--;
        }
        if (m_nLockCount == m_nCount - 1) {
            m_mutex->Release();
        }
    }
}
