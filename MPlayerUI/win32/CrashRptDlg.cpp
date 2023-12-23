#include "../MPlayerApp.h"
#include "CrashRptDlg.h"
#include "../Utils/win32/MiniDump.h"
#include "../../Window/win32/BaseDialog.h"
#include "../../Utils/win32/MailFileTo.h"
//#include "../resource.h"


class CCrashRptDlg : public CBaseDialog {
public:
    CCrashRptDlg(): CBaseDialog(IDD_CRASH_REPORT) { }

    void onOK() {
        string strSubjet = getAppNameLong();
        strSubjet += " Minidumps";

        CBaseDialog::onOK();

        sendMail(HWND_DESKTOP, SZ_COMPANY_NAME " support", getStrName(SN_SUPPORT_MAIL), _fnDumpSaved.c_str(), strSubjet.c_str());
    }

public:
    void updateListBoxHorzExtent(cstr_t szStrInsert);

    string                      _fnDumpSaved;

};

void initMiniDumper() {
    auto fnToSave = getAppDataFile(SZ_APP_NAME "_crash.dmp");
    MiniDumper::init(fnToSave.c_str(), [](cstr_t dumpFileSaved) {
        CCrashRptDlg dlg;

        dlg._fnDumpSaved = dumpFileSaved;

        // show error report dialog
        dlg.doModal(MPlayerApp::getMainWnd());
    });
}
