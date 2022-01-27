#pragma once

namespace LyricsDownloader
{

#define SZ_SECT_LYR_DOWNLOADER        "LyrDownloader"

class CEmbeddedOptDialog : public CBaseDialog
{
public:
    CEmbeddedOptDialog(uint32_t nIDTemplate) : CBaseDialog(nIDTemplate) { }

    void getEmbeddedOptCheck(CBaseDialog *pDlg, VecStrings &vEmbeddedLyrNames)
    {
        uint32_t    lst = 0;
        if (pDlg->isButtonChecked(IDC_C_EDL_M4A))
        {
            lst |= LST_M4A_LYRICS;
            vEmbeddedLyrNames.push_back(SZ_SONG_M4A_LYRICS);
        }

        if (pDlg->isButtonChecked(IDC_C_EDL_ID3V2_SYLY))
        {
            lst |= LST_ID3V2_SYLT;
            vEmbeddedLyrNames.push_back(SZ_SONG_ID3V2_SYLT);
        }

        if (pDlg->isButtonChecked(IDC_C_EDL_ID3V2_USLY))
        {
            lst |= LST_ID3V2_USLT;
            vEmbeddedLyrNames.push_back(SZ_SONG_ID3V2_USLT);
        }

        if (pDlg->isButtonChecked(IDC_C_EDL_LYR3V2))
        {
            lst |= LST_LYRICS3V2;
            vEmbeddedLyrNames.push_back(SZ_SONG_LYRICS3V2);
        }

        g_profile.writeInt(SZ_SECT_LYR_DOWNLOADER, "EmbeddedLyr", lst);
    }

    void loadEmbeddedOptCheck(CBaseDialog *pDlg)
    {
        uint32_t lst = g_profile.getInt(SZ_SECT_LYR_DOWNLOADER, "EmbeddedLyr", 0);

        if (isFlagSet(lst, LST_M4A_LYRICS))
            pDlg->checkButton(IDC_C_EDL_M4A, true);

        if (isFlagSet(lst, LST_ID3V2_SYLT))
            pDlg->checkButton(IDC_C_EDL_ID3V2_SYLY, true);

        if (isFlagSet(lst, LST_ID3V2_USLT))
            pDlg->checkButton(IDC_C_EDL_ID3V2_USLY, true);

        if (isFlagSet(lst, LST_LYRICS3V2))
            pDlg->checkButton(IDC_C_EDL_LYR3V2, true);
    }

    bool loadBtnCheckStatus(int btnId, cstr_t szValueName, bool bDefault)
    {
        bool bVal = g_profile.getBool(SZ_SECT_LYR_DOWNLOADER, szValueName, bDefault);
        checkButton(btnId, bVal);
        return bVal;
    }

    bool saveBtnCheckStatus(int btnId, cstr_t szValueName)
    {
        bool bVal = isButtonChecked(btnId);
        g_profile.writeInt(SZ_SECT_LYR_DOWNLOADER, szValueName, bVal);
        return bVal;
    }

    void enableEmbeddedLyr(bool bEnable)
    {
        enableDlgItem(IDC_C_EDL_M4A, bEnable);
        enableDlgItem(IDC_C_EDL_ID3V2_SYLY, bEnable);
        enableDlgItem(IDC_C_EDL_ID3V2_USLY, bEnable);
        enableDlgItem(IDC_C_EDL_LYR3V2, bEnable);
    }

    void onCommand(uint32_t uID, uint32_t nNotifyCode)
    {
        if (uID == IDC_C_SAVE_IN_SONG_FILE || uID == IDC_C_REMOVE_EMBEDDED_LYR)
        {
            enableEmbeddedLyr(isButtonChecked(uID));
        }
        else
            CBaseDialog::onCommand(uID, nNotifyCode);
    }

};

class CDlgAdvDownloadPrompt : public CEmbeddedOptDialog
{
public:
    CDlgAdvDownloadPrompt() : CEmbeddedOptDialog(IDD_LD_ADV_DOWNLOAD_LYR) {
    }
    
    bool onInitDialog()
    {
        CBaseDialog::onInitDialog();

        loadBtnCheckStatus(IDC_C_USE_LOCAL_ONLY, "OnlyUseLocalLyr", false);

        loadBtnCheckStatus(IDC_C_SAVE_IN_SONG_DIR, "SaveInSongDir", false);
        loadBtnCheckStatus(IDC_C_SAVE_IN_DIR, "SaveInDir", false);
        if (!loadBtnCheckStatus(IDC_C_AS_SONG_NAME, "SaveAsSongName", false))
            checkButton(IDC_C_KEEP_NAME, true);

        if (!loadBtnCheckStatus(IDC_C_SAVE_IN_SONG_FILE, "SaveInSongFile", false))
            enableEmbeddedLyr(false);

        loadEmbeddedOptCheck(this);

        setDlgItemText(IDC_BR_DIR, g_LyricsDownloader.getDefSavePath());

        return true;
    }

