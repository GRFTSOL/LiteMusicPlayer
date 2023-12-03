#include "../UtilsTypes.h"
#include "../Error.h"
#include <sys/errno.h>


OSError::~OSError() {
}

void OSError::doFormatMessage(unsigned int dwLastErr) {
    m_dwErrCode = dwLastErr;
    m_szErrMsg = strerror((int)dwLastErr);
}

uint32_t OSError::getLastError() {
    return errno;
}
