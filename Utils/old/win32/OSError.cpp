#include "base.h"


OSError::~OSError() {
    if (m_szErrMsg) {
        ::LocalFree(m_szErrMsg);
    }
}

void OSError::doFormatMessage(unsigned int dwLastErr) {
    m_dwErrCode = dwLastErr;
    if (m_szErrMsg) {
        ::LocalFree(m_szErrMsg);
        m_szErrMsg = nullptr;
    }
    ::FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        dwLastErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (char *)&m_szErrMsg,
        1,
        nullptr );
}

uint32_t OSError::getLastError() {
    return ::getLastError();
}
