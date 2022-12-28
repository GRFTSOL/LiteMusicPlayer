#include "MPlayerApp.h"
#include "crashRptDlg.h"

#ifdef _WIN32_DESKTOP

#include "../Utils/win32/MailFileTo.h"


CMiniDumperNotify dumpNotify;

bool CMiniDumperNotify::onBeginDump(HMODULE hCrashMod, char szDumpFileToSave[], int nLen) {
    getAppDataDir(szDumpFileToSave);
    strcat_safe(szDumpFileToSave, nLen, SZ_APP_NAME "_crash.dmp");

    return true;
}

bool CMiniDumperNotify::onDumpFinished(HMODULE hCrashMod, cstr_t szDumpFileToSave) {
    CCrashRptDlg dlg;

    dlg.m_strDumpFile = szDumpFileToSave;

    // show error report dialog
    dlg.doModal(CMPlayerAppBase::getMainWnd());

    return true;
}


bool CCrashRptDlg::onInitDialog() {
    CBaseDialog::onInitDialog();

    return true;
}

void CCrashRptDlg::onOK() {
    CMailFileTo mail;
    string strSubjet;

    strSubjet = getAppNameLong();
    strSubjet += " Minidumps";

    CBaseDialog::onOK();

    mail.sendMail(HWND_DESKTOP, SZ_COMPANY_NAME " support", getStrName(SN_SUPPORT_MAIL), m_strDumpFile.c_str(), strSubjet.c_str());
}

#endif // #ifdef _WIN32_DESKTOP
