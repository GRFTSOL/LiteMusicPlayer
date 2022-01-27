#ifndef _SOCK_ADAPTER_INC_
#define _SOCK_ADAPTER_INC_

#ifdef WIN32
// socket
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#ifndef WIN32
#include <unistd.h>

typedef int SOCKET;
#define closesocket(s)        close(s)

#define INVALID_SOCKET (SOCKET)(~0) 

#endif

#endif // _SOCK_ADAPTER_INC_
