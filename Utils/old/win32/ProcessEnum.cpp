#include "base.h"
#include "ProcessEnum.h"

CProcessEnum::CProcessEnum()
{
    m_funcEnum = nullptr; m_funcGetModuleFileNameEx = nullptr;

    m_hPsApi = LoadLibrary("psapi.dll");
    assert(m_hPsApi);
    if (!m_hPsApi)
        return;

    m_funcEnum = (EnumProcessesFunc)GetProcAddress(m_hPsApi, "EnumProcesses");
#ifdef UNICODE
    m_funcGetModuleFileNameEx = (GetModuleFileNameExWFunc)GetProcAddress(m_hPsApi, "GetModuleFileNameExW");
#else
    m_funcGetModuleFileNameEx = (GetModuleFileNameExWFunc)GetProcAddress(m_hPsApi, "GetModuleFileNameExA");
#endif
}
CProcessEnum::~CProcessEnum()
{
    FreeLibrary(m_hPsApi);
}

bool CProcessEnum::enum(uint32_t * lpidProcess, uint32_t cb, LPDWORD lpcbNeeded)
{
    if (m_funcEnum)
        return m_funcEnum(lpidProcess, cb, lpcbNeeded);
    else
        return false;
}

uint32_t CProcessEnum::getModuleFileNameEx(HANDLE hProcess, HMODULE hModule, char * lpFilename, uint32_t nSize)
{
    if (m_funcGetModuleFileNameEx)
        return m_funcGetModuleFileNameEx(hProcess, hModule, lpFilename, nSize);
    else
        return 0;
}

string CProcessEnum::getProcessFileName(uint32_t dwProcessID)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwProcessID);
    // assert(hProcess);
    if (hProcess)
    {
        char    szFile[MAX_PATH];
        int n = getModuleFileNameEx(hProcess, nullptr, szFile, CountOf(szFile));
        if (n > 0)
            return szFile;
    }

    return "";
}
