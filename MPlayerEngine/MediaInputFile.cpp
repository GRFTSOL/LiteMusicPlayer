#include "IMPlayer.h"
#include "MediaInputFile.h"


MediaInputFile::MediaInputFile() {
    OBJ_REFERENCE_INIT;

    m_fp = nullptr;
}

MediaInputFile::~MediaInputFile() {
    close();
}

ResultCode MediaInputFile::open(cstr_t szSourceMedia) {
    m_fp = fopenUtf8(szSourceMedia, "rb");
    if (m_fp == nullptr) {
        if (!isFileExist(szSourceMedia)) {
            return ERR_MI_NOT_FOUND;
        }
        return ERR_MI_OPEN_SRC;
    }

    m_strFile = szSourceMedia;

    return ERR_OK;
}

uint32_t MediaInputFile::read(void *lpBuffer, uint32_t dwSize) {
    return (uint32_t)fread(lpBuffer, 1, dwSize, m_fp);
}

ResultCode MediaInputFile::seek(uint32_t dwOffset, int nOrigin) {
    if (fseek(m_fp, dwOffset, nOrigin) == 0) {
        return ERR_OK;
    } else {
        return ERR_MI_SEEK;
    }
}

ResultCode MediaInputFile::getSize(uint32_t &dwSize) {
    auto dwPos = (uint32_t)ftell(m_fp);
    if (fseek(m_fp, 0, SEEK_END) == 0) {
        dwSize = (uint32_t)ftell(m_fp);
        fseek(m_fp, dwPos, SEEK_SET);
        return ERR_OK;
    } else {
        return ERR_MI_SEEK;
    }
}

uint32_t MediaInputFile::getPos() {
    return (uint32_t)ftell(m_fp);
}

bool MediaInputFile::isEOF() {
    if (feof(m_fp)) {
        return true;
    } else {
        return false;
    }
}

bool MediaInputFile::isError() {
    return ferror(m_fp) != 0;
}

void MediaInputFile::close() {
    if (m_fp) {
        fclose(m_fp);
        m_fp = nullptr;
    }
}

cstr_t MediaInputFile::getSource() {
    return m_strFile.c_str();
}
