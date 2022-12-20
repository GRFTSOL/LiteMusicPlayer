#include "BaseWnd.h"
#include "FileSaveDlg.h"
#include <Dlgs.h>


// COMMENT:
//        取得 GetSaveFileName 中lpstrFilter 的第nIndex个扩展名。
// INPUT:
//        szFilters    :    "All supported files (*.lrc; *.txt; *.snc)\0*.lrc;*.txt;*.snc\0LRC Lyrics File (*.lrc)\0*.LRC\0Text File (*.txt)\0*.TXT\0Snc File (*.snc)\0*.snc\0\0"
cstr_t getFileFilterExtByIndex(cstr_t szFilters, int nIndex)
{
    assert(nIndex >= 1);
    cstr_t        szReturn;

    szReturn = szFilters;

    //
    // 找到第nIndex个filter
    for (int i = 0; i < nIndex * 2 - 1; i++)
    {
        while (*szReturn != '\0')
            szReturn++;
        szReturn++;
        if (*szReturn == '\0')
            return nullptr;
    }

    // 移动直到扩展名
    while (*szReturn != '.' && *szReturn != '\0')
        szReturn++;

    return szReturn;
}

UINT_PTR CALLBACK MyOFNHookProc(
                                HWND hdlg,      // handle to dialog box
                                uint32_t uiMsg,      // message identifier
                                WPARAM wParam,  // message parameter
                                LPARAM lParam   // message parameter
                                )
{
    switch (uiMsg)
    {
    case WM_NOTIFY:
        LPOFNOTIFY        lpOfNotify;

        lpOfNotify = (LPOFNOTIFY)lParam;
        if (lpOfNotify)
        {
            //DBG_LOG2("hdr.code: %d, hdr.idFrom: %d", lpOfNotify->hdr.code, lpOfNotify->hdr.idFrom);

            if (lpOfNotify->hdr.code == CDN_TYPECHANGE)
            {
                //if (lpOfNotify->pszFile)
                //_tcscpy(lpOfNotify->pszFile, "aa.snc");
                //DBG_LOG3("HWND: %x, hWndFrom: %x, FilterIndex: %d", hdlg, lpOfNotify->hdr.hwndFrom, lpOfNotify->lpOFN->nFilterIndex);
                char    szFileName[MAX_PATH];
                cstr_t    szNewExt;

                szNewExt = getFileFilterExtByIndex(lpOfNotify->lpOFN->lpstrFilter, lpOfNotify->lpOFN->nFilterIndex);
                if (lpOfNotify->lpOFN->nFilterIndex > 1 && szNewExt)
                {

                    getDlgItemText(lpOfNotify->hdr.hwndFrom, cmb13, szFileName, MAX_PATH);
                    fileSetExt(szFileName, CountOf(szFileName), szNewExt);
                    setDlgItemText(lpOfNotify->hdr.hwndFrom, cmb13, szFileName);
                }
            }
            return 1;
        }
        break;
    }
    return 0;
}

CFileSaveDlg::CFileSaveDlg(cstr_t szTitle, cstr_t szFile, cstr_t extFilter, int nDefFileType)
{
    strcpy_safe(m_szFile, CountOf(m_szFile), szFile);

    memset(&m_openfile, 0, sizeof(m_openfile));
    m_openfile.lpstrTitle = szTitle;
    m_openfile.lpstrFilter = extFilter;
    m_openfile.lpstrFile = m_szFile;
    m_openfile.nMaxFile = CountOf(m_szFile);
    m_openfile.nFilterIndex = nDefFileType;

    m_openfile.lStructSize = sizeof(m_openfile);
    m_openfile.hInstance = getAppInstance();
    m_openfile.Flags = OFN_OVERWRITEPROMPT | OFN_ENABLEHOOK | OFN_EXPLORER;
    m_openfile.lpfnHook = (LPOFNHOOKPROC)MyOFNHookProc;
}

CFileSaveDlg::~CFileSaveDlg(void)
{
}

int CFileSaveDlg::doModal(Window *pWndParent)
{
    m_openfile.hwndOwner = pWndParent->getHandle();
    if (GetSaveFileName(&m_openfile))
        return IDOK;
    else
        return IDCANCEL;
}

cstr_t CFileSaveDlg::getSaveFile()
{
    return m_szFile;
}

cstr_t CFileSaveDlg::getSelectedExt()
{
    cstr_t        szNewExt;
    szNewExt = getFileFilterExtByIndex(m_openfile.lpstrFilter, m_openfile.nFilterIndex);
    if (szNewExt)
        return szNewExt;
    else
        return "";
}
