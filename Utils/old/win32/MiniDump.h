#pragma once

#include "dbghelp.h"


class IMiniDumperNotify {
public:
    virtual bool onBeginDump(HMODULE hCrashMod, char szDumpFileToSave[], int nLen) = 0;
    virtual bool onDumpFinished(HMODULE hCrashMod, cstr_t szDumpFileToSave) = 0;

};

// based on dbghelp.h
typedef bool (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, uint32_t dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
    CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
    );

class CMiniDumper {
public:
    static void init(IMiniDumperNotify *pDumpNotify);

private:
    static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);

    static bool getModuleByAddress(PVOID addr, char * szModule, int nLen, HMODULE &hModule);

};
