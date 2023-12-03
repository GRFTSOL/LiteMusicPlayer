#include "IMPlayer.h"
#include "MILocalFile.h"


CMILocalFile::CMILocalFile() {
    OBJ_REFERENCE_INIT;

    m_fp = nullptr;
}

CMILocalFile::~CMILocalFile() {
    close();
}

ResultCode CMILocalFile::open(cstr_t szSourceMedia) {
    m_fp = fopen(szSourceMedia, "rb");
    if (m_fp == nullptr) {
        if (!isFileExist(szSourceMedia)) {
            return ERR_MI_NOT_FOUND;
        }
        return ERR_MI_OPEN_SRC;
    }

    m_strFile = szSourceMedia;

    return ERR_OK;
}

uint32_t CMILocalFile::read(void *lpBuffer, uint32_t dwSize) {
    return (uint32_t)fread(lpBuffer, 1, dwSize, m_fp);
}

ResultCode CMILocalFile::seek(uint32_t dwOffset, int nOrigin) {
    if (fseek(m_fp, dwOffset, nOrigin) == 0) {
        return ERR_OK;
    } else {
        return ERR_MI_SEEK;
    }
}

ResultCode CMILocalFile::getSize(uint32_t &dwSize) {
    auto dwPos = (uint32_t)ftell(m_fp);
    if (fseek(m_fp, 0, SEEK_END) == 0) {
        dwSize = (uint32_t)ftell(m_fp);
        fseek(m_fp, dwPos, SEEK_SET);
        return ERR_OK;
    } else {
        return ERR_MI_SEEK;
    }
}

uint32_t CMILocalFile::getPos() {
    return (uint32_t)ftell(m_fp);
}

bool CMILocalFile::isEOF() {
    if (feof(m_fp)) {
        return true;
    } else {
        return false;
    }
}

bool CMILocalFile::isError() {
    return ferror(m_fp) != 0;
}

void CMILocalFile::close() {
    if (m_fp) {
        fclose(m_fp);
        m_fp = nullptr;
    }
}

cstr_t CMILocalFile::getSource() {
    return m_strFile.c_str();
}
