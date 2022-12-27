//==========================================
// Matt Pietrek
// Microsoft Systems Journal, May 1997
// FILE: MSJEXHND.CPP
//==========================================
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include "msjexhnd.h"

#include <vector>


using namespace std;


//============================== Global Variables =============================

//
// Declare the static variables of the MSJExceptionHandler class
//
// char MSJExceptionHandler::m_szLogFileName[MAX_PATH];
LPTOP_LEVEL_EXCEPTION_FILTER MSJExceptionHandler::m_previousFilter;
// HANDLE MSJExceptionHandler::m_hReportFile;

MSJExceptionHandler::SYMINITIALIZEPROC MSJExceptionHandler::_SymInitialize = 0;
MSJExceptionHandler::SYMCLEANUPPROC MSJExceptionHandler::_SymCleanup = 0;
MSJExceptionHandler::STACKWALKPROC MSJExceptionHandler::_StackWalk = 0;

MSJExceptionHandler::SYMFUNCTIONTABLEACCESSPROC
MSJExceptionHandler::_SymFunctionTableAccess = 0;

MSJExceptionHandler::SYMGETMODULEBASEPROC
MSJExceptionHandler::_SymGetModuleBase = 0;

MSJExceptionHandler::SYMGETSYMFROMADDRPROC
MSJExceptionHandler::_SymGetSymFromAddr = 0;

FunCrashCallback MSJExceptionHandler::m_funCallback = nullptr;
void * MSJExceptionHandler::m_lpUserData = nullptr;
string MSJExceptionHandler::_strReport;

MSJExceptionHandler g_CrashHandler; // Declare global instance of class

//============================== Class Methods =============================

//=============
// Constructor
//=============
MSJExceptionHandler::MSJExceptionHandler( ) {
}

//============
// Destructor
//============
MSJExceptionHandler::~MSJExceptionHandler( ) {
    if (m_previousFilter) {
        SetUnhandledExceptionFilter( m_previousFilter );
    }
}

void MSJExceptionHandler::init(FunCrashCallback funCallback, void *lpUserData) {
    // add this filter in the exception callback chain
    // install the unhandled exception filter function
    m_previousFilter = SetUnhandledExceptionFilter(MSJUnhandledExceptionFilter);

    //    // Figure out what the report file will be named, and store it away
    //    GetModuleFileName( 0, m_szLogFileName, MAX_PATH );
    //
    //    // Look for the '.' before the "EXE" extension.  Replace the extension
    //    // with "RPT"
    //    PTSTR pszDot = strrchr( m_szLogFileName, '.' );
    //    if ( pszDot )
    //    {
    //        pszDot++;   // advance past the '.'
    //        if ( strlen(pszDot) >= 3 )
    //            _tcscpy( pszDot, "RPT" );   // "RPT" -> "report"
    //    }

    m_funCallback = funCallback;
    m_lpUserData = lpUserData;
}

