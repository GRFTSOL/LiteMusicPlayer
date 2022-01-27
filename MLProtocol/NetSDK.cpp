#include "NetSDK.h"

NETLIB_API bool InitNetLib()
{
#ifdef _WIN32
    uint16_t sockVersion;
    WSADATA wsaData;

    sockVersion = MAKEWORD(2, 2);
    //start dll
    return (WSAStartup(sockVersion, &wsaData) == 0);
#else
    return true;
#endif
}
