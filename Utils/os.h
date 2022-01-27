#ifndef __os__
#define __os__

#include <stdio.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>


//
// 操作系统的类型定义
//
enum OPERATION_SYSTEM_TYPE
{
    OPS_WIN95,
    OPS_WINNT4,
    OPS_WIN98,
    OPS_WINME,
    OPS_WIN9XMORE,
    OPS_WINNT351,
    OPS_WIN2000,
    OPS_WINXP,
    OPS_WINXPMORE,
    OPS_UNKNOWN,

    OPS_LINUX,
    OPS_MACOSX,
};

// 取得操作系统的类型
int getOperationSystemType();

bool isWin9xSystem();

time_t getTimeInSecond();

uint32_t getTickCount();

void Sleep(uint32_t milliseconds);

class FileFind {
public:
    FileFind();
    virtual ~FileFind();

    bool openDir(const char *path, const char *filter = nullptr);
    void close();

    bool findNext();

    const char *getCurName() { return _dirp->d_name; }

    bool isCurDir();

protected:
    dirent                  *_dirp;
    DIR                     *_dp;
    std::string             _path;

};

#endif /* __os__ */
