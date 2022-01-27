#import "DlgSaveEmbeddedLyricsController.h"

#import "MPlayerApp.h"
#import "DlgSaveEmbeddedLyrics.h"
#import "AutoProcessEmbeddedLyrics.h"

//////////////////////////////////////////////////////////////////////

DlgSaveEmbeddedLyricsController * GetController(CDlgSaveEmbeddedLyrics *pThis)
{
    return (DlgSaveEmbeddedLyricsController *)pThis->getControllerHandle();
}

CDlgSaveEmbeddedLyrics::CDlgSaveEmbeddedLyrics()
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
    
    m_pParentWnd = pWnd;

    // Get current media and lyrics information
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
            pWnd->messageOut(_TL("Embedded Lyrics were saved successfully."));
        return IDOK;
    }
    else if (MediaTags::isID3v2TagSupported(m_strMediaUrl.c_str()))
    {
        DlgSaveEmbeddedLyricsController *controller = [[DlgSaveEmbeddedLyricsController alloc] initWithWindowNibName:@"DlgSaveEmbeddedLyricsController"];
        m_pDlgController = controller;
        onInitDialog();
        [controller doModal:this];
        
        return IDOK;
    }
    else
    {
        pWnd->messageOut(_TL("This file doesn't support embedded lyrics."));
        return IDCANCEL;
    }
    
    
//    NSWindow *pAbtWindow = [controller window];
//    
//    [NSApp runModalForWindow: pAbtWindow];
//    
//    [NSApp endSheet: pAbtWindow];
//    
//    [pAbtWindow orderOut: nil];
//    [controller release];
    
    return IDOK;
}

bool CDlgSaveEmbeddedLyrics::onInitDialog()
{
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
    bool bIsChecked = false;

    for (uint32_t i = 0; i < vAlreadyHave.size(); i++)
    {
        if (startsWith(vAlreadyHave[i].c_str(), szToAdd))
        {
            bIsChecked = true;
        }
    }

    if (!bIsChecked)
    {
        if (vAlreadyHave.empty())
        {
            if (isFlagSet(m_nDefaultEmbededLST, lstToAdd))
                bIsChecked = true;
        }
    }

    [GetController(this) addItem: _TL(lyrSrcTypeToDesc(lstToAdd)) name:szToAdd isChecked:bIsChecked];
}

void CDlgSaveEmbeddedLyrics::onOK()
{
    VecStrings vLyrNames;
    uint32_t nEmbeddedLST = 0;
    DlgSaveEmbeddedLyricsController *controller = GetController(this);
    
    for (int i = 0; i < [controller getItemCount]; i++)
    {
        if ([controller getCheckAtIndex:i])
        {
            string name = [controller getNameAtIndex:i];
            vLyrNames.push_back(name);
            nEmbeddedLST |= lyrSrcTypeFromName(name.c_str());
        }
    }

    if (!m_bHasEmbeddedLyrAlready)
        g_profile.writeInt("DefaultEmbeddedLST", nEmbeddedLST);

    doSaveAction(vLyrNames, m_pParentWnd);
}

int CDlgSaveEmbeddedLyrics::doSaveAction(VecStrings &vLyrNames, Window *pWnd)
{
    int nRet = g_autoProcessEmbeddedLyrics.saveEmbeddedLyrics(m_strMediaUrl.c_str(), "", &m_bufLyrics, vLyrNames);
    if (nRet != ERR_OK)
    {
        string strMsg = _TLT("Failed to save embedded lyrics, MiniLyrics will auto try again later.");
        strMsg += "\n\n";
        strMsg += ERROR2STR_LOCAL(nRet);
        pWnd->messageOut(strMsg.c_str());
    }

    return nRet;
}
