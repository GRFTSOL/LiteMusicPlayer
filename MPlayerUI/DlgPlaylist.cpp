//
//  DlgPlaylist.cpp
//

#include "MPlayerApp.h"
#include "DlgPlaylist.hpp"
#include "../LyricsLib/CurrentLyrics.h"


class SkinWndNewPlaylist : public CMPSkinWnd {
public:
    void onCommand(uint32_t id) override {
        if (id == ID_OK) {
            auto name = getUIObjectText("CID_E_NAME");
            if (name.empty()) {
                return;
            }

            auto mediaLib = g_player.getMediaLibrary();
            auto pl = mediaLib->newPlaylist(name.c_str());
            pl->append(playlist.get());
            mediaLib->savePlaylist(pl);
        }

        CMPSkinWnd::onCommand(id);
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

class SkinWndOpenPlaylist : public CMPSkinWnd {
public:
    enum {
        IDX_NAME,
        IDX_DURATION,
        IDX_COUNT,
        IDX_DATE_MODIFIED,
    };

    void onCreate() override {
        CMPSkinWnd::onCreate();

        GET_ID_BY_NAME4(CID_MORE, CID_E_NAME, CID_DO_RENAME, CID_CANCEL_RENAME);

        _listCtrl = (CSkinListCtrl *)getUIObjectById("CID_PLAYLIST_CTRL", CSkinListCtrl::className());
        if (_listCtrl) {
            _listCtrl->addColumn(_TL("Name"), 150);
            _listCtrl->addColumn(_TL("Duration"), 80);
            _listCtrl->addColumn(_TL("Count"), 80);
            _listCtrl->addColumn(_TL("Date Modified"), 200);
            _listCtrl->loadColumnWidth("dlg_playlist");

            updatePlaylist();
        }

        _containerRename = (CSkinContainer *)getUIObjectById("CID_C_RENAME", CSkinContainer::className());

        _menu = m_pSkinFactory->loadMenu(this, "OpenSavePlaylistMenu");
    }

    void onCommand(uint32_t nId) override {
        if (nId == ID_OK) {
            if (_listCtrl->getSelectedCount() == 0) {
                return;
            }

            _listCtrl->saveColumnWidth("dlg_playlist");

            auto mediaLib = g_player.getMediaLibrary();
            g_player.clearNowPlaying();

            for (auto index : _listCtrl->getSelectedItems()) {
                auto id = _listCtrl->getItemData(index);
                auto playlist = mediaLib->getPlaylist(id);
                if (playlist) {
                    g_player.addToNowPlaying(playlist);
                }
            }

            g_player.setNowPlayingModified(true);
            g_player.saveNowPlaying();

            CMPSkinWnd::onCommand(nId);
        } else if (nId == CID_MORE) {
            auto obj = getUIObjectById(CID_MORE);
            if (_menu && obj) {
                auto countSelected = _listCtrl->getSelectedCount();
                CPoint pos(obj->m_rcObj.left, obj->m_rcObj.top);
                clientToScreen(pos);

                _menu->enableItem(ID_DELETE, countSelected >= 1);
                _menu->enableItem(ID_RENAME, countSelected == 1);
                _menu->trackPopupMenu(pos, this);
            }
        } else if (nId == CID_DO_RENAME && _containerRename) {
            auto name = getUIObjectText(CID_E_NAME);
            if (!name.empty()) {
                _containerRename->getParent()->switchToLastPage(0, true);

                auto mediaLib = g_player.getMediaLibrary();
                auto index = _listCtrl->getNextSelectedItem();
                if (index != -1) {
                    auto playlist = mediaLib->getPlaylist(_listCtrl->getItemData(index));
                    if (playlist && playlist->name != name) {
                        playlist->name = name;
                        playlist->refreshTimeModified();
                        mediaLib->savePlaylist(playlist);
                        updatePlaylist();
                    }
                }
            }
        } else if (nId == CID_CANCEL_RENAME && _containerRename) {
            _containerRename->getParent()->switchToLastPage(0, true);
        } else if (nId == ID_DELETE) {
            auto mediaLib = g_player.getMediaLibrary();
            for (auto index : _listCtrl->getSelectedItems()) {
                auto id = _listCtrl->getItemData(index);
                mediaLib->deltePlaylist(id);
            }

            updatePlaylist();
        } else if (nId == ID_RENAME) {
            if (_containerRename) {
                auto index = _listCtrl->getNextSelectedItem();
                if (index != -1) {
                    setUIObjectText(CID_E_NAME, _listCtrl->getItemText(index, IDX_NAME));
                    _containerRename->getParent()->switchToPage(_containerRename, false, 0, true);
                    setFocusUIObj(getUIObjectById(CID_E_NAME));
                }
            }
        } else {
            CMPSkinWnd::onCommand(nId);
        }
    }

    void updatePlaylist() {
        if (!_listCtrl) {
            return;
        }

        auto mediaLib = g_player.getMediaLibrary();
        auto playlists = mediaLib->getAllPlaylistBriefs();

        _listCtrl->deleteAllItems();
        for (auto &item : playlists) {
            auto index = _listCtrl->appendItem(item.name.c_str());
            _listCtrl->setItemData(index, item.id);
            _listCtrl->setItemText(index, IDX_DURATION, formatDuration(item.duration));
            _listCtrl->setItemText(index, IDX_COUNT, std::to_string(item.count));
            _listCtrl->setItemText(index, IDX_DATE_MODIFIED, DateTime(item.timeModified * 1000).toDateTimeString());
        }
    }

protected:
    SkinMenuPtr             _menu;

    int                     CID_MORE = -1;
    int                     CID_E_NAME = -1, CID_DO_RENAME = -1, CID_CANCEL_RENAME = -1;
    CSkinListCtrl           *_listCtrl = nullptr;
    CSkinContainer          *_containerRename = nullptr;

};

void showOpenPlaylistDialog(CSkinWnd *parent) {
    SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME,
        "DlgOpenPlaylist.xml", parent);

    auto *window = new SkinWndOpenPlaylist();
    skinWndStartupInfo.pSkinWnd = window;

    CSkinApp::getInstance()->getSkinFactory()->activeOrCreateSkinWnd(skinWndStartupInfo);
}

class SkinWndSavePlaylist : public CMPSkinWnd {
public:
    void onCreate() override {
        CMPSkinWnd::onCreate();

        GET_ID_BY_NAME4(CID_R_CREATE_NEW, CID_E_NAME, CID_R_OVERWRITE, CID_PLAYLIST_CTRL);

        auto mediaLib = g_player.getMediaLibrary();
        auto playlists = mediaLib->getAllPlaylistBriefs();

        updateControlStatus();

        _combox = (CSkinComboBox *)getUIObjectById(CID_PLAYLIST_CTRL, CSkinComboBox::className());
        if (!_combox) {
            return;
        }

        for (auto &item : playlists) {
            int idx = _combox->addString(item.name.c_str());
            _combox->setItemData(idx, item.id);
        }
    }

