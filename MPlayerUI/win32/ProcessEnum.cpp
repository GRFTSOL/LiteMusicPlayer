#include "../../TinyJS/utils/UtilsTypes.h"
#include "../../TinyJS/utils/CharEncoding.h"
#include "ProcessEnum.h"


class ProcessEnum {
public:
    ProcessEnum() {
        m_hPsApi = LoadLibraryA("psapi.dll");
        assert(m_hPsApi);
        if (!m_hPsApi) {
            return;
        }

        m_funcEnum = (EnumProcessesFunc)GetProcAddress(m_hPsApi, "EnumProcesses");    
        m_funcGetModuleFileNameEx = (GetModuleFileNameExWFunc)GetProcAddress(m_hPsApi, "GetModuleFileNameExW");
    }

    ~ProcessEnum() {
        FreeLibrary(m_hPsApi);
    }

    VecInts enumAll() {
        DWORD buf[2048];
        DWORD sizeReturned = 0;

        VecInts processes;
        if (m_funcEnum && m_funcEnum(buf, sizeof(buf), &sizeReturned)) {
			sizeReturned /= sizeof(uint32_t);
            for (uint32_t i = 0; i < sizeReturned; i++) {
                processes.push_back(buf[i]);
            }
        }

        return processes;
    }

    std::string getModuleFileNameEx(HANDLE hProcess, HMODULE hModule) {
		WCHAR filename[MAX_PATH];
        if (m_funcGetModuleFileNameEx) {
            int size = m_funcGetModuleFileNameEx(hProcess, hModule, filename, CountOf(filename));
			return ucs2ToUtf8(filename, size);
		}

        return std::string();
    }

    std::string getProcessFileName(uint32_t pid) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
			return getModuleFileNameEx(hProcess, NULL);
        }

        return std::string();
    }

protected:
    typedef BOOL (WINAPI *EnumProcessesFunc)(DWORD *lpidProcess, DWORD cb, LPDWORD lpcbNeeded);
    typedef DWORD (WINAPI *GetModuleFileNameExWFunc)(HANDLE hProcess, HMODULE hModule, WCHAR *lpFilename, DWORD nSize);

    HMODULE                     	m_hPsApi = nullptr;
    EnumProcessesFunc           	m_funcEnum = nullptr;
    GetModuleFileNameExWFunc    	m_funcGetModuleFileNameEx = nullptr;

};

VecInts listAllProcesses() {
    return ProcessEnum().enumAll();
}

std::string getProcessFileName(uint32_t pid) {
    return ProcessEnum().getProcessFileName(pid);
}
