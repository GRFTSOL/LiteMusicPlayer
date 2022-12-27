#include <limits.h>
#include "Semaphore.h"


Semaphore::Semaphore(int nCount) {
    m_nCount = nCount;
    m_hSemaphore = CreateSemaphore(nullptr,
        nCount,
        LONG_MAX,
        "");
}

Semaphore::~Semaphore() {
    CloseHandle(m_hSemaphore);
}

bool Semaphore::wait(int ms) {
    if (WaitForSingleObject(m_hSemaphore, ms) == WAIT_TIMEOUT) {
        return false;
    }

    return true;
}

void Semaphore::signal() {
    ReleaseSemaphore(m_hSemaphore,
        1,
        nullptr);
}
