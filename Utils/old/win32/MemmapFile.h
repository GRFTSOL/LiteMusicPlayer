#pragma once

class CMemmapFile  
{
public:
    CMemmapFile();
    virtual ~CMemmapFile();

    bool open(cstr_t szFile, int nSizeMin, bool bMustExist = false);

    bool resize(size_t nSize);

    int getSize() { return m_nFileSize; }

    void close();

    bool isValid() const { return m_lpFile != nullptr; }

    uint8_t *getPtr()
    {
        return m_lpFile;
    }

protected:
    HANDLE        m_hFile;            // 文件句柄
    HANDLE        m_hFilemap;            // 文件映射句柄
    int            m_nFileSize;        // 文件大小
    uint8_t *m_lpFile;            // 文件映射之后的文件内容

};
