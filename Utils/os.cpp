/*
 * os.cpp
 *
 *  Copyright (c) 2019 River Security Technology Corporation, Ltd. All rights reserved.
 */

#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "UtilsTypes.h"
#include "SizedString.h"
#include "os.h"


int getOperationSystemType() {
#ifdef _WIN32
    OSVERSIONINFO        version;

    memset(&version, 0, sizeof(version));
    version.dwOSVersionInfoSize = sizeof(version);

    if (!GetVersionEx(&version))
    {
        // LOG1(LOG_LVL_ERROR, "GetVersionEx FAILED! Error Id: %d", getLastError);
        return OPS_UNKNOWN;
    }

    // Major version: 5, windows 2000
    if (version.dwMajorVersion == 3)
    {
        if (version.dwMinorVersion == 51)
            return OPS_WINNT351;
    }
    else if (version.dwMajorVersion == 4)
    {
        if (version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            if (version.dwMinorVersion == 0)
                return OPS_WIN95;
            else if (version.dwMinorVersion == 10)
                return OPS_WIN98;
            else if (version.dwMinorVersion == 90)
                return OPS_WINME;
            else
                return OPS_WIN9XMORE;
        }
        else if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if (version.dwMinorVersion == 0)
                return OPS_WINNT4;
        }
    }
    else if (version.dwMajorVersion == 5)
    {
        if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if (version.dwMinorVersion == 0)
                return OPS_WIN2000;
            else if (version.dwMinorVersion == 1)
                return OPS_WINXP;
            else
                return OPS_WINXPMORE;
        }
    }
    else
        return OPS_WINXPMORE;

    return OPS_UNKNOWN;
#endif

#ifdef _LINUX
    return OPS_LINUX;
#endif

#ifdef _MAC_OS
    return OPS_MACOSX;
#endif
}

bool isWin9xSystem()
{
#ifdef _WIN32
    return (getOperationSystemType() <= OPS_WIN9XMORE);
#else
    return false;
#endif
}

#ifndef _WIN32
void Sleep(uint32_t milliseconds) {
    usleep((unsigned int)(milliseconds * 1000));
}
#endif

FileFind::FileFind() {
    _dirp = nullptr;
    _dp = nullptr;
}

FileFind::~FileFind() {
    close();
}

bool FileFind::openDir(const char *path, const char *filter) {
    close();

    _dp = opendir(path);
    _dirp = nullptr;
    _path = path;
    if (!makeSizedString(_path).endsWith(SizedString("/"))) {
        _path += "/";
    }

    return _dp != nullptr;
}

void FileFind::close() {
    if (_dp) {
        closedir(_dp);
        _dirp = nullptr;
        _dp = nullptr;
    }
}

bool FileFind::findNext() {
    _dirp = readdir(_dp);

    while (_dirp && _dirp->d_name[0] == '.') {
        if (!(_dirp->d_name[1] == '\0' || (_dirp->d_name[1] == '.' && _dirp->d_name[2] == '\0'))) {
            break;
        }
        _dirp = readdir(_dp);
    }

    return _dirp != nullptr;
}

bool FileFind::isCurDir() {
    if (!_dirp) {
        return false;
    }

    std::string fn = _path + _dirp->d_name;

    struct stat st;
    memset(&st, 0, sizeof(st));
    if (stat(fn.c_str(), &st) != 0) {
        return false;
    }

    return S_ISDIR(st.st_mode);
}

time_t getTimeInSecond() {
    return time(nullptr);
}

uint32_t getTickCount() {
    timeval tim;
    gettimeofday(&tim, nullptr);
    return (uint32_t)(tim.tv_sec * 1000 + tim.tv_usec / 1000);
}
