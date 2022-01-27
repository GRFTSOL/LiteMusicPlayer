#pragma once

bool
setFileTime(
    cstr_t szFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime
    );

bool isDiskIdle(const char* disk);
void getLogicalDrives(VecStrings &vDrives);

class FileFind
{
public:
    FileFind();
    virtual ~FileFind();

    bool openDir(cstr_t szDir, cstr_t szFilter = nullptr);

    void close();

    bool findNext();

    cstr_t getCurName()
        { return FindData.cFileName; }

    bool isCurDir()
        { return isFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY); }

    uint32_t getFileSize()
        { return FindData.nFileSizeLow; }

protected:
    HANDLE                hFileFind;
    WIN32_FIND_DATA        FindData;
    bool                bFirst;

};
