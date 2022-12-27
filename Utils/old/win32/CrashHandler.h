#pragma once

#ifndef Utils_old_win32_CrashHandler_h
#define Utils_old_win32_CrashHandler_h




class CCrashHandler;

typedef bool (*FunCrashCallback)(CCrashHandler *pCrashHandle, cstr_t szMsg, HMODULE hCrashMod, void *lpUserData);

class CCrashHandler {
public:
    bool report(PEXCEPTION_POINTERS pExceptionRecord);

    void init(FunCrashCallback funCallback, void *lpUserData = nullptr);

    // void AddWatchModule(HANDLE hModule);

    CCrashHandler();
    virtual ~CCrashHandler();

protected:
    LPTOP_LEVEL_EXCEPTION_FILTER m_oldFilter;        // previous exception filter
    // LPGETLOGFILE                  m_lpfnCallback;   // client crash callback
    int                         m_pid;              // process id
    FunCrashCallback            m_funCallback;
    void                        *                           m_lpUserData;
    // vector<HANDLE>                    m_vWatchModule;

    friend LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);

};

extern CCrashHandler g_CrashHandler;

#endif // !defined(Utils_old_win32_CrashHandler_h)
