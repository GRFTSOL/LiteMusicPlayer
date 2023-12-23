#pragma once

#include "dbghelp.h"


typedef void (*FuncDumpFinished)(cstr_t fnDumpSaved);

// based on dbghelp.h
typedef bool (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, uint32_t dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
    CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
    );

class MiniDumper {
public:
    static void init(cstr_t fnDumpToSave, FuncDumpFinished callback);

private:
    static LONG WINAPI topLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);

    // static bool getModuleByAddress(PVOID addr, char * szModule, int nLen, HMODULE &hModule);

};
