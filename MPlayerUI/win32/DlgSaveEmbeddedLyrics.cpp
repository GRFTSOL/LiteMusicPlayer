#include "MPlayerApp.h"
#include "DlgSaveEmbeddedLyrics.h"
#include "AutoProcessEmbeddedLyrics.h"


CDlgSaveEmbeddedLyrics::CDlgSaveEmbeddedLyrics() : CBaseDialog(IDD_SAVE_EMBEDDED_LYR)
{
    m_nDefaultEmbededLST = g_profile.getInt("DefaultEmbeddedLST", LST_ID3V2_USLT);
    m_bHasEmbeddedLyrAlready = false;
}

CDlgSaveEmbeddedLyrics::~CDlgSaveEmbeddedLyrics()
{
}

int CDlgSaveEmbeddedLyrics::doModal(Window *pWnd)
{
    if (!g_LyricData.hasLyricsOpened())
    {
        pWnd->messageOut(_TLT("No Lyrics file was opened."));
        return IDCANCEL;
    }

    // get current media and lyrics information
    m_strMediaUrl = g_Player.getSrcMedia();
    
    if (m_strMediaUrl.empty() || !isFileExist(m_strMediaUrl.c_str()))
    {
        pWnd->messageOut(_TL("Can't locate the song file path."));
        return IDCANCEL;
    }

    string        str;
    g_LyricData.toString(str, FT_LYRICS_LRC, true);
    m_bufLyrics = insertWithFileBom(str);

    if (MediaTags::isM4aTagSupported(m_strMediaUrl.c_str()))
    {
        // save lyrics directly
        VecStrings vLyrNames;
        vLyrNames.push_back(SZ_SONG_M4A_LYRICS);
        if (doSaveAction(vLyrNames, pWnd) == ERR_OK)
            pWnd->messageOut(_TLT("Embedded Lyrics were saved successfully."));
        return IDOK;
    }
    else if (MediaTags::isID3v2TagSupported(m_strMediaUrl.c_str()))
        return CBaseDialog::doModal(pWnd);
    else
    {
        pWnd->messageOut(_TLT("This file doesn't support embedded lyrics."));
        return IDCANCEL;
    }
}

bool CDlgSaveEmbeddedLyrics::onInitDialog()
{
    if (!CBaseDialog::onInitDialog())
        return false;

    m_ctrlListLyrics.attach(this, IDC_L_LYRICS);
    m_ctrlListLyrics.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
    int n = m_ctrlListLyrics.InsertColumn(COL_DESC, _TLT("Embedded Lyrics Type"), LVCFMT_LEFT, 450);
    n = m_ctrlListLyrics.InsertColumn(COL_NAME, _TLT(""), LVCFMT_LEFT, 200);

    VecStrings vLyricsNames;
    MediaTags::getEmbeddedLyrics(m_strMediaUrl.c_str(), vLyricsNames);

    m_bHasEmbeddedLyrAlready = vLyricsNames.size() > 0;

    addEmbeddedItem(vLyricsNames, SZ_SONG_ID3V2_USLT);
    addEmbeddedItem(vLyricsNames, SZ_SONG_ID3V2_SYLT);
    addEmbeddedItem(vLyricsNames, SZ_SONG_LYRICS3V2);

    return true;
}

void CDlgSaveEmbeddedLyrics::addEmbeddedItem(VecStrings &vAlreadyHave, cstr_t szToAdd)
{
    LRC_SOURCE_TYPE lstToAdd = lyrSrcTypeFromName(szToAdd);
    bool bAdded = false;

    for (uint32_t i = 0; i < vAlreadyHave.size(); i++)
    {
        if (startsWith(vAlreadyHave[i].c_str(), szToAdd))
        {
            bAdded = true;
            int n = m_ctrlListLyrics.appendItem(_TL(lyrSrcTypeToDesc(lstToAdd)));
            m_ctrlListLyrics.setItemText(n, COL_NAME, vAlreadyHave[i].c_str());
            m_ctrlListLyrics.setCheck(n, true);
        }
    }

    if (!bAdded)
    {
        int n = m_ctrlListLyrics.appendItem(_TL(lyrSrcTypeToDesc(lstToAdd)));
        m_ctrlListLyrics.setItemText(n, COL_NAME, szToAdd);

        if (vAlreadyHave.empty())
        {
            if (isFlagSet(m_nDefaultEmbededLST, lstToAdd))
                m_ctrlListLyrics.setCheck(n, true);
        }
    }
}

void CDlgSaveEmbeddedLyrics::onOK()
{
    VecStrings vLyrNames;
    uint32_t nEmbeddedLST = 0;
    for (int i = 0; i < m_ctrlListLyrics.getItemCount(); i++)
    {
        if (m_ctrlListLyrics.GetCheck(i))
        {
            string name;
            m_ctrlListLyrics.getItemText(i, COL_NAME, name);
            vLyrNames.push_back(name);
            nEmbeddedLST |= lyrSrcTypeFromName(name.c_str());
        }
    }

    if (!m_bHasEmbeddedLyrAlready)
        g_profile.writeInt("DefaultEmbeddedLST", nEmbeddedLST);

    doSaveAction(vLyrNames, this);

    CBaseDialog::onOK();
}

int CDlgSaveEmbeddedLyrics::doSaveAction(VecStrings &vLyrNames, Window *pWnd)
{
    int nRet = g_autoProcessEmbeddedLyrics.saveEmbeddedLyrics(m_strMediaUrl.c_str(), "", &m_bufLyrics, vLyrNames);
    if (nRet != ERR_OK)
    {
        string strMsg = _TLT("Failed to save embedded lyrics, MiniLyrics will auto try again later.");
        strMsg += SZ_RETURN;
        strMsg += SZ_RETURN;
        strMsg += ERROR2STR_LOCAL(nRet);
        pWnd->messageOut(strMsg.c_str());
    }

    return nRet;
}