//===========================================================
// Entry point where control comes on an unhandled exception
//===========================================================
LONG WINAPI MSJExceptionHandler::MSJUnhandledExceptionFilter(
    PEXCEPTION_POINTERS pExceptionInfo ) {
    /*
    char    szLogFile[MAX_PATH];
    for (int i = 0; i < 10; i++)
    {
        sprintf(szLogFile, "%s%d", m_szLogFileName, i);
        if (!isFileExist(szLogFile))
        {
            break;
        }
    }
    char    szDelLogFile[MAX_PATH];
    sprintf(szDelLogFile, "%s%d", m_szLogFileName, (i + 1) % 10);
    deleteFile(szDelLogFile);

*/
    /*    HANDLE        m_hReportFile;
    char        szLogFile[MAX_PATH];
    GetModuleFileName(nullptr, szLogFile, MAX_PATH);
    fileSetExt(szLogFile, ".txt");
    m_hReportFile = CreateFile(szLogFile,
        GENERIC_WRITE,
        0,
        0,
        OPEN_ALWAYS,
        FILE_FLAG_WRITE_THROUGH,
        0 );

    if ( m_hReportFile )
    {
        uint32_t    dwWritten;
        SetFilePointer(m_hReportFile, 0, 0, FILE_END);
        WriteFile(m_hReportFile, "Exception catched!", 5, &dwWritten, nullptr);
        CloseHandle(m_hReportFile);
    }

    ERR_LOG0("Exception catched!");*/
    //        CloseHandle( m_hReportFile );
    //        m_hReportFile = 0;
    HMODULE hFaultingMod = nullptr;
    _strReport = "";
    generateExceptionReport( pExceptionInfo, hFaultingMod);
    if (m_funCallback(_strReport.c_str(), hFaultingMod, m_lpUserData)) {
        return EXCEPTION_EXECUTE_HANDLER;
    }
    //    }

    if ( m_previousFilter ) {
        return m_previousFilter( pExceptionInfo );
    } else {
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

//===========================================================================
// open the report file, and write the desired information to it.  Called by
// MSJUnhandledExceptionFilter
//===========================================================================
void MSJExceptionHandler::generateExceptionReport(PEXCEPTION_POINTERS pExceptionInfo, HMODULE &hFaultingMod) {
    // start out with a banner
    // _tprintf( "//=====================================================\n" );
    // CTime tmNow = CTime::getCurrentTime();
    SYSTEMTIME SysTime;
    getLocalTime(&SysTime);
    _tprintf( "Timestamp: %d-%d-%d %d:%d:%d\n", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
    PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

    // First print information about the type of fault
    _tprintf(   "Exception code: %08X %s\n",
        pExceptionRecord->ExceptionCode,
        getExceptionString(pExceptionRecord->ExceptionCode) );

    // Now print information about where the fault occured
    char szFaultingModule[MAX_PATH];
    uint32_t section, offset;
    if (!getLogicalAddress(  pExceptionRecord->ExceptionAddress,
        szFaultingModule,
        sizeof( szFaultingModule ),
        section, offset )) {
        return;
    }

    _tprintf( "Fault address:  %08X %02X:%08X %s\n",
        pExceptionRecord->ExceptionAddress,
        section, offset, szFaultingModule );
    //
    //    _tprintf( "Fault Thread:  %08X\n",
    //        GetCurrentThread());

    hFaultingMod = GetModuleHandle(szFaultingModule);

    PCONTEXT pCtx = pExceptionInfo->ContextRecord;

    if ( !initImagehlpFunctions() ) {
        OutputDebugString("IMAGEHLP.DLL or its exported procs not found");

#ifdef _M_IX86  // Intel Only!
        // Walk the stack using x86 specific code
        intelStackWalk( pCtx );
#endif
    } else {
        imagehlpStackWalk( pCtx );

        _SymCleanup( GetCurrentProcess() );
    }

    // show the registers
#ifdef _M_IX86  // Intel Only!
    _tprintf( "\nRegisters:\n" );

    _tprintf("EAX:%08X\nEBX:%08X\nECX:%08X\nEDX:%08X\nESI:%08X\nEDI:%08X\n",
        pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi );

    _tprintf( "CS:EIP:%04X:%08X\n", pCtx->SegCs, pCtx->Eip );
    _tprintf( "SS:ESP:%04X:%08X  EBP:%08X\n",
        pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
    _tprintf( "DS:%04X  ES:%04X  FS:%04X  GS:%04X\n",
        pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs );
    _tprintf( "Flags:%08X\n", pCtx->EFlags );

#endif

    _tprintf( "\n" );
}

//======================================================================
// Given an exception code, returns a pointer to a static string with a
// description of the exception
//======================================================================
cstr_t MSJExceptionHandler::getExceptionString( uint32_t dwCode ) {
#define EXCEPTION( x )      case EXCEPTION_##x: return #x;

    switch ( dwCode ) {
        EXCEPTION( ACCESS_VIOLATION )
        EXCEPTION( DATATYPE_MISALIGNMENT )
        EXCEPTION( BREAKPOINT )
        EXCEPTION( SINGLE_STEP )
        EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
        EXCEPTION( FLT_DENORMAL_OPERAND )
        EXCEPTION( FLT_DIVIDE_BY_ZERO )
        EXCEPTION( FLT_INEXACT_RESULT )
        EXCEPTION( FLT_INVALID_OPERATION )
        EXCEPTION( FLT_OVERFLOW )
        EXCEPTION( FLT_STACK_CHECK )
        EXCEPTION( FLT_UNDERFLOW )
        EXCEPTION( INT_DIVIDE_BY_ZERO )
        EXCEPTION( INT_OVERFLOW )
        EXCEPTION( PRIV_INSTRUCTION )
        EXCEPTION( IN_PAGE_ERROR )
        EXCEPTION( ILLEGAL_INSTRUCTION )
        EXCEPTION( NONCONTINUABLE_EXCEPTION )
        EXCEPTION( STACK_OVERFLOW )
        EXCEPTION( INVALID_DISPOSITION )
        EXCEPTION( GUARD_PAGE )
        EXCEPTION( INVALID_HANDLE )
    }

    // If not one of the "known" exceptions, try to get the string
    // from NTDLL.DLL's message table.

    static char szBuffer[512] = { 0 };

    FormatMessage(  FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
        GetModuleHandle( "NTDLL.DLL" ),
        dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

    return szBuffer;
}

//==============================================================================
// Given a linear address, locates the module, section, and offset containing
// that address.
//
// Note: the szModule paramater buffer is an output buffer of length specified
// by the len parameter (in characters!)
//==============================================================================
bool MSJExceptionHandler::getLogicalAddress(
    PVOID addr, PTSTR szModule, uint32_t len, uint32_t& section, uint32_t& offset ) {
    MEMORY_BASIC_INFORMATION mbi;

    if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) ) {
        return false;
    }

    uint32_t hMod = (uint32_t)mbi.AllocationBase;

    if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) ) {
        return false;
    }

    hMod = (uint32_t)GetModuleHandle(szModule);
    if (!hMod) {
        return false;
    }

    // Point to the DOS header in memory
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

    // From the DOS header, find the NT (PE) header
    PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

    uint32_t rva = (uint32_t)addr - hMod; // RVA is offset from module load address

    // Iterate through the section table, looking for the one that encompasses
    // the linear address.
    for (   unsigned i = 0;
    i < pNtHdr->FileHeader.NumberOfSections;
    i++, pSection++ )
    {
        uint32_t sectionStart = pSection->VirtualAddress;
        uint32_t sectionEnd = sectionStart
        + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

        // Is the address in this section???
        if ( (rva >= sectionStart) && (rva <= sectionEnd) ) {
            // Yes, address is in the section.  Calculate section and offset,
            // and store in the "section" & "offset" params, which were
            // passed by reference.
            section = i+1;
            offset = rva - sectionStart;
            return true;
        }
    }

    return false; // Should never get here!
}

//============================================================
// Walks the stack, and writes the results to the report file
//============================================================
void MSJExceptionHandler::intelStackWalk( PCONTEXT pContext ) {
    _tprintf( "\nCall stack:\n" );

    _tprintf( "Address   Frame     Logical addr  Module\n" );

    uint32_t pc = pContext->Eip;
    PDWORD pFrame, pPrevFrame;

    pFrame = (PDWORD)pContext->Ebp;

    do {
        char szModule[MAX_PATH] = "";
        uint32_t section = 0, offset = 0;

        if (!getLogicalAddress((PVOID)pc, szModule,sizeof(szModule),section,offset )) {
            break;
        }

        // Can two DWORDs be read from the supposed frame address?
        if ( IsBadWritePtr(pFrame, sizeof(PVOID)*2) ) {
            break;
        }

        _tprintf( "%08X  %08X  %04X:%08X %s\n",
            pc, pFrame, section, offset, szModule );

        pc = pFrame[1];

        pPrevFrame = pFrame;

        pFrame = (PDWORD)pFrame[0]; // proceed to next higher frame on stack

        // Frame pointer must be aligned on a uint32_t boundary.  Bail if not so.
        if ( (uint32_t)pFrame & 3 ) {
            break;
        }

        if ( pFrame <= pPrevFrame ) {
            break;
        }
    }
    while ( 1 );
}

//============================================================
// Walks the stack, and writes the results to the report file
//============================================================
void MSJExceptionHandler::imagehlpStackWalk( PCONTEXT pContext ) {
    _tprintf( "\nCall stack:\n" );

    _tprintf( "Address   Frame\n" );

    // Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag

    STACKFRAME sf;
    memset( &sf, 0, sizeof(sf) );

    // Initialize the STACKFRAME structure for the first call.  This is only
    // necessary for Intel CPUs, and isn't mentioned in the documentation.
    sf.AddrPC.offset = pContext->Eip;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrStack.offset = pContext->Esp;
    sf.AddrStack.Mode = AddrModeFlat;
    sf.AddrFrame.offset = pContext->Ebp;
    sf.AddrFrame.Mode = AddrModeFlat;

    while ( 1 ) {
        if ( ! _StackWalk(  IMAGE_FILE_MACHINE_I386,
            GetCurrentProcess(),
            GetCurrentThread(),
            &sf,
            pContext,
            0,
            _SymFunctionTableAccess,
            _SymGetModuleBase,
            0 ) ) {
            break;
        }

        // Basic sanity check to make sure the frame is OK.  Bail if not.
        if ( 0 == sf.AddrFrame.offset ) {
            break;
        }

        _tprintf( "%08X  %08X  ", sf.AddrPC.offset, sf.AddrFrame.offset );

        // IMAGEHLP is wacky, and requires you to pass in a pointer to an
        // IMAGEHLP_SYMBOL structure.  The problem is that this structure is
        // variable length.  That is, you determine how big the structure is
        // at runtime.  This means that you can't use sizeof(struct).
        // So...make a buffer that's big enough, and make a pointer
        // to the buffer.  We also need to initialize not one, but TWO
        // members of the structure before it can be used.

        uint8_t symbolBuffer[ sizeof(IMAGEHLP_SYMBOL) + 512 ];
        PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
        pSymbol->SizeOfStruct = sizeof(symbolBuffer);
        pSymbol->MaxNameLength = 512;

        uint32_t symDisplacement = 0; // Displacement of the input address,
        // relative to the start of the symbol

        if ( _SymGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.offset,
            &symDisplacement, pSymbol) ) {
            _tprintf( "%hs+%X\n", pSymbol->Name, symDisplacement );

        } else {
            // No symbol found.  print out the logical address instead.
            char szModule[MAX_PATH] = "";
            uint32_t section = 0, offset = 0;

            if (!getLogicalAddress(  (PVOID)sf.AddrPC.offset,
                szModule, sizeof(szModule), section, offset )) {
                break;
            }

            _tprintf( "%04X:%08X %s\n",
                section, offset, szModule );
        }
    }

}

