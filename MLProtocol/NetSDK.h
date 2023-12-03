#pragma once

#ifndef _NET_SDK_H
#define _NET_SDK_H

#include "../Utils/Utils.h"
#include "sockadapt.h"

#pragma warning(disable:4786)


#define NETLIB_API

NETLIB_API bool InitNetLib();

#define TC_LEN_TYPE         4
#define TC_LEN_XMLLEN       7

class CComunicator;
class CServer;


#endif // _NET_SDK_H
