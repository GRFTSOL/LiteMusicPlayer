// CrashHandler.cpp: implementation of the CCrashHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "Utils.h"
#include "CrashHandler.h"

CCrashHandler        g_CrashHandler;

#define MAIL_RT    "%0d%0a"

//
// 异常转换
//
IdToString    arrExceptIds[] = 
{
    DEFINE_ID(EXCEPTION_ACCESS_VIOLATION        ),
    DEFINE_ID(EXCEPTION_DATATYPE_MISALIGNMENT   ),
    DEFINE_ID(EXCEPTION_BREAKPOINT              ),
    DEFINE_ID(EXCEPTION_SINGLE_STEP             ),
    DEFINE_ID(EXCEPTION_ARRAY_BOUNDS_EXCEEDED   ),
    DEFINE_ID(EXCEPTION_FLT_DENORMAL_OPERAND    ),
    DEFINE_ID(EXCEPTION_FLT_DIVIDE_BY_ZERO      ),
    DEFINE_ID(EXCEPTION_FLT_INEXACT_RESULT      ),
    DEFINE_ID(EXCEPTION_FLT_INVALID_OPERATION   ),
    DEFINE_ID(EXCEPTION_FLT_OVERFLOW            ),
    DEFINE_ID(EXCEPTION_FLT_STACK_CHECK         ),
    DEFINE_ID(EXCEPTION_FLT_UNDERFLOW           ),
    DEFINE_ID(EXCEPTION_INT_DIVIDE_BY_ZERO      ),
    DEFINE_ID(EXCEPTION_INT_OVERFLOW            ),
    DEFINE_ID(EXCEPTION_PRIV_INSTRUCTION        ),
    DEFINE_ID(EXCEPTION_IN_PAGE_ERROR           ),
    DEFINE_ID(EXCEPTION_ILLEGAL_INSTRUCTION     ),
    DEFINE_ID(EXCEPTION_NONCONTINUABLE_EXCEPTION),
    DEFINE_ID(EXCEPTION_STACK_OVERFLOW          ),
    DEFINE_ID(EXCEPTION_INVALID_DISPOSITION     ),
    DEFINE_ID(EXCEPTION_GUARD_PAGE              ),
    DEFINE_ID(EXCEPTION_INVALID_HANDLE          ),
    {0, nullptr}
};
cstr_t exptionIdToStr(uint32_t dwExceptId)
{
    for (IdToString *pStart = arrExceptIds; pStart->szId != nullptr; pStart++)
    {
        if (pStart->dwId == dwExceptId)
        {
            return pStart->szId;
        }
    }
    return "EXCEPTION_UNKNOWN";
}

void enumModules(vector<string> &vModules)
{
    PBYTE    pb = nullptr;
    MEMORY_BASIC_INFORMATION    mbi;
    PBYTE    pAddrLast = (PBYTE)1;
    
    while (VirtualQuery(pb, &mbi, sizeof(mbi)) == sizeof(mbi))
    {
        int        nLen;
        char    szModuleName[MAX_PATH];
        
        if (mbi.State == MEM_FREE)
            mbi.AllocationBase = mbi.BaseAddress;
        
            /*        if (mbi.AllocationBase == hInst)
            {
            nLen = 0;
            }
            else
        {*/
        if (pAddrLast != (PBYTE)mbi.AllocationBase)
        {
            nLen = GetModuleFileName((HINSTANCE)mbi.AllocationBase, szModuleName, MAX_PATH);
            pAddrLast = (PBYTE)mbi.AllocationBase;
        }
        else
            nLen = 0;
        //        }
        if (nLen > 0)
        {
            vModules.push_back(szModuleName);
            // messageBox(nullptr, szModuleName, szModuleName, MB_OK);
        }
        
        pb += mbi.RegionSize;
    }
}


// unhandled exception callback set with SetUnhandledExceptionFilter()
LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
    // _crashStateMap.Lookup(_getpid())->GenerateErrorReport(pExInfo);
    if (g_CrashHandler.report(pExInfo))
        return EXCEPTION_EXECUTE_HANDLER;
    else
        return g_CrashHandler.m_oldFilter(pExInfo);
    //return EXCEPTION_CONTINUE_EXECUTION;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCrashHandler::CCrashHandler()
{
    m_oldFilter    = nullptr;
}

CCrashHandler::~CCrashHandler()
{
    
}

void CCrashHandler::init(FunCrashCallback funCallback, void *lpUserData)
{
    // add this filter in the exception callback chain
    m_oldFilter = SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

    m_funCallback = funCallback;
    m_lpUserData = lpUserData;
}
/*
void CCrashHandler::AddWatchModule(HANDLE hModule)
{
    assert(find(m_vWatchModule.begin(), m_vWatchModule.end(), hModule) == m_vWatchModule.end());
    m_vWatchModule.push_back(hModule);
}*/