    void onCommand(uint32_t nId) override {
        if (nId == ID_OK) {
            if (!playlist) {
                return;
            }

            auto mediaLib = g_player.getMediaLibrary();

            if (_isCreateNew) {
                playlist->id = -1;
                playlist->name = getUIObjectText(CID_E_NAME);
                playlist->refreshTimeModified();
                mediaLib->savePlaylist(playlist);
            } else if (_combox) {
                auto idx = _combox->getCurSel();
                auto id = _combox->getItemData(idx);

                auto existing = mediaLib->getPlaylist(id);
                playlist->setInfo(existing->toPlaylistInfo());
                mediaLib->savePlaylist(playlist);
            }

            CMPSkinWnd::onCommand(nId);
        } else if (nId == CID_R_CREATE_NEW) {
            _isCreateNew = true;
            updateControlStatus();
        } else if (nId == CID_R_OVERWRITE) {
            _isCreateNew = false;
            updateControlStatus();
        } else {
            CMPSkinWnd::onCommand(nId);
        }
    }

    void updateControlStatus() {
        enableUIObject(CID_E_NAME, _isCreateNew);
        enableUIObject(CID_PLAYLIST_CTRL, !_isCreateNew);
        checkButton(CID_R_CREATE_NEW, _isCreateNew);
        checkButton(CID_R_OVERWRITE, !_isCreateNew);
    }

    PlaylistPtr                 playlist;

protected:
    int CID_R_CREATE_NEW = -1, CID_E_NAME = -1, CID_R_OVERWRITE = -1, CID_PLAYLIST_CTRL = -1;

    CSkinComboBox               *_combox = nullptr;
    bool                        _isCreateNew = true;

};

void showSavePlaylistDialog(CSkinWnd *parent, const PlaylistPtr &playlist) {
    SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME,
        "DlgSavePlaylist.xml", parent);

    auto *window = new SkinWndSavePlaylist();
    window->playlist = playlist;
    skinWndStartupInfo.pSkinWnd = window;

    CSkinApp::getInstance()->getSkinFactory()->activeOrCreateSkinWnd(skinWndStartupInfo);
}
