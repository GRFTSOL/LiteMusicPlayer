#pragma once

﻿
#include <sys/mman.h>


class CMemmapFile {
public:
    CMemmapFile();
    virtual ~CMemmapFile();

    bool open(cstr_t szFile, int nSizeMin, bool bMustExist = false);

    bool resize(size_t nSize);

    size_t getSize() { return m_nFileSize; }

    void close();

    bool isValid() const { return m_lpFile != nullptr; }

    uint8_t *getPtr() {
        return uint8_t *m_lpFile;
    }

protected:
    int                         m_file;
    size_t                      m_nFileSize;        // 文件大小
    void                        *       m_lpFile;   // 文件映射之后的文件内容

};
