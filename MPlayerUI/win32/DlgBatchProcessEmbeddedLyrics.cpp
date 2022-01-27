// DlgBatchProcessEmbeddedLyrics.cpp: implementation of the CDlgBatchProcessEmbeddedLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "DlgBatchProcessEmbeddedLyrics.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgBatchProcessEmbeddedLyrics::CDlgBatchProcessEmbeddedLyrics() : CBaseDialog(IDD_BATCH_EMBEDDED_LYR)
{

}

CDlgBatchProcessEmbeddedLyrics::~CDlgBatchProcessEmbeddedLyrics()
{

}

bool CDlgBatchProcessEmbeddedLyrics::onInitDialog()
{
    if (!CBaseDialog::onInitDialog())
        return false;

    cstr_t        szColName[] = { _TLM("Action"), _TLM("Song File"), _TLM("Result"), _TLM("Embedded Lyrics Type"), _TLM("Lyrics File")};
    int            nColWidth[] = { 50, 150, 200, 200, 200};

    m_listCtrl.attach(this, IDC_LIST);
    m_listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);

    assert(CountOf(szColName) == CountOf(nColWidth));
    for (int i = 0; i < CountOf(szColName); i++)
        m_listCtrl.InsertColumn(i, _TL(szColName[i]), LVCFMT_LEFT, nColWidth[i]);

    MutexAutolock    autolock(g_autoProcessEmbeddedLyrics.m_mutex);

    CAutoProcessEmbeddedLyrics::LIST_ITEMS::iterator    it, itEnd;
    itEnd = g_autoProcessEmbeddedLyrics.m_listJobs.end();
    for (it = g_autoProcessEmbeddedLyrics.m_listJobs.begin(); it != itEnd; ++it)
    {
        CAutoProcessEmbeddedLyrics::Item    &item = *it;
        int n = m_listCtrl.insertItem(m_listCtrl.getItemCount(), item.m_bRemove ? "remove" : "add");
        m_listCtrl.setItemText(n, COL_SONG, fileGetName(item.m_strSongFile.c_str()));
        string name = strJoin(item.m_vLyrNames.begin(), item.m_vLyrNames.end(), ", ");
        m_listCtrl.setItemText(n, COL_EMBEDDED_TYPE, name.c_str());
    }

    return true;
}

void CDlgBatchProcessEmbeddedLyrics::onOK()
{
    MutexAutolock    autolock(g_autoProcessEmbeddedLyrics.m_mutex);
    int        nRet, n = 0, nFailedTimes = 0;
    CAutoProcessEmbeddedLyrics::LIST_ITEMS::iterator    it, itEnd;

    itEnd = g_autoProcessEmbeddedLyrics.m_listJobs.end();
    for (it = g_autoProcessEmbeddedLyrics.m_listJobs.begin(); it != itEnd; ++it)
    {
        CAutoProcessEmbeddedLyrics::Item    &item = *it;
        if (!item.m_bDealOK)
        {
            nRet = g_autoProcessEmbeddedLyrics.dealEmbeddedLyrics(item);
            if (nRet != ERR_OK)
            {
                m_listCtrl.setItemText(n, COL_RESULT, ERROR2STR_LOCAL(nRet));
                nFailedTimes++;
            }
            else
                item.m_bDealOK = true;
        }
        n++;
    }

    if (nFailedTimes > 0)
        messageOut(CStrPrintf("%d Lyrics were failed to process.", nFailedTimes).c_str());
    else
        CBaseDialog::onOK();
}
