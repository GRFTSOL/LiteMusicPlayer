#include "../stdafx.h"
#include "../safestr.h"
#include "fileapi.h"
#include "../stringex_t.h"
#include "../CharEncoding.h"
#include "../string.h"
#include "../FileApiBase.h"


bool filetruncate(FILE *fp, int nLen) {
    if (ftruncate(fileno(fp), nLen) == -1) {
        return false;
    }

    return true;
}

bool isFileExist(cstr_t szFileName) {
    struct stat        filestat;

    memset(&filestat, 0, sizeof(filestat));
    return stat(szFileName, &filestat) == 0;
}

bool isDirExist(cstr_t szDir) {
    DIR *dp;

    dp = opendir(szDir);
    if (dp == nullptr) {
        return false;
    }

    closedir(dp);

    return true;
}

bool getFileLength(cstr_t szFileName, uint64_t &nLength) {
    struct stat        filestat;
    int ret;

    memset(&filestat, 0, sizeof(filestat));
    ret = stat(szFileName, &filestat);
    if (ret == 0) {
        nLength = filestat.st_size;
        return true;
    } else {
        nLength = 0;
        return false;
    }
}

// RETURN:
//        INVALID_FILE_SIZE    - Invalid file size
uint32_t getFileLength(cstr_t szFileName) {
    struct stat        filestat;
    int ret;

    memset(&filestat, 0, sizeof(filestat));
    ret = stat(szFileName, &filestat);
    if (ret == 0) {
        return filestat.st_size;
    } else {
        return INVALID_FILE_SIZE;
    }
}

bool deleteFile(cstr_t lpFileName) {
    if (unlink(lpFileName) == 0) {
        return true;
    }

    return false;
}

bool moveFile(const char *oldname, const char *newname) {
    return rename(oldname, newname) == 0;
}

bool removeDirectory(cstr_t lpPathName) {
    return rmdir(lpPathName) == 0;
}

bool createDirectory(cstr_t lpPathName) {
    int n = mkdir(lpPathName, S_IRWXU | S_IRWXG | S_IROTH);

    return n == 0;
}

#ifdef _LINUX_GTK2
#include <copyfile.h>


bool copyFile(cstr_t lpExistingFileName, cstr_t lpNewFileName, bool bFailIfExists) {
    if (!bFailIfExists) {
        if (isFileExist(lpNewFileName)) {
            deleteFile(lpNewFileName);
        }
    }
    int n = copyfile(lpExistingFileName, lpNewFileName, 0, COPYFILE_STAT | COPYFILE_DATA);
    return n == 0;
}

#else

bool copyFile(cstr_t lpExistingFileName, cstr_t lpNewFileName, bool bFailIfExists) {
    if (!bFailIfExists) {
        if (isFileExist(lpNewFileName)) {
            deleteFile(lpNewFileName);
        }
    }
    // TODO: TBD, int n = copyfile(lpExistingFileName, lpNewFileName, 0, COPYFILE_STAT | COPYFILE_DATA);
    return false;
}

#endif

uint32_t isDirWritable(cstr_t szDir) {
    return true;
}

//////////////////////////////////////////////////////////////////////////

FileFind::FileFind() {
    m_dirp = nullptr;
    m_dp = nullptr;
    m_bStatValid = false;
}

FileFind::~FileFind() {
    close();
}

bool FileFind::openDir(cstr_t szDir, cstr_t extFilter) {
    close();

    m_dp = opendir(szDir);
    m_dirp = nullptr;
    _tcscpy(m_szDir, szDir);

    return m_dp != nullptr;
}

void FileFind::close() {
    if (m_dp) {
        closedir(m_dp);
        m_dirp = nullptr;
        m_dp = nullptr;
    }
}

bool FileFind::findNext() {
    m_dirp = readdir(m_dp);
    m_bStatValid = false;

    while (m_dirp && m_dirp->d_name[0] == '.') {
        if (!(m_dirp->d_name[1] == '\0'
            || (m_dirp->d_name[1] == '.' && m_dirp->d_name[2] == '\0'))) {
            break;
        }
        m_dirp = readdir(m_dp);
    }

    return m_dirp != nullptr;
}

bool FileFind::isCurDir() {
    if (!getCurStat()) {
        return false;
    }

    return S_ISDIR(m_stat.st_mode);
}

bool FileFind::getCurStat() {
    if (m_bStatValid) {
        return true;
    }

    if (!m_dirp) {
        return false;
    }

    char szFile[400];
    _tcscpy(szFile, m_szDir);
    dirStringAddSep(szFile);
    _tcscat(szFile, m_dirp->d_name);

    memset(&m_stat, 0, sizeof(m_stat));
    m_bStatValid = (stat(szFile, &m_stat) == 0);

    return m_bStatValid;
}
