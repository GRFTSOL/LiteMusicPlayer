#pragma once

﻿/******************************************************************************
The idea here to keep media libarary are updated in time. The detail requirements are:
    1>  每次软件启动之后都做一次“几乎不能被用户感知的的后台扫描”，扫描中自动增加新的歌曲，删除已经不存在的文件
    2>  用户手动添加到播放器的文件，如果不存在于媒体库中，都需要自动添加进入

    3>  “几乎不能被用户感知的的后台扫描”进程需要扫描，所有硬盘，U盘，CD/DVD,软驱，和网络驱动器
    4>  “几乎不能被用户感知的的后台扫描”发现有新文件会自动添加到媒体库，除了播放器的All tag会及时更新，不会有其他任何提示（某个特定的Tag也可能因为文件不再存在而被更新，但是概率很低）
    5>  “几乎不能被用户感知的的后台扫描”发现一个文件不在存在时，就删除它
    5+> 第一版本之后：记录文件的MD5码，允许用户移动文件的时候，播放器仍然能够正确的找到以前播放的次数，和相关的歌词。  
*****************************************************************************/

#ifndef FILE_SYSTEM_MONITOR_H
#define FILE_SYSTEM_MONITOR_H


///////////////////////////////////////////////////////////////////////////////
class CFileSystemMonitor {
public:
    virtual bool startFullDiskSearch(void) = 0;

protected:
    // Platform depend implementation may override below call back functions
    virtual bool cbIsFileIntereasted(const char* szFile) { return false; }
    virtual void cbOnNewFile(const char* path, const char* file) { }
    virtual void cbOnEndOfFolder(const char* path) { }
    virtual void cbOnRemoveFile(const char* path, const char* file) { }

private:

};
///////////////////////////////////////////////////////////////////////////////

#endif //FILE_SYSTEM_MONITOR_H
