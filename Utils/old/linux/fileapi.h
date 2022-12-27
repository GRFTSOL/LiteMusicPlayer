#pragma once

bool deleteFile(cstr_t lpFileName);
bool removeDirectory(cstr_t lpPathName);
bool moveFile(const char *oldname, const char *newname);
bool copyFile(const char *oldname, const char *newname, bool bFailIfExists);

class FileFind {
public:
    FileFind();
    virtual ~FileFind();

    bool openDir(cstr_t szDir, cstr_t extFilter = nullptr);

    void close();

    bool findNext();

    cstr_t getCurName() { return m_dirp->d_name; }

    bool isCurDir();

protected:
    bool getCurStat();

    dirent                      *m_dirp;
    DIR                         *m_dp;
    bool                        m_bStatValid;
    struct stat    m_stat;
    char                        m_szDir[MAX_PATH];

};
