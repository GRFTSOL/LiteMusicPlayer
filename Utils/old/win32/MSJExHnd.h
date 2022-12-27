#pragma once

#ifndef __MSJEXHND_H__
#define __MSJEXHND_H__
#include <imagehlp.h>


typedef bool (*FunCrashCallback)(cstr_t szMsg, HMODULE hCrashMod, void *lpUserData);

class MSJExceptionHandler {
public:

    MSJExceptionHandler( );
    ~MSJExceptionHandler( );

    void init(FunCrashCallback funCallback, void *lpUserData);

private:

    // entry point where control comes on an unhandled exception
    static LONG WINAPI MSJUnhandledExceptionFilter(
        PEXCEPTION_POINTERS pExceptionInfo );

    // where report info is extracted and generated
    static void generateExceptionReport( PEXCEPTION_POINTERS pExceptionInfo, HMODULE &hFaultingMod);

    // Helper functions
    static cstr_t getExceptionString( uint32_t dwCode );
    static bool getLogicalAddress(PVOID addr, PTSTR szModule, uint32_t len,
        uint32_t& section, uint32_t& offset );
    static void intelStackWalk( PCONTEXT pContext );
#if 1
    static void imagehlpStackWalk( PCONTEXT pContext );
#endif
    static int __cdecl _tprintf(const char * format, ...);

#if 1
    static bool initImagehlpFunctions( void );
#endif

    // Variables used by the class
    // static char m_szLogFileName[MAX_PATH];
    static LPTOP_LEVEL_EXCEPTION_FILTER m_previousFilter;
    // static HANDLE m_hReportFile;

#if 1
    // Make typedefs for some IMAGEHLP.DLL functions so that we can use them
    // with GetProcAddress
    typedef bool (__stdcall * SYMINITIALIZEPROC)( HANDLE, char *, bool );
    typedef bool (__stdcall *SYMCLEANUPPROC)( HANDLE );

    typedef bool (__stdcall * STACKWALKPROC)
    ( uint32_t, HANDLE, HANDLE, LPSTACKFRAME, void *,
        PREAD_PROCESS_MEMORY_ROUTINE,PFUNCTION_TABLE_ACCESS_ROUTINE,
        PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE );

    typedef void *(__stdcall *SYMFUNCTIONTABLEACCESSPROC)( HANDLE, uint32_t );

    typedef uint32_t (__stdcall *SYMGETMODULEBASEPROC)( HANDLE, uint32_t );

    typedef bool (__stdcall *SYMGETSYMFROMADDRPROC)
    ( HANDLE, uint32_t, PDWORD, PIMAGEHLP_SYMBOL );

    static SYMINITIALIZEPROC    _SymInitialize;
    static SYMCLEANUPPROC       _SymCleanup;
    static STACKWALKPROC        _StackWalk;
    static SYMFUNCTIONTABLEACCESSPROC   _SymFunctionTableAccess;
    static SYMGETMODULEBASEPROC         _SymGetModuleBase;
    static SYMGETSYMFROMADDRPROC        _SymGetSymFromAddr;

#endif

    static FunCrashCallback     m_funCallback;
    static void                 *                       m_lpUserData;
    static string               _strReport;

};

extern MSJExceptionHandler g_CrashHandler; // global instance of class

#endif
