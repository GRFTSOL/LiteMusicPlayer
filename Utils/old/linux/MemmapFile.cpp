#include "../stdafx.h"
#include "../base.h"
#include "fileapi.h"
#include "MemmapFile.h"
#include <sys/mman.h>


CMemmapFile::CMemmapFile() {
    m_file = 0;
    m_nFileSize = 0;
    m_lpFile = nullptr;
}

CMemmapFile::~CMemmapFile() {
    close();
}

bool CMemmapFile::open(cstr_t szFile, int nSizeMin, bool bMustExist) {
    if (isFileExist(szFile)) {
        struct stat        filestat;

        m_file = open(szFile, O_RDWR);
        if (m_file < 0) {
            return false;
        }

        memset(&filestat, 0, sizeof(filestat));
        if (fstat(m_file, &filestat) < 0) {
            close();
            return false;
        }

        m_nFileSize = filestat.st_size;
        if (m_nFileSize < nSizeMin) {
            m_nFileSize = nSizeMin;
            if (lseek(m_file, m_nFileSize - 1, SEEK_SET) == -1) {
                close();
                return false;
            }

            if (write(m_file, "", 1) != 1) {
                close();
                return false;
            }
        }
    } else {
        if (bMustExist) {
            return false;
        }

        m_file = open(szFile, O_RDWR);
        if (m_file < 0) {
            return false;
        }

        m_nFileSize = nSizeMin;
        if (lseek(m_file, m_nFileSize - 1, SEEK_SET) == -1) {
            close();
            return false;
        }

        if (write(m_file, "", 1) != 1) {
            close();
            return false;
        }
    }

    m_lpFile = mmap(0, m_nFileSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, m_file, 0);
    if (m_lpFile == (void *)-1) {
        m_lpFile = mmap(0, m_nFileSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, m_file, 0);
        if (m_lpFile == (void *)-1) {
            m_lpFile = nullptr;
            close();
            return false;
        }
    }

    return true;
}

void CMemmapFile::close() {
    if (m_lpFile) {
        munmap(m_lpFile, m_nFileSize);
        m_lpFile = nullptr;
    }

    if (m_file >= 0) {
        close(m_file);
        m_file = -1;
    }
}