// #define messageOut(msg)           messageBox(nullptr, msg, "message", MB_OK)
bool CCrashHandler::report(PEXCEPTION_POINTERS pExceptionRecord)
{
    LOG0(LOG_LVL_ERROR, "Exception ocurred!");
    
//    string            strRpt;
    string            strRptShort;
    
    //
    // OS INFO
    //
    OSVERSIONINFO oi;
    
    char        szVersion[256];
    
    oi.dwOSVersionInfoSize = sizeof(oi);
    if (GetVersionEx(&oi))
    {
        // wsprintf(szVersion, "OperatingSystem, MajorVersion: %d, MinorVersion : %d, BuildNumber: %d, CSDVersion: %s",
        wsprintf(szVersion, "OS: %d.%d.%d, SP: %s",
            oi.dwMajorVersion, oi.dwMinorVersion, oi.dwBuildNumber, oi.szCSDVersion);
    }
    else
        strcpy(szVersion, "get Os version Info FAILED!");
    
    LOG0(LOG_LVL_ERROR, szVersion);
    strRptShort += szVersion;
    strRptShort += MAIL_RT;
    
    //
    // module Info
    //
    vector<string>    vModules;
    HMODULE            hExcpModule = nullptr;
    string            strModules = "Exception Address is between moudule: ";
    enumModules(vModules);
    for (uint32_t i = 0; i < vModules.size(); i++)
    {
        char    szTemp[MAX_PATH];
        char    szName[MAX_PATH];
        HMODULE    hModule;
        hModule = GetModuleHandle(vModules[i].c_str());
        if (pExceptionRecord->ExceptionRecord->ExceptionAddress <= hModule)
        {
            if (i > 0)
            {
                fileGetName(szName, MAX_PATH, vModules[i - 1].c_str());
                hExcpModule = hModule = GetModuleHandle(vModules[i - 1].c_str());
                wsprintf(szTemp, "%s, 0x%X; ", szName, hModule);
                strModules += szTemp;
                
                fileGetName(szName, MAX_PATH, vModules[i].c_str());
                hModule = GetModuleHandle(vModules[i].c_str());
                wsprintf(szTemp, "%s, 0x%X; ", szName, hModule);
                strModules += szTemp;
            }
            break;
        }
    }
    assert(hExcpModule);
    LOG0(LOG_LVL_ERROR, strModules.c_str());
    strRptShort += strModules;
    strRptShort += MAIL_RT;
    
    //
    // processor info
    //
    /*    SYSTEM_INFO si;
    char            szProcessor[256];
    char            szArchitecture[64];
    char            szLevel[64];
    
      GetSystemInfo(&si);
      switch (si.wProcessorArchitecture)
      {
      case PROCESSOR_ARCHITECTURE_INTEL:
      strcpy(szArchitecture, "PROCESSOR_ARCHITECTURE_INTEL");
      break;
      case PROCESSOR_ARCHITECTURE_MIPS:
      strcpy(szArchitecture, "PROCESSOR_ARCHITECTURE_MIPS");
      break;
      case PROCESSOR_ARCHITECTURE_ALPHA:
      strcpy(szArchitecture, "PROCESSOR_ARCHITECTURE_ALPHA");
      break;
      case PROCESSOR_ARCHITECTURE_PPC:
      strcpy(szArchitecture, "PROCESSOR_ARCHITECTURE_PPC");
      break;
      case PROCESSOR_ARCHITECTURE_SHX:
      strcpy(szArchitecture, "PROCESSOR_ARCHITECTURE_SHX");
      break;
      case PROCESSOR_ARCHITECTURE_ARM:
      strcpy(szArchitecture, "PROCESSOR_ARCHITECTURE_ARM");
      break;
      case PROCESSOR_ARCHITECTURE_IA64:
      strcpy(szArchitecture, "PROCESSOR_ARCHITECTURE_IA64");
      break;
      case PROCESSOR_ARCHITECTURE_ALPHA64:
      strcpy(szArchitecture, "PROCESSOR_ARCHITECTURE_ALPHA64");
      break;
      case PROCESSOR_ARCHITECTURE_UNKNOWN:
      strcpy(szArchitecture, "PROCESSOR_ARCHITECTURE_UNKNOWN");
      break;
      default:
      strcpy(szArchitecture, "UnKnown");
      }
      
        //
        // set level
        //
        if (PROCESSOR_ARCHITECTURE_INTEL == si.wProcessorArchitecture)
        {
        switch (si.wProcessorLevel)
        {
        case 3:
        strcpy(szLevel, "Intel 30386");
        break;
        case 4:
        strcpy(szLevel, "Intel 30486");
        break;
        case 5:
        strcpy(szLevel, "Intel Pentium");
        break;
        case 6:
        strcpy(szLevel, "Intel Pentium Pro or Pentium II");
        break;
        default:
        strcpy(szLevel, "Unknown");
        }
        }
        
          wsprintf(szProcessor, "Processor Architecture: %s, Level: %s, NumberOfProcessor: %d", szArchitecture, szLevel, si.dwNumberOfProcessors);
          LOG0(LOG_LVL_ERROR, szProcessor);
          strRpt += szProcessor;
    strRpt += "\r\n";*/
    
    //
    // exception info
    //
    //
    
    char        szException[1024];
    char        szModule[MAX_PATH];
    
    GetModuleFileName(nullptr, szModule, MAX_PATH);
    
    //
    // set exception description
    //
    
    wsprintf(szException, "ExceptionRecord: ModuleName=\"%s\" ExceptionCode=\"%#x\" ExceptionDescription=\"%s\" ExceptionAddress=\"%#x\"",
        szModule, pExceptionRecord->ExceptionRecord->ExceptionCode, 
        exptionIdToStr(pExceptionRecord->ExceptionRecord->ExceptionCode), 
        pExceptionRecord->ExceptionRecord->ExceptionAddress);
    
    LOG0(LOG_LVL_ERROR, szException);
//    strRpt += szException;
//    strRpt += "\r\n";
    strRptShort += szException;
    strRptShort += MAIL_RT;

    return m_funCallback(this, strRptShort.c_str(), hExcpModule, m_lpUserData);
}
