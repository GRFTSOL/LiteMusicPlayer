#include "../Utils.h"
#include "../LogAlias.h"
#include "../../TinyJS/utils/CharEncoding.h"
#include "MiniDump.h"


static string g_fnDumpToSave;
static FuncDumpFinished g_callbackDumpFinished = nullptr;

void null_invalid_parameter(
    const WCHAR * expression,
    const WCHAR * function,
    const WCHAR * file,
    int line,
    uintptr_t pReserved
    ) {
}

void MiniDumper::init(cstr_t fnDumpToSave, FuncDumpFinished callback) {
    assert(fnDumpToSave);
    assert(callback);
    g_fnDumpToSave = fnDumpToSave;
    g_callbackDumpFinished = callback;

#ifndef DEBUG
    // Disable debug invalid parameter prompt
    _set_invalid_parameter_handler((_invalid_parameter_handler)null_invalid_parameter);
#endif

    SetUnhandledExceptionFilter(topLevelFilter);
}

LONG MiniDumper::topLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo) {
    LONG retval = EXCEPTION_CONTINUE_SEARCH;
    HWND hParent = nullptr; // find a better value for your app

    if (!g_callbackDumpFinished || g_fnDumpToSave.empty()) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // firstly see if dbghelp.dll is around and has the function we need
    // look next to the EXE first, as the one in System32 might be old
    // (e.g. Windows 2000)
    HMODULE hDll = ::LoadLibraryA("dbghelp.dll");
    if (!hDll) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // get MiniDumpWrite function ptr
    MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
    if (!pDump) {
        FreeLibrary(hDll);
        ERR_LOG0("DbgHelp.dll is too old.");
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // create the file
    HANDLE hFile = ::CreateFileW(utf8ToUCS2(g_fnDumpToSave.c_str()).c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        FreeLibrary(hDll);
        ERR_LOG1("FAILED to create dump file: %s.", g_fnDumpToSave.c_str());
        return EXCEPTION_CONTINUE_SEARCH;
    }

    _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

    ExInfo.ThreadId = ::GetCurrentThreadId();
    ExInfo.ExceptionPointers = pExceptionInfo;
    ExInfo.ClientPointers = false;

    // write the dump
    if (pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, nullptr, nullptr)) {
        ERR_LOG1("save dump file to: %s.", g_fnDumpToSave.c_str());
        retval = EXCEPTION_EXECUTE_HANDLER;

        ::CloseHandle(hFile);

        // Notify mini dump file is saved.
        g_callbackDumpFinished(g_fnDumpToSave.c_str());
    } else {
        ::CloseHandle(hFile);
        ERR_LOG0("Failed to write mini dump file.");
    }

    return retval;
}

/*
bool MiniDumper::getModuleByAddress(PVOID addr, char *szModule, int nLen, HMODULE &hModule) {
    MEMORY_BASIC_INFORMATION mbi;

    if (!VirtualQuery(addr, &mbi, sizeof(mbi))) {
        return false;
    }

    uint32_t hMod = (uint32_t)mbi.AllocationBase;

    if (!GetModuleFileName((HMODULE)hMod, szModule, nLen)) {
        return false;
    }

    hModule = GetModuleHandle(szModule);
    if (!hModule) {
        return false;
    }

    return true;
}
*/