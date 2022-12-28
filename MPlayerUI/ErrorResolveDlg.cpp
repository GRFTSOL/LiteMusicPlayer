#include "MPlayerApp.h"
#include "ErrorResolveDlg.h"
#include "Helper.h"
#include "PreferenceDlg.h"


void showInetErrorDlg(Window *pWnd, int nError) {
    string strError;
    string strResolve;
    char szErrorId[256];

    snprintf(szErrorId, CountOf(szErrorId), "%s %d,  ", _TLT("Error Code:"), nError);
    strError = szErrorId;
    strError += ERROR2STR_LOCAL(nError);
    strError += "\r\n\r\n";
    strError += _TLT("Do you want to visit our website for more information?");
    if (pWnd->messageOut(strError.c_str(), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK) {
        openUrl(pWnd, getStrName(SN_HTTP_FAQ_INET));
    }
}