    virtual void onOK()
    {
        m_bOnlyUseLocalLyrics = saveBtnCheckStatus(IDC_C_USE_LOCAL_ONLY, "OnlyUseLocalLyr");

        saveBtnCheckStatus(IDC_C_SAVE_IN_SONG_DIR, "SaveInSongDir");
        saveBtnCheckStatus(IDC_C_SAVE_IN_DIR, "SaveInDir");
        saveBtnCheckStatus(IDC_C_AS_SONG_NAME, "SaveAsSongName");

        if (isButtonChecked(IDC_C_SAVE_IN_SONG_DIR))
            m_downSaveDir = DOWN_SAVE_IN_SONG_DIR;
        else if (isButtonChecked(IDC_C_SAVE_IN_DIR))
            m_downSaveDir = DOWN_SAVE_IN_CUSTOM_DIR;
        else
            m_downSaveDir = DOWN_SAVE_NO_FILE;

        if (isButtonChecked(IDC_C_AS_SONG_NAME))
            m_downSaveName = DOWN_SAVE_NAME_AS_SONG_NAME;
        else
            m_downSaveName = DOWN_SAVE_NAME_KEEP;

        getEmbeddedOptCheck(this, m_vSaveEmbeddedLyrNames);

        bool bSaveInSongFile = saveBtnCheckStatus(IDC_C_SAVE_IN_SONG_FILE, "SaveInSongFile");
        if (!bSaveInSongFile)
            m_vSaveEmbeddedLyrNames.clear();

        if (m_downSaveDir == DOWN_SAVE_NO_FILE
            && m_vSaveEmbeddedLyrNames.empty())
            return;

        CBaseDialog::onOK();
    }

    void onCommand(uint32_t uID, uint32_t nNotifyCode)
    {
        if (uID == IDC_C_SAVE_IN_SONG_DIR)
        {
            if (isButtonChecked(IDC_C_SAVE_IN_SONG_DIR))
                checkButton(IDC_C_SAVE_IN_DIR, false);
        }
        else if (uID == IDC_C_SAVE_IN_DIR)
        {
            if (isButtonChecked(IDC_C_SAVE_IN_DIR))
                checkButton(IDC_C_SAVE_IN_SONG_DIR, false);
        }
        else if (uID == IDC_BR_DIR)
        {
            CFolderDialog dlg;
            dlg.setInitFolder(g_LyricsDownloader.getDefSavePath());
            if (dlg.doBrowse(this))
            {
                g_LyricsDownloader.setDefSavePath(dlg.getFolder());
                setDlgItemText(IDC_BR_DIR, g_LyricsDownloader.getDefSavePath());
            }
        }

        CEmbeddedOptDialog::onCommand(uID, nNotifyCode);
    }
    
public:
    bool            m_bOnlyUseLocalLyrics;
    DOWN_SAVE_DIR    m_downSaveDir;
    DOWN_SAVE_NAME    m_downSaveName;
    VecStrings        m_vSaveEmbeddedLyrNames;

};


class CDlgAdvRemovePrompt : public CEmbeddedOptDialog
{
public:
    CDlgAdvRemovePrompt() : CEmbeddedOptDialog(IDD_LD_ADV_REMOVE_LYR) {
    }

    bool onInitDialog()
    {
        CBaseDialog::onInitDialog();

        loadBtnCheckStatus(IDC_C_REMOVE_LYR_FILE, "RemoveLyrFile", false);

        if (!loadBtnCheckStatus(IDC_C_REMOVE_EMBEDDED_LYR, "RemoveEmbeddedLyr", false))
            enableEmbeddedLyr(false);

        loadEmbeddedOptCheck(this);

        return true;
    }

    virtual void onOK()
    {
        m_bRemoveAssociateLyrFile = saveBtnCheckStatus(IDC_C_REMOVE_LYR_FILE, "RemoveLyrFile");

        getEmbeddedOptCheck(this, m_vEmbeddedLyrNames);

        bool bRemoveEmbeddedLyr = saveBtnCheckStatus(IDC_C_REMOVE_EMBEDDED_LYR, "RemoveEmbeddedLyr");
        if (!bRemoveEmbeddedLyr)
            m_vEmbeddedLyrNames.clear();

        if (!m_bRemoveAssociateLyrFile && m_vEmbeddedLyrNames.empty())
            return;

        CBaseDialog::onOK();
    }

public:
    bool        m_bRemoveAssociateLyrFile;
    VecStrings    m_vEmbeddedLyrNames;

};


}
