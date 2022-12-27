#include "base.h"
#include "fileapi.h"
#include "MemmapFile.h"


CMemmapFile::CMemmapFile() {
    m_hFile = nullptr;
    m_hFilemap = nullptr;
    m_nFileSize = 0;
    m_lpFile = nullptr;
}

CMemmapFile::~CMemmapFile() {
    close();
}

bool CMemmapFile::open(cstr_t szFile, int nSizeMin, bool bMustExist) {
    if (isFileExist(szFile)) {
        m_hFile = CreateFile(szFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        assert(m_hFile != INVALID_HANDLE_VALUE);
        if (m_lpFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        m_nFileSize = getFileSize(m_hFile, nullptr);
        if (m_nFileSize <= nSizeMin) {
            m_nFileSize = nSizeMin;
        }
    } else {
        if (bMustExist) {
            return false;
        }

        m_nFileSize = nSizeMin;
        m_hFile = CreateFile(szFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        assert(m_hFile != INVALID_HANDLE_VALUE);
        if (m_lpFile == INVALID_HANDLE_VALUE) {
            return false;
        }
    }

    m_hFilemap = CreateFileMapping(m_hFile, nullptr, PAGE_READWRITE, 0, m_nFileSize, nullptr);
    assert(m_hFilemap);
    if (!m_hFilemap) {
        CloseHandle(m_hFile);
        m_hFile = nullptr;
        return false;
    }

    m_lpFile = uint8_t *MapViewOfFile(m_hFilemap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, m_nFileSize);
    assert(m_lpFile);
    if (!m_lpFile) {
        CloseHandle(m_hFile);
        m_hFile = nullptr;
        CloseHandle(m_hFilemap);
        m_hFilemap = nullptr;
        return false;
    }

    return true;
}

bool CMemmapFile::resize(size_t nSize) {
    if (m_lpFile == nullptr) {
        return false;
    }

    if (nSize < 1024) {
        nSize = 1024;
    }

    // m_nFileSize = (uint32_t)(m_nFileSize * FI_FREE_RATE);
    m_nFileSize = nSize + sizeof(int);
    m_nFileSize -= m_nFileSize % sizeof(int);

    if (m_lpFile) {
        UnmapViewOfFile(m_lpFile);
        m_lpFile = nullptr;
    }

    if (m_hFilemap) {
        CloseHandle(m_hFilemap);
        m_hFilemap = nullptr;
    }

    m_hFilemap = CreateFileMapping(m_hFile, nullptr, PAGE_READWRITE, 0, m_nFileSize, nullptr);
    assert(m_hFilemap);
    if (!m_hFilemap) {
        CloseHandle(m_hFile);
        m_lpFile = nullptr;
        return false;
    }

    m_lpFile = uint8_t *MapViewOfFile(m_hFilemap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, m_nFileSize);
    assert(m_lpFile);
    if (!m_lpFile) {
        CloseHandle(m_hFile);
        CloseHandle(m_hFilemap);
        m_lpFile = nullptr;
        m_hFilemap = nullptr;
        return false;
    }

    return true;
}

void CMemmapFile::close() {
    if (m_lpFile) {
        UnmapViewOfFile(m_lpFile);
        m_lpFile = nullptr;
    }

    if (m_hFilemap) {
        CloseHandle(m_hFilemap);
        m_hFilemap = nullptr;
    }

    if (m_hFile) {
        CloseHandle(m_hFile);
        m_hFile = nullptr;
    }
}