//============================================================================
// Helper function that writes to the report file, and allows the user to use
// printf style formating
//============================================================================
int __cdecl MSJExceptionHandler::_tprintf(const char * format, ...) {
    char szBuff[1024];
    int retValue;
    va_list argptr;

    va_start( argptr, format );
    retValue = wvsprintf( szBuff, format, argptr );
    va_end( argptr );

    _strReport += szBuff;
    // WriteFile( m_hReportFile, szBuff, retValue * sizeof(char), &cbWritten, 0 );

    return retValue;
}


//=========================================================================
// load IMAGEHLP.DLL and get the address of functions in it that we'll use
//=========================================================================
bool MSJExceptionHandler::initImagehlpFunctions( void ) {
    HMODULE hModImagehlp = LoadLibrary( "IMAGEHLP.DLL" );
    if ( !hModImagehlp ) {
        return false;
    }

    _SymInitialize = (SYMINITIALIZEPROC)GetProcAddress( hModImagehlp,
        "SymInitialize" );
    if ( !_SymInitialize ) {
        return false;
    }

    _SymCleanup = (SYMCLEANUPPROC)GetProcAddress( hModImagehlp, "SymCleanup" );
    if ( !_SymCleanup ) {
        return false;
    }

    _StackWalk = (STACKWALKPROC)GetProcAddress( hModImagehlp, "StackWalk" );
    if ( !_StackWalk ) {
        return false;
    }

    _SymFunctionTableAccess = (SYMFUNCTIONTABLEACCESSPROC)
    GetProcAddress( hModImagehlp, "SymFunctionTableAccess" );

    if ( !_SymFunctionTableAccess ) {
        return false;
    }

    _SymGetModuleBase=(SYMGETMODULEBASEPROC)GetProcAddress( hModImagehlp,
        "SymGetModuleBase");
    if ( !_SymGetModuleBase ) {
        return false;
    }

    _SymGetSymFromAddr=(SYMGETSYMFROMADDRPROC)GetProcAddress( hModImagehlp,
        "SymGetSymFromAddr" );
    if ( !_SymGetSymFromAddr ) {
        return false;
    }

    if ( !_SymInitialize( GetCurrentProcess(), 0, true ) ) {
        return false;
    }

    return true;
}
