#include "MPlayerApp.h"
#include "MPHelper.h"
#include "MLCmd.h"
#include "MediaDetectionService.h"


bool onSongOpenFileCmd(Window *pWndParent, bool bOpen) {
    string strExtentions;
    g_player.getFileOpenDlgExtention(strExtentions);
    CFileOpenDlg dlg("add Music File", "", strExtentions.c_str(), 0, true);

    if (dlg.doModal(pWndParent) == IDOK) {
        vector<string> vFiles;
        string strFile;

        dlg.getOpenFile(vFiles);

        if (bOpen) {
            g_player.clearNowPlaying();
        } else {
            g_player.setNowPlayingModified(true);
        }

        for (int i = 0; i < (int)vFiles.size(); i++) {
            g_player.addToNowPlaying(vFiles[i].c_str());
        }
        g_player.saveNowPlaying();
        if (bOpen) {
            g_player.play();
        }

        return true;
    }

    return false;
}

bool onSongOpenDirCmd(Window *pWndParent, bool bOpen) {
    string strFolder;

    CFolderDialog dlg(g_profile.getString("Last open Dir", ""));
    if (dlg.doBrowse(pWndParent) == IDOK) {
        strFolder = dlg.getFolder();

        g_profile.writeString("Last open Dir", strFolder.c_str());

        if (bOpen) {
            g_player.clearNowPlaying();
        } else {
            g_player.setNowPlayingModified(true);
        }

        g_player.addDirToNowPlaying(strFolder.c_str(), true);
        g_player.saveNowPlaying();
        if (bOpen) {
            g_player.play();
        }

        return true;
    }

    return false;
}

bool onCmdSongAddDirToMediaLib(Window *pWndParent) {
    string strFolder;

    CFolderDialog dlg(g_profile.getString("Last open Dir", ""));
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
    g_player.getFileOpenDlgExtention(strExtentions);
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
