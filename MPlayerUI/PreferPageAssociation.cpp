#include "MPlayerApp.h"
#include "PreferPageAssociation.h"
#include "MLCmd.h"

/*
#define MEDIA_FILE_ICON_ID  2

#define KEY_LOCATION        "Software\\Crintsoft\\DHPlayer\\MP3\\Location"
#define NAME_LOCATION       "Location"

#define KEY_AUDIOFILE       "DHPlayer AudioFile"
#define NAME_AUDIOFILE      "Audio File"
#define KEY_PLAYLIST        "DHPlayer Playlist"
#define NAME_PLAYLIST       "Playlist File"
#define VALUE_BACKUP        "DHPlayer Backup"

void associateFile(cstr_t pszExt, cstr_t pszKey, cstr_t pszName, uint32_t uIconID) {

}

void unassociateFile(cstr_t pszExt, cstr_t pszKey) {

}

bool isAssociatedFile(cstr_t pszExt, cstr_t pszKey) {

    return true;
}



CPreferPageAssociation::CPreferPageAssociation() : CPreferencePageBase(IDD_PG_MEDIA_ASSOCIATE, "Associations") {
}

CPreferPageAssociation::~CPreferPageAssociation() {

}

bool CPreferPageAssociation::onInitDialog() {
    CPreferencePageBase::onInitDialog();

    m_wndHelper.addAlignRightBottom(IDC_LIST_MEDIA_TYPE, false, false);
    m_wndHelper.addAlignBottom(IDC_CHECK_ALL, true);
    m_wndHelper.addAlignBottom(IDC_CLEAR_ALL, true);

    m_wndHelper.addAlignCenter(IDC_CHECK_ALL, false);
    m_wndHelper.addAlignCenter(IDC_CLEAR_ALL, false);

    m_listCtrl.attach(this, IDC_LIST_MEDIA_TYPE);
    m_listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

    m_listCtrl.InsertColumn(0, _TLT("File Types"), LVCFMT_LEFT, 150);

    vector<string> vExt;
    g_player.getSupportedExtentions(vExt);
    for (int i = 0; i < (int)vExt.size(); i++) {
        int n = m_listCtrl.insertItem(m_listCtrl.getItemCount(), vExt[i].c_str());
        if (g_player.isExtAudioFile(vExt[i].c_str())) {
            m_listCtrl.setCheck(n, isAssociatedFile(vExt[i].c_str(), KEY_AUDIOFILE));
        } else {
            m_listCtrl.setCheck(n, isAssociatedFile(vExt[i].c_str(), KEY_PLAYLIST));
        }
    }

    return true;
}

void CPreferPageAssociation::onDestroy() {
    char szExt[256];

    for (int i = 0; i < m_listCtrl.getItemCount(); i++) {
        emptyStr(szExt);
        m_listCtrl.getItemText(i, 0, szExt, CountOf(szExt));
        bool bPlaylist;
        bool bAssociated;

        bPlaylist = g_player.isExtPlaylistFile(szExt);
        if (bPlaylist) {
            bAssociated = isAssociatedFile(szExt, KEY_PLAYLIST);
        } else {
            bAssociated = isAssociatedFile(szExt, KEY_AUDIOFILE);
        }

        if (m_listCtrl.GetCheck(i)) {
            if (!bAssociated) {
                if (bPlaylist) {
                    associateFile(szExt, KEY_PLAYLIST, NAME_PLAYLIST, IDI_MEDIATYPE2);
                } else {
                    associateFile(szExt, KEY_AUDIOFILE, NAME_AUDIOFILE, IDI_MEDIATYPE2);
                }
            }
        } else {
            if (bAssociated) {
                if (bPlaylist) {
                    unassociateFile(szExt, KEY_PLAYLIST);
                } else {
                    unassociateFile(szExt, KEY_AUDIOFILE);
                }
            }
        }
    }

    CPreferencePageBase::onDestroy();
}

void CPreferPageAssociation::onCommand(uint32_t uID, uint32_t nNotifyCode) {
    CBaseDialog::onCommand(uID, nNotifyCode);

    switch (uID) {
    case IDC_CHECK_ALL:
    case IDC_CLEAR_ALL:
        {
            bool bCheck = uID == IDC_CHECK_ALL;
            for (int i = 0; i < m_listCtrl.getItemCount(); i++) {
                m_listCtrl.setCheck(i, bCheck);
            }
        }
        break;
    }
}
*/