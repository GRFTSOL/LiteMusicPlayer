#include "base.h"
#include "safestr.h"
#include "base.h"
#include "fileapi.h"
#include "io.h"


bool filetruncate(FILE *fp, int nLen) {
    if (fseek(fp, nLen, SEEK_SET) != 0) {
        return false;
    }

    HANDLE hFile;
    hFile = (HANDLE)_get_osfhandle(fp->_file);
    if (hFile) {
        if (!SetEndOfFile(hFile)) {
            return false;
        }
    }

    return true;
}

bool isFileExist(cstr_t szFileName) {
    return (GetFileAttributes(szFileName) != -1);
}

bool isDirExist(cstr_t szDir) {
    uint32_t dwRet;

    dwRet = GetFileAttributes(szDir);

    return (dwRet != -1 && (dwRet & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
}

bool getFileLength(cstr_t szFileName, uint64_t &nLength) {
    HANDLE hFileFind;
    WIN32_FIND_DATA FindData;
    hFileFind = FindFirstFile(szFileName, &FindData);
    if (hFileFind != INVALID_HANDLE_VALUE) {
        if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) {
            LARGE_INTEGER length;
            length.LowPart = FindData.nFileSizeLow;
            length.HighPart = FindData.nFileSizeHigh;
            nLength = length.QuadPart;
            FindClose(hFileFind);
            return true;
        }
    }

    FindClose(hFileFind);
    return false;
}

// RETURN:
//        INVALID_FILE_SIZE    - Invalid file size
uint32_t getFileLength(cstr_t szFileName) {
    HANDLE hFileFind;
    WIN32_FIND_DATAA FindData;
    uint32_t dwSize = INVALID_FILE_SIZE;

    hFileFind = FindFirstFileA(szFileName, &FindData);
    if (hFileFind != INVALID_HANDLE_VALUE) {
        if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) {
            dwSize = FindData.nFileSizeLow;
        }
    }

    FindClose(hFileFind);

    return dwSize;
}

uint32_t getFileLength(cwstr_t szFileName) {
    HANDLE hFileFind;
    WIN32_FIND_DATAW FindData;
    uint32_t dwSize = INVALID_FILE_SIZE;

    hFileFind = FindFirstFileW(szFileName, &FindData);
    if (hFileFind != INVALID_HANDLE_VALUE) {
        if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) {
            dwSize = FindData.nFileSizeLow;
        }
    }

    FindClose(hFileFind);

    return dwSize;
}

bool createDirectory(cstr_t lpPathName) {
    return tobool(createDirectory(lpPathName, nullptr));
}

bool setFileTime(
    cstr_t szFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime
    ) {
    HANDLE hFile;
    bool bRet;

    hFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING, 0, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    bRet = setFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);

    CloseHandle(hFile);

    return bRet;
}

uint32_t isDirWritable(cstr_t szDir) {
    uint32_t dwAttr;

    dwAttr = GetFileAttributes(szDir);
    if (dwAttr == 0xFFFFFFFF) {
        return false;
    }

    if (!setFileAttributes(szDir, dwAttr)) {
        return false;
    }

    // Try to create a file in the folder, then delete it.
    string strFile = szDir;
    dirStringAddSep(strFile);
    strFile += "temp.txt";
    if (!saveDataAsFile(strFile.c_str(), "abc", 3)) {
        return false;
    }
    deleteFile(strFile.c_str());

    return true;
}

//////////////////////////////////////////////////////////////////////////

FileFind::FileFind() {
    hFileFind = INVALID_HANDLE_VALUE;
    bFirst = false;
    memset(&FindData, 0, sizeof(FindData));
}

FileFind::~FileFind() {
    if (hFileFind) {
        FindClose(hFileFind);
    }
}

bool FileFind::openDir(cstr_t szDir, cstr_t extFilter) {
    if (!extFilter) {
        extFilter = "*.*";
    }

    string str;
    str = szDir;
    dirStringAddSep(str);
    str += extFilter;

    hFileFind = FindFirstFile(str.c_str(), &FindData);
    if (hFileFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    bFirst = true;

    // remove "." and ".." from result.
    while (isFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)
        && isCurOrParentDir(FindData.cFileName)) {
        if (!FindNextFile(hFileFind, &FindData)) {
            close();
            return false;
        }
    }

    return true;
}

void FileFind::close() {
    if (hFileFind) {
        FindClose(hFileFind);
        hFileFind = nullptr;
    }
}

bool FileFind::findNext() {
    if (bFirst) {
        bFirst = false;
        return true;
    }

    if (!FindNextFile(hFileFind, &FindData)) {
        return false;
    }

    // remove "." and ".." from result.
    while (isFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)
        && isCurOrParentDir(FindData.cFileName)) {
        if (!FindNextFile(hFileFind, &FindData)) {
            return false;
        }
    }

    return true;
}

#include "Winioctl.h"
#include "base.h"


void getLogicalDrives(VecStrings &vDrives) {
#define MAX_DRIVER_SUPPORT  ('Z' - 'A' + 1)
#define DRIVE_STR_BUF_SIZE    ((3 + 1)  * MAX_DRIVER_SUPPORT + 1)
#define DRIVE_STR_STR_LEN    (3 * MAX_DRIVER_SUPPORT)

    char aLogicDiskString[DRIVE_STR_BUF_SIZE] = { 0 };
    uint32_t len = GetLogicalDriveStrings(DRIVE_STR_STR_LEN, aLogicDiskString);

    if (len > 0) {
        multiStrToVStr(aLogicDiskString, vDrives);
    }

}

// Check disk queue length
bool isDiskIdle(const char* disk) {
    HANDLE hDevice; // handle to the drive to be examined
    bool bResult; // results flag
    uint32_t junk; // discard results

    hDevice = CreateFile("\\\\.\\PhysicalDrive0",  // drive to open
        0,                // no access to the drive
        FILE_SHARE_READ | // share mode
        FILE_SHARE_WRITE,
        nullptr,             // default security attributes
        OPEN_EXISTING,    // disposition
        0,                // file attributes
        nullptr);            // do not copy file attributes

    if (INVALID_HANDLE_VALUE == hDevice) {
        return false;
    }

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
