// CrashHandler.h: interface for the CCrashHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRASHHANDLER_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_)
#define AFX_CRASHHANDLER_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class CCrashHandler;

typedef bool (*FunCrashCallback)(CCrashHandler *pCrashHandle, cstr_t szMsg, HMODULE hCrashMod, void *lpUserData);

class CCrashHandler  
{
public:
    bool report(PEXCEPTION_POINTERS pExceptionRecord);

    void init(FunCrashCallback funCallback, void *lpUserData = nullptr);

    // void AddWatchModule(HANDLE hModule);

    CCrashHandler();
    virtual ~CCrashHandler();

protected:
    LPTOP_LEVEL_EXCEPTION_FILTER  m_oldFilter;      // previous exception filter
    // LPGETLOGFILE                  m_lpfnCallback;   // client crash callback
    int                           m_pid;            // process id
    FunCrashCallback                m_funCallback;
    void *                           m_lpUserData;
    // vector<HANDLE>                    m_vWatchModule;

friend LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);

};

extern CCrashHandler        g_CrashHandler;

#endif // !defined(AFX_CRASHHANDLER_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_)
