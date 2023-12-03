#pragma once

/******************************************************************************
Implement CFileSystemMonitor on Win32 Platform
*****************************************************************************/
#ifndef FILE_SYSTEM_MONITOR_WIN32_H
#define FILE_SYSTEM_MONITOR_WIN32_H


#include "filesystemmonitor.h"
//#include "mutex.h"


#define MAX_DRIVER_SUPPORT  ('Z' - 'A' + 1)
#define DRIVE_STR_BUF_SIZE    ((3 + 1)  * MAX_DRIVER_SUPPORT + 1)
#define DRIVE_STR_STR_LEN    (3 * MAX_DRIVER_SUPPORT)

class CFileSystemMonitorWin32: public CFileSystemMonitor {
public:
    CFileSystemMonitorWin32():m_hWorkingThread(nullptr), m_bDiskQueueIdle(true){};
    ~CFileSystemMonitorWin32(){};

public:
    virtual bool startFullDiskSearch(void);

protected:
    // Platform depend implementation may override below call back functions
    // for full disk search, cbOnNewFile will always be called with any files, Media libarary need to check if the file is a new one, or exisitng one
    // for file system monitor, when cbOnNewFile is called, there are new file add to Media Library; when cbOnRemoveFile is called there are file deleted.
    virtual bool cbIsFileIntereasted(const char* szFile);
    virtual void cbOnNewFile(const char* path, const char* file);
    virtual void cbOnEndOfFolder(const char* path);
    virtual void cbOnRemoveFile(const char* path, const char* file);



protected:
    void searchFolder(const char* szStr, bool bRoot);
    static unsigned int __stdcall seachWholeDisk(void * param);
    bool isDiskIdle(const char* disk);

protected:
    HANDLE                      m_hWorkingThread;   // cbFullDiskSearchThread() thread
    bool                        m_bDiskQueueIdle;

};



#endif //FILE_SYSTEM_MONITOR_WIN32_H
