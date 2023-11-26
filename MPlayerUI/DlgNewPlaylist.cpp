//
//  DlgNewPlaylist.cpp
//

#include "MPlayerApp.h"
#include "DlgNewPlaylist.hpp"
#include "../LyricsLib/CurrentLyrics.h"


class SkinWndNewPlaylist : public CMPSkinWnd {
public:
    bool onCustomCommand(int nId) override {
        if (nId == CMD_OK) {
            auto name = getUIObjectText("CID_E_NAME");
            if (name.empty()) {
                return true;
            }

            auto mediaLib = g_player.getMediaLibrary();
            auto pl = mediaLib->newPlaylist(name.c_str());
            pl->append(playlist.get());
            mediaLib->savePlaylist(pl);
        }

        return CMPSkinWnd::CSkinWnd::onCustomCommand(nId);
    }

public:
    PlaylistPtr                 playlist;

};

void showNewPlaylistDialog(CSkinWnd *parent, const PlaylistPtr &playlist) {
    SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME,
        "DlgNewPlaylist.xml", parent);

    auto *window = new SkinWndNewPlaylist();
    window->playlist = playlist;

    skinWndStartupInfo.pSkinWnd = window;

    CSkinApp::getInstance()->getSkinFactory()->activeOrCreateSkinWnd(skinWndStartupInfo);
}

void showOpenPlaylistDialog(CSkinWnd *pParent) {

}

void showSavePlaylistDialog(CSkinWnd *pParent, const PlaylistPtr &playlist) {
    
}

