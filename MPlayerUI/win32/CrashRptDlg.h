#ifndef MPlayerUI_win32_CrashRptDlg_h
#define MPlayerUI_win32_CrashRptDlg_h

#pragma once

/*
#include "../Utils/win32/MiniDump.h"


class CMiniDumperNotify : public IMiniDumperNotify {
public:
    virtual bool onBeginDump(HMODULE hCrashMod, char szDumpFileToSave[], int nLen);
    virtual bool onDumpFinished(HMODULE hCrashMod, cstr_t szDumpFileToSave);

};

extern CMiniDumperNotify dumpNotify;

inline void initMiniDumper() { CMiniDumper::init(&dumpNotify); }

//////////////////////////////////////////////////////////////////////
// CCrashRptDlg
class CCrashRptDlg : public CBaseDialog {
public:
    bool onInitDialog();
    void onOK();
    CCrashRptDlg(): CBaseDialog(IDD_CRASH_REPORT) { }

public:
    void updateListBoxHorzExtent(cstr_t szStrInsert);

    string                      m_strDumpFile;

};

inline void initMiniDumper() { }

*/

#endif // !defined(MPlayerUI_win32_CrashRptDlg_h)
