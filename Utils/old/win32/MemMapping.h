#if !defined(_MEM_MAPPING_H_)
#define _MEM_MAPPING_H_

#pragma once

class CMemMapping
{
public:
    CMemMapping()
    {
        m_hMap = nullptr;
        m_nSize = nullptr;
        m_lpData = nullptr;
    }
    virtual ~CMemMapping()
    {
        close();
    }

    bool create(cstr_t szName, int nSize)
    {
        m_hMap = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, nSize, szName);
        assert(m_hMap);
        if (!m_hMap)
            return false;

        m_lpData = uint8_t *MapViewOfFile(m_hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, nSize);
        assert(m_lpData);
        if (!m_lpData)
        {
            CloseHandle(m_hMap);
            m_hMap = nullptr;
            return false;
        }

        return true;
    }

    void close()
    {
        if (m_lpData)
        {
            UnmapViewOfFile(m_lpData);
            m_lpData = nullptr;
        }

        if (m_hMap)
        {
            CloseHandle(m_hMap);
            m_hMap = nullptr;
        }
    }

    uint8_t *getData() { return m_lpData; }

protected:
    HANDLE        m_hMap;
    int            m_nSize;
    uint8_t *m_lpData;

};

#endif // !defined(_MEM_MAPPING_H_)
