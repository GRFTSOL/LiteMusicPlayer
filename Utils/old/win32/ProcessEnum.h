#pragma once

class CProcessEnum {
public:
    CProcessEnum();
    ~CProcessEnum();

    bool enum(uint32_t * lpidProcess, uint32_t cb, LPDWORD lpcbNeeded);

    uint32_t getModuleFileNameEx(HANDLE hProcess, HMODULE hModule, char * lpFilename, uint32_t nSize);
    string getProcessFileName(uint32_t dwProcessID);

protected:
    typedef bool (WINAPI *EnumProcessesFunc) (uint32_t * lpidProcess, uint32_t cb, LPDWORD lpcbNeeded);
    typedef uint32_t (WINAPI *GetModuleFileNameExWFunc)(HANDLE hProcess, HMODULE hModule, char * lpFilename, uint32_t nSize);

    HMODULE                     m_hPsApi;
    EnumProcessesFunc           m_funcEnum;
    GetModuleFileNameExWFunc    m_funcGetModuleFileNameEx;

};
