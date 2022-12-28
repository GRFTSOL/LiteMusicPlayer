#include "MPlayerAppBase.h"
#include "MPHelper.h"
#include "MLCmd.h"
#include "MediaDetectionService.h"


void getDefaultPlaylistName(string &strPlaylistFile) {
    strPlaylistFile = getAppDataDir();
    strPlaylistFile += "DefPlaylist.m3u";
}

bool onSongOpenFileCmd(Window *pWndParent, bool bOpen) {
    string strExtentions;
    g_Player.getFileOpenDlgExtention(strExtentions);
    CFileOpenDlg dlg("add Music File", "", strExtentions.c_str(), 0, true);

    if (dlg.doModal(pWndParent) == IDOK) {
        vector<string> vFiles;
        string strFile;

        dlg.getOpenFile(vFiles);

        if (bOpen) {
            g_Player.newCurrentPlaylist();
        } else {
            g_Player.setPlaylistModified(true);
        }

        for (int i = 0; i < (int)vFiles.size(); i++) {
            g_Player.addToPlaylist(vFiles[i].c_str());
        }
        g_Player.saveCurrentPlaylist();
        if (bOpen) {
            g_Player.play();
        }

        return true;
    }

    return false;
}

bool onSongOpenDirCmd(Window *pWndParent, bool bOpen) {
    string strFolder;

    CFolderDialog dlg;
    dlg.setInitFolder(g_profile.getString("Last open Dir", ""));
    if (dlg.doBrowse(pWndParent) == IDOK) {
        strFolder = dlg.getFolder();

        g_profile.writeString("Last open Dir", strFolder.c_str());

        if (bOpen) {
            g_Player.clearPlaylist();
        } else {
            g_Player.setPlaylistModified(true);
        }

        g_Player.addDirToPlaylist(strFolder.c_str(), true);
        g_Player.saveCurrentPlaylist();
        if (bOpen) {
            g_Player.play();
        }

        return true;
    }

    return false;
}

bool onCmdSongAddDirToMediaLib(Window *pWndParent) {
    string strFolder;

    CFolderDialog dlg;
    dlg.setInitFolder(g_profile.getString("Last open Dir", ""));
    if (dlg.doBrowse(pWndParent) == IDOK) {
        strFolder = dlg.getFolder();

        g_profile.writeString("Last open Dir", strFolder.c_str());

        pWndParent->messageOut("The music in the folder will added in the background.");
        g_mediaDetectionService.addMediaInDir(strFolder.c_str());

        return true;
    }

    return false;
}

bool onCmdSongAddFilesToMediaLib(Window *pWndParent) {
    string strExtentions;
    g_Player.getFileOpenDlgExtention(strExtentions);
    CFileOpenDlg dlg("add Music File", "", strExtentions.c_str(), 0, true);

    if (dlg.doModal(pWndParent) == IDOK) {
        pWndParent->messageOut("The music in the folder will added in the background.");
        vector<string> vFiles;
        dlg.getOpenFile(vFiles);
        g_mediaDetectionService.addMedia(vFiles);

        return true;
    }

    return false;
}

void enumPlaylists(cstr_t szDir, int &nLevel, vector<string> &vFiles) {
    FileFind finder;
    string strDir, strFile;

    if (!finder.openDir(szDir)) {
        return;
    }

    strDir = szDir;
    dirStringAddSep(strDir);

    nLevel--;

    while (finder.findNext()) {
        if (finder.isCurDir()) {
            if (nLevel >= 0 &&
                strcmp(finder.getCurName(), ".") != 0 &&
                strcmp(finder.getCurName(), "..") != 0) {
                strFile = strDir + finder.getCurName();
                enumPlaylists(strFile.c_str(), nLevel, vFiles);
            }
        } else {
            if (CPlayer::isExtPlaylistFile(fileGetExt(finder.getCurName()))) {
                strFile = strDir + finder.getCurName();
                vFiles.push_back(strFile);
            }
        }
    }

    nLevel++;
}

void enumPlaylistsFast(vector<string> &vFiles) {
    int nLevel = 0;
    enumPlaylists(getAppDataDir().c_str(), nLevel, vFiles);

#ifdef _WIN32
    char szDir[MAX_PATH];
    if (SUCCEEDED(SHGetSpecialFolderPath(nullptr, szDir, CSIDL_PERSONAL, false))) {
        nLevel = 1;
        enumPlaylists(szDir, nLevel, vFiles);
    }

    if (SUCCEEDED(SHGetSpecialFolderPath(nullptr, szDir, CSIDL_DESKTOPDIRECTORY , false))) {
        nLevel = 0;
        enumPlaylists(szDir, nLevel, vFiles);
    }
#endif
}
