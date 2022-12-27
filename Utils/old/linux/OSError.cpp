#include "../stdafx.h"
#include "../base.h"


OSError::~OSError() {
}

void OSError::doFormatMessage(unsigned int dwLastErr) {
    m_dwErrCode = dwLastErr;
    m_szErrMsg = strerror((int)dwLastErr);
}

uint32_t OSError::getLastError() {
    return errno;
}
