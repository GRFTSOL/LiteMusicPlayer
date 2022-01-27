// xstr.h: interface for the Cxstr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ML_MISC_INC_)
#define _ML_MISC_INC_

bool getMonitorRestrictRect(const CRect &rcIn, CRect &rcRestrict);


#ifndef _WIN32

// return system elapsed time in ms.
uint32_t getTickCount();

void sleep(uint32_t dwMilliseconds);

#endif

#ifdef WIN32

bool executeCmdAndWait(cstr_t szCmd, uint32_t dwTimeOut = INFINITE, uint32_t *pExitCode = nullptr);

bool getModulePath(char szPath[], HINSTANCE hInstance = nullptr);

void centerWindowToMonitor(HWND hWnd);


#else

#endif // WIN32

#endif // !defined(_ML_MISC_INC_)
