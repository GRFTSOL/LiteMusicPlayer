#include "Mutex.h"


Mutex::Mutex() {
    m_hMutex = CreateMutex(    nullptr,
        false,
        nullptr);
}

Mutex::~Mutex() {
    CloseHandle(m_hMutex);
}

bool Mutex::acquire(uint32_t nTimeOut) {
    switch (WaitForSingleObject(m_hMutex, nTimeOut)) {
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0:
        return true;
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
        break;
    }

    return false;
}

void Mutex::release() {
    ReleaseMutex(m_hMutex);
}
