
#include "filesystemmonitor.h"
#include "filesystemmonitorwin32.h"
#include <process.h>
#include <winioctl.h>

//#include "mutex.h"


///////////////////////////////////////////////////////////////////////////////
// startFullDiskSearch()
// create a low priority task to start search 
///////////////////////////////////////////////////////////////////////////////
bool CFileSystemMonitorWin32::startFullDiskSearch(void)
{
    m_hWorkingThread = (HANDLE)_beginthreadex(nullptr, 0, seachWholeDisk, this, CREATE_SUSPENDED, nullptr);

    if(m_hWorkingThread)
    {
        m_bDiskQueueIdle = true;
        assert(SetThreadPriority(m_hWorkingThread, THREAD_PRIORITY_IDLE));
        ResumeThread(m_hWorkingThread);
        return true;
    }
    else
    {
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// cbIsFileIntereasted()
///////////////////////////////////////////////////////////////////////////////
bool CFileSystemMonitorWin32::cbIsFileIntereasted(const char* szFile)
{
    const char * const aExtensions[]= {"mp3", "wav", "ape"};
    const char* p1 = szFile, *p2 = szFile;

    while(nullptr != *p2)
    {
        if(*p2 == '.') p1 = p2 + 1;
        p2++;
    }
    int i = SIZEOFARRAY(aExtensions);
    
    while(i-- > 0)
    {
        if(0 == strcasecmp(aExtensions[i], p1)) break;
    }

    return (-1 != i);
}

///////////////////////////////////////////////////////////////////////////////
// cbOnNewFile()
// media library will set call back function to add or update media info
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::cbOnNewFile(const char* path, const char* file)
{

#ifdef _DEBUG
    FILE * fp = nullptr;
    static bool bFirstCall = true;
    
    SYSTEMTIME time;
    ::GetSystemTime(&time);

    if(bFirstCall)
    {
        fp = fopen("C:\\music_log.txt", "w+t");
        bFirstCall = false; 
    }
    else
    {
        fp = fopen("C:\\music_log.txt", "at");
    }

    if(fp)
    {
        _ftprintf(fp, "[%02d:%02d'%03d] %s%s\r\n", time.wMinute, time.wSecond, time.wMilliseconds, path, file);
        fflush(fp);
    }
    fclose(fp);
#endif // _DEBUG

    // update media library and notify UI
}

///////////////////////////////////////////////////////////////////////////////
// cbOnNewFile()
// media library will set call back function to add or update media info
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::cbOnRemoveFile(const char* path, const char* file)
{
}

///////////////////////////////////////////////////////////////////////////////
// cbOnEndOfFolder()
// media library will set call back function to remove all existing items under this folder
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::cbOnEndOfFolder(const char* path)
{
    // update media library and notify UI
}

///////////////////////////////////////////////////////////////////////////////
// searchFolder()
// actually search a folder
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::searchFolder(const char* szStr, /* "C:\" */ bool bRoot)
{
    WIN32_FIND_DATA data;
    string searchStr = szStr;
    searchStr += "*"; // "C:\*"

    while(!isDiskIdle(nullptr)) sleep(2000);

    HANDLE hFind = FindFirstFile(searchStr.c_str(), &data);
    if (INVALID_HANDLE_VALUE != hFind) 
    {
        if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes )
        {
            if( ('.' == data.cFileName[0]) && 
                (('\0' == data.cFileName[1]) || (('.' == data.cFileName[1]) && ('\0' == data.cFileName[2]))))
            {
                // do nothing for "." and ".."
            }
            else
            {
                string subFolder = szStr;
                subFolder += data.cFileName;
                subFolder += "\\";
                searchFolder(subFolder.c_str(), false);
            }
        }
        else
        {
            if(cbIsFileIntereasted(data.cFileName ))
            {
                cbOnNewFile(szStr, data.cFileName);
            }
        }

        while(FindNextFile(hFind, &data))
        {
            if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes )
            {
                if(bRoot)
                {
                    int i = 0;
                }


                if( ('.' == data.cFileName[0]) && 
                    (('\0' == data.cFileName[1]) || (('.' == data.cFileName[1]) && ('\0' == data.cFileName[2]))))
                {
                    // do nothing for "." and ".."
                }
                else
                {
                    string subFolder = szStr;
                    subFolder += data.cFileName;
                    subFolder += "\\";
                    searchFolder(subFolder.c_str(), false);
                }
            }
            else
            {
                if(cbIsFileIntereasted(data.cFileName ))
                {
                    cbOnNewFile(szStr, data.cFileName);
                }
            }
        }// while

        cbOnEndOfFolder(szStr);
    }// if 

}
///////////////////////////////////////////////////////////////////////////////
// seachWholeDisk()
// entry of working thread
///////////////////////////////////////////////////////////////////////////////
unsigned int __stdcall CFileSystemMonitorWin32::seachWholeDisk(void* param)
{
    CFileSystemMonitorWin32* p = (CFileSystemMonitorWin32*)param;
    // enum all disk with the type

    char aLogicDiskString[DRIVE_STR_BUF_SIZE] = {0};
    uint32_t len = GetLogicalDriveStrings(DRIVE_STR_STR_LEN, aLogicDiskString);
    char* szStr = aLogicDiskString;


    if(len < DRIVE_STR_STR_LEN)
    {
        // c:\<null>d:\<null><null> //
        while(nullptr != *szStr)
        {
            // search this disk
            p->searchFolder(szStr, true);

            szStr += strlen(szStr) + 1; // jump to next 
        }
    }
    return 1; // 0 for error
}


///////////////////////////////////////////////////////////////////////////////
// isDiskIdle()
// to check disk queue length 
///////////////////////////////////////////////////////////////////////////////
bool CFileSystemMonitorWin32::isDiskIdle(const char* disk)
{
    HANDLE  hDevice;    // handle to the drive to be examined 
    bool    bResult;    // results flag
    uint32_t   junk;       // discard results
    
    hDevice = CreateFile("\\\\.\\PhysicalDrive0",  // drive to open
                      0,                // no access to the drive
                      FILE_SHARE_READ | // share mode
                      FILE_SHARE_WRITE, 
                      nullptr,             // default security attributes
                      OPEN_EXISTING,    // disposition
                      0,                // file attributes
                      nullptr);            // do not copy file attributes
    
    if (INVALID_HANDLE_VALUE != hDevice) // cannot open the drive
    {
        DISK_PERFORMANCE data;

        bResult = DeviceIoControl(hDevice,  // device to be queried
                  IOCTL_DISK_PERFORMANCE,   // operation to perform
                                  nullptr, 0,  // no input buffer
                                  &data, sizeof(data),   // output buffer
                                  &junk,                 // # bytes returned
                                  (LPOVERLAPPED) nullptr);  // synchronous I/O

        CloseHandle(hDevice);
        return (0 == data.QueueDepth);
    }
    
    return (false);

}
