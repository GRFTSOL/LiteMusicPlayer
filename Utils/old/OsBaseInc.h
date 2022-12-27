#pragma once

#include "config.h"

#if defined(_LINUX_GTK2)
#include <gtk/gtk.h>
#define MLDebug             g_print
#endif

// Define always inline for GCC compiler.
#ifdef WIN32
#define
#else
#define __attribute__((always_inline)   )
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN        // Exclude rarely-used stuff from Windows headers

#define _WIN32_WINNT        0x0500

// Windows Header Files:
#include <windows.h>
#include <winuser.h>
#include <Shlobj.h>
#include <shellapi.h>
#include <commdlg.h>

#include <tchar.h>
#include <malloc.h>
#endif


#ifdef _LINUX
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#endif

#ifdef _ANDROID
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#endif

#ifdef _MAC_OS
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#endif

// C RunTime Header Files
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>


// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#pragma warning(disable:4996)

#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
using namespace std;

#ifndef assert
#define assert              assert
#endif

#include "mltypes.h"
#include "common.h"
#include "wintypes.h"

typedef vector<string> VecStrings;
typedef vector<string> VecStrings;
typedef list<string> ListStrings;
typedef set<string> SetStrings;

class SetStrLessICmp {
public:
    bool operator()(const string &str1, const string &str2) const {
        return strcasecmp(str1.c_str(), str2.c_str()) < 0;
    }
};

typedef set<string, SetStrLessICmp> SetICaseStr;

//
//#define ML_USES_CONVERSION        USES_CONVERSION
//#define ML_T2A                    T2A
//#define ML_T2CA                    T2CA
//#define ML_T2W                    T2W
//#define ML_T2CW                    T2CW
//#define ML_A2CT                    A2CT
//#define ML_W2CA                    W2CA

// CPPUnit test headers
#include "cppunit-utils.h"
