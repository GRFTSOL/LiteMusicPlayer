
#include "App.h"
#include "MiniDump.h"

static IMiniDumperNotify            *m_pDumpNotify = nullptr;

void null_invalid_parameter(
                        const wchar_t * expression,
                        const wchar_t * function, 
                        const wchar_t * file, 
                        int line,
                        uintptr_t pReserved
                        )
{
}

void CMiniDumper::init(IMiniDumperNotify *pDumpNotify)
{
    if (m_pDumpNotify)
        return;
    m_pDumpNotify = pDumpNotify;

#ifndef DEBUG
    // Disable debug invalid parameter prompt
    _set_invalid_parameter_handler((_invalid_parameter_handler)null_invalid_parameter);
#endif

    SetUnhandledExceptionFilter(TopLevelFilter);
}

LONG CMiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
    LONG retval = EXCEPTION_CONTINUE_SEARCH;
    HWND hParent = nullptr;                        // find a better value for your app

    if (!m_pDumpNotify)
        return EXCEPTION_CONTINUE_SEARCH;

    // firstly see if dbghelp.dll is around and has the function we need
    // look next to the EXE first, as the one in System32 might be old 
    // (e.g. Windows 2000)
    HMODULE hDll = nullptr;
    char    szDbgHelpPath[_MAX_PATH];

    if (getModulePath(szDbgHelpPath, getAppInstance()))
    {
        strcat_safe(szDbgHelpPath, CountOf(szDbgHelpPath), "DBGHELP.DLL");
        hDll = ::LoadLibrary(szDbgHelpPath);
    }

    // load any version we can
    if (hDll == nullptr)
        hDll = ::LoadLibrary("DBGHELP.DLL");
    if (!hDll)
        return EXCEPTION_CONTINUE_SEARCH;

    char szFaultingModule[MAX_PATH];
    HMODULE    hFaultModule;

    // get Except fault module
    if (!getModuleByAddress(pExceptionInfo->ExceptionRecord->ExceptionAddress, szFaultingModule, CountOf(szFaultingModule), hFaultModule))
        return EXCEPTION_CONTINUE_SEARCH;

    ERR_LOG1("Got crash: %s", szFaultingModule);

    // get MiniDumpWrite function ptr
    MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
    if (!pDump)
    {
        FreeLibrary(hDll);
        ERR_LOG0("DbgHelp.dll is too old.");
        return EXCEPTION_CONTINUE_SEARCH;
    }

    char szDumpPath[_MAX_PATH] = { 0 };

    // 
    if (!m_pDumpNotify->onBeginDump(hFaultModule, szDumpPath, CountOf(szDumpPath)))
        return EXCEPTION_CONTINUE_SEARCH;

    fileSetExt(szDumpPath, CountOf(szDumpPath), ".dmp");

    // create the file
    HANDLE hFile = ::CreateFile(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        FreeLibrary(hDll);
        ERR_LOG1("FAILED to create dump file: %s.", szDumpPath);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

    ExInfo.ThreadId = ::GetCurrentThreadId();
    ExInfo.ExceptionPointers = pExceptionInfo;
    ExInfo.ClientPointers = nullptr;

    // write the dump
    if (pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, nullptr, nullptr))
    {
        ERR_LOG1("save dump file to: %s.", szDumpPath);
        retval = EXCEPTION_EXECUTE_HANDLER;

        ::CloseHandle(hFile);

        // Notify mini dump file is saved.
        if (!m_pDumpNotify->onDumpFinished(hFaultModule, szDumpPath))
            return EXCEPTION_CONTINUE_SEARCH;
    }
    else
    {
        ::CloseHandle(hFile);
        ERR_LOG0("Failed to write mini dump file.");
    }

    return retval;
}

bool CMiniDumper::getModuleByAddress(PVOID addr, char * szModule, int nLen, HMODULE &hModule)
{
    MEMORY_BASIC_INFORMATION mbi;

    if (!VirtualQuery(addr, &mbi, sizeof(mbi)))
        return false;

    uint32_t hMod = (uint32_t)mbi.AllocationBase;

    if (!GetModuleFileName((HMODULE)hMod, szModule, nLen))
        return false;

    hModule = GetModuleHandle(szModule);
    if (!hModule)
        return false;

    return true;
}
