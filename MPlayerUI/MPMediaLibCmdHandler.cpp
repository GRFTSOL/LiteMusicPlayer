#include "MPlayerApp.h"
#include "MPMediaLibCmdHandler.h"
#include "MPHelper.h"


void CMediaLibTreeProvider::init() {
    Item item;

    item.bUptodate = false;
    item.folderType = FT_MEDIA_LIB;
    item.name = "Media Library";
    item.nImageIndex = II_MEDIA_LIB;
    m_tree.addChild(item);

    m_tree.chToChild(0);

    item.bUptodate = false;
    item.folderType = FT_ALL_SONGS_BY_ARTIST;
    item.name = "All Songs By Artist";
    item.nImageIndex = II_MUSIC;
    m_tree.addChild(item);

    item.folderType = FT_ALL_ARTIST;
    item.name = "Artist";
    item.nImageIndex = II_ARTIST;
    m_tree.addChild(item);

    item.folderType = FT_ALL_ALBUM;
    item.name = "Album";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_ALL_PLAYLIST_FILES;
    item.name = "Playlists";
    item.nImageIndex = II_PLAYLISTS;
    m_tree.addChild(item);

    item.folderType = FT_TOP_PLAYED;
    item.name = "Top 50 played";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_TOP_RATING;
    item.name = "Top 50 rating";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_RECENT_PLAYED;
    item.name = "Recent played";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_RECENT_ADDED;
    item.name = "Recent Added";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_NOW_PLAYING;
    item.name = "Now Playing";
    item.nImageIndex = II_NOW_PLAYING;
    m_tree.addChild(item);

    m_mediaLib = g_player.getMediaLibrary();
}

void CMediaLibTreeProvider::close() {
    m_tree.clear();
}

void CMediaLibTreeProvider::update() {

}

bool CMediaLibTreeProvider::enumChildren(V_ITEMS &vItems) {
    for (int i = 0; i < m_tree.getChildrenCount(); i++) {
        vItems.push_back(m_tree.getChild(i));
    }

    return true;
}

bool CMediaLibTreeProvider::chToChild(int nIndex) {
    if (!m_tree.chToChild(nIndex)) {
        return false;
    }

    Item &item = m_tree.getCurNodeData();

    if (item.bUptodate) {
        return true;
    }

    PlaylistPtr playlist;
    ResultCode nRet = ERR_OK;

    item.bUptodate = true;

    if (item.folderType == FT_ALL_PLAYLIST_FILES) {
        // nRet = m_mediaLib->getAll(&playlist);
        vector<string> vFiles;
        Item newitem;

        newitem.bUptodate = false;
        newitem.folderType = FT_PLAYLIST_FILE;
        newitem.nImageIndex = II_PLAYLISTS;

        // enumPlaylists("\\", nLevel, vFiles);
        enumPlaylistsFast(vFiles);
        for (int i = 0; i < (int)vFiles.size(); i++) {
            newitem.name = vFiles[i];
            m_tree.addChild(newitem);
        }
    } else if (item.folderType == FT_PLAYLIST_FILE) {
        g_player.clearNowPlaying();
        g_player.addToNowPlaying(item.name.c_str());
        g_player.saveNowPlaying();
    } else if (item.folderType == FT_ALL_SONGS_BY_ARTIST) {
        playlist = m_mediaLib->getAll(MLOB_ARTIST, -1);
    } else if (item.folderType == FT_ALL_ARTIST) {
        Item newitem;

        newitem.bUptodate = false;
        newitem.folderType = FT_ARTIST;
        newitem.nImageIndex = II_ARTIST;

        VecStrings results = m_mediaLib->getAllArtist();
        for (auto &name : results) {
            newitem.name = name;
            if (newitem.name.empty()) {
                newitem.name = "Unknown artist";
                newitem.folderType = FT_ARTIST_UNKNOWN;
                m_tree.addChild(newitem);
                newitem.folderType = FT_ARTIST;
                continue;
            }
            m_tree.addChild(newitem);
        }
    } else if (item.folderType == FT_ALL_ALBUM) {
        Item newitem;

        newitem.bUptodate = false;
        newitem.folderType = FT_ALBUM;
        newitem.nImageIndex = II_ALBUM;

        VecStrings results = m_mediaLib->getAllAlbum();
        for (auto &name : results) {
            newitem.name = name;
            if (newitem.name.empty()) {
                newitem.name = "Unknown Album";
                newitem.folderType = FT_ALBUM_UNKNOWN;
                m_tree.addChild(newitem);
                newitem.folderType = FT_ALBUM;
                continue;
            }
            m_tree.addChild(newitem);
        }
    } else if ((item.folderType == FT_ARTIST || item.folderType == FT_ARTIST_UNKNOWN)) {
        Item newitem;

        newitem.bUptodate = false;
        newitem.folderType = FT_ARTIST_ALL_MUSIC;
        newitem.nImageIndex = II_MUSIC;
        newitem.name = "All songs by artist";
        m_tree.addChild(newitem);

        newitem.bUptodate = false;
        newitem.folderType = FT_ARTIST_TOP_RATING;
        newitem.nImageIndex = II_MUSIC;
        newitem.name = "top rated songs by artist";
        m_tree.addChild(newitem);

        newitem.bUptodate = false;
        newitem.nImageIndex = II_ALBUM;
        newitem.folderType = FT_ARTIST_ALBUM;

        VecStrings results = m_mediaLib->getAlbumOfArtist(item.getValue());
        for (auto &name : results) {
            newitem.name = name;
            if (newitem.name.empty()) {
                newitem.name = "Unknown album";
                newitem.folderType = FT_ARTIST_ALBUM_UNKNOWN;
                m_tree.addChild(newitem);
                newitem.folderType = FT_ARTIST_ALBUM;
                continue;
            }
            m_tree.addChild(newitem);
        }
        item.bUptodate = true;
    } else if (item.folderType == FT_ARTIST_ALBUM ||
        item.folderType == FT_ARTIST_ALBUM_UNKNOWN ||
        item.folderType == FT_ARTIST_TOP_RATING ||
        item.folderType == FT_ARTIST_ALL_MUSIC) {
        Item &itemArtist = m_tree.getParentNodeData();

        if (item.folderType == FT_ARTIST_ALL_MUSIC) {
            playlist = m_mediaLib->getByArtist(itemArtist.getValue(), MLOB_NONE, -1);
        } else if (item.folderType == FT_ARTIST_TOP_RATING) {
            playlist = m_mediaLib->getByArtist(itemArtist.getValue(), MLOB_RATING, -1);
            if (playlist) {
                g_player.filterLowRatingMedia(playlist.get());
            }
        } else {
            playlist = m_mediaLib->getByAlbum(itemArtist.getValue(), item.getValue(), MLOB_NONE, -1);
        }
    } else if (item.folderType == FT_ALBUM_UNKNOWN ||
        item.folderType == FT_ALBUM) {
        playlist = m_mediaLib->getByAlbum(item.getValue(), MLOB_NONE, -1);
    } else if (item.folderType == FT_TOP_RATING) {
        playlist = m_mediaLib->getTopRating(16);
        if (playlist) {
            g_player.filterLowRatingMedia(playlist.get());
        }
    } else if (item.folderType == FT_TOP_PLAYED) {
        playlist = m_mediaLib->getTopPlayed(16);
    } else if (item.folderType == FT_RECENT_ADDED) {
        playlist = m_mediaLib->getRecentAdded(16);
    } else if (item.folderType == FT_RECENT_PLAYED) {
        playlist = m_mediaLib->getRecentPlayed(16);
    }

    if (playlist) {
        addPlaylistToTree(playlist);
        item.playlist = playlist;
    }

    return true;
}

bool CMediaLibTreeProvider::chToParent() {
    return m_tree.chToParent();
}

bool CMediaLibTreeProvider::chToPath(V_ITEMS &vPath) {
    m_tree.chToRoot();

    for (int i = 0; i < (int)vPath.size(); i++) {
        int nCount = m_tree.getChildrenCount();
        int k;
        for (k = 0; k < nCount; k++) {
            if (strcmp(m_tree.getChild(k).name.c_str(), vPath[i].name.c_str()) == 0) {
                chToChild(k);
                break;
            }
        }
        if (k == nCount) {
            return false;
        }
    }

    return true;
}
//
// bool CMediaLibTreeProvider::chToPath(int nIndex)
// {
//     return m_tree.chToChild(nIndex);
// }

bool CMediaLibTreeProvider::getPath(V_ITEMS &vPath) {
    m_tree.getPath(vPath);

    for (int i = 0; i < (int)vPath.size(); i++) {
        vPath[i].playlist = nullptr;
    }

    return true;
}

CMediaLibTreeProvider::Item CMediaLibTreeProvider::getChildData(int n) {
    return m_tree.getChild(n);
}

bool CMediaLibTreeProvider::isCurNodePlaylist() {
    return m_tree.getCurNodeData().playlist != nullptr;
}

PlaylistPtr CMediaLibTreeProvider::getCurNodePlaylist() {
    Item &item = m_tree.getCurNodeData();
    return item.playlist;
}

bool CMediaLibTreeProvider::isCurNodePlaylistFile() {
    Item &item = m_tree.getCurNodeData();
    if (item.folderType == FT_ALL_PLAYLIST_FILES) {
        return true;
    }

    return false;
}

bool CMediaLibTreeProvider::getCurNodePlaylistFile(int nChildPos, string &strPlaylistFile) {
    if (nChildPos >= 0 && nChildPos < m_tree.getChildrenCount()) {
        strPlaylistFile = m_tree.getChild(nChildPos).name;
    } else {
        strPlaylistFile.resize(0);
    }

    return true;
}

void CMediaLibTreeProvider::addPlaylistToTree(const PlaylistPtr &playlist) {
    Item newitem;

    newitem.bUptodate = true;
    newitem.folderType = FT_MEDIA_FILE;
    newitem.nImageIndex = II_MUSIC;

    int nCount = playlist->getCount();
    for (int i = 0; i < nCount; i++) {
        auto media = playlist->getItem(i);
        if (media) {
            newitem.name = g_player.formatMediaTitle(media.get());
            m_tree.addChild(newitem);
        }
    }
}


CMPMediaLibCmdHandler::CMPMediaLibCmdHandler() {

}

CMPMediaLibCmdHandler::~CMPMediaLibCmdHandler() {

}

void CMPMediaLibCmdHandler::init(CSkinWnd *pSkinWnd) {
    ISkinCmdHandler::init(pSkinWnd);

    m_mediaLibTree.init();

    if (m_vPathLatest.size()) {
        m_mediaLibTree.chToPath(m_vPathLatest);
    }

    reloadMedialibView();
    updateMediaList();
}

// if the command id is processed, return true.
bool CMPMediaLibCmdHandler::onCommand(uint32_t nId) {
    switch (nId) {
    case ID_PLAY:
        onDblClickMediaList();
        break;
    case IDOK:
        m_pSkinWnd->destroy();
        break;
    case ID_QUEUE_UP:
        {
            CSkinListCtrl *pMediaList = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
            int nSel = pMediaList->getNextSelectedItem(-1);
            if (nSel == -1) {
                break;
            }

            auto playlist = m_mediaLibTree.getCurNodePlaylist();
            if (playlist) {
                auto media = playlist->getItem(nSel);
                if (media) {
                    auto curPl = g_player.getNowPlaying();
                    curPl->insertItem(-1, media);
                }
            }
        }
        break;
    case ID_NOWPLAYING:
        {
            SkinWndStartupInfo startInfo("MPSkin", "Playlist", "Playlist.xml", nullptr);
            MPlayerApp::getMPSkinFactory()->activeOrCreateSkinWnd(startInfo);
        }
        break;
    case ID_ADD_DIR_TO_ML:
        //
        // add Media to library
        //
        if (onCmdSongAddDirToMediaLib(m_pSkinWnd)) {
            reloadMedialibView();
        }
        break;
    case ID_REMOVE_FROM_ML:
        {
            CSkinListCtrl *pMediaList = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
            if (pMediaList) {
                int nSel = pMediaList->getNextSelectedItem(-1);
                if (nSel == -1) {
                    return true;
                }

                auto playlist = m_mediaLibTree.getCurNodePlaylist();
                if (playlist) {
                    vector<int> vIndex;
                    while (nSel != -1) {
                        vIndex.push_back(nSel);
                        nSel = pMediaList->getNextSelectedItem(nSel);
                    }
                    for (int i = (int)vIndex.size() - 1; i >= 0; i--) {
                        nSel = vIndex[i];
                        auto media = playlist->getItem(nSel);
                        if (media) {
                            auto mediaLib = g_player.getMediaLibrary();
                            mediaLib->remove(media, false);
                            playlist->removeItem(nSel);
                            pMediaList->deleteItem(nSel, true);
                            m_mediaLibTree.eraseChild(nSel);
                        }
                    }
                    reloadMedialibView();
                }
            }
        }
        break;
    case ID_ML_BACK:
        backHistoryPath();
        break;
    default:
        return false;
    }

    return true;
}

bool CMPMediaLibCmdHandler::onUIObjNotify(IUIObjNotify *pNotify) {
    if (pNotify->nID == ID_ML_MEDIA_LIST) {
        if (pNotify->pUIObject->isKindOf(CSkinListCtrl::className())) {
            CSkinListCtrl *pMediaList = (CSkinListCtrl*)pNotify->pUIObject;
            int nSel = pMediaList->getNextSelectedItem(-1);
            if (nSel == -1) {
                return true;
            }

            CSkinListCtrlEventNotify *pListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;
            if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_DBL_CLICK ||
                pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_ENTER ||
                pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_CLICK) {
                if (m_mediaLibTree.isCurNodePlaylist()) {
                    if (pListCtrlNotify->cmd != CSkinListCtrlEventNotify::C_CLICK) {
                        // play current music...
                        auto playlist = m_mediaLibTree.getCurNodePlaylist();
                        if (playlist) {
                            g_player.setNowPlaying(playlist);
                            g_player.setCurrentMediaInNowPlaying(nSel);
                            g_player.play();
                        }
                    }
                } else if (m_mediaLibTree.isCurNodePlaylistFile()) {
                    if (pListCtrlNotify->cmd != CSkinListCtrlEventNotify::C_CLICK) {
                        // play selected playlist
                        string strFile;
                        if (m_mediaLibTree.getCurNodePlaylistFile(nSel, strFile)) {
                            g_player.clearNowPlaying();
                            g_player.addToNowPlaying(strFile.c_str());
                            g_player.saveNowPlaying();
                            g_player.play();
                        }
                    }
                } else {
                    CMediaLibTreeProvider::Item item;

                    item = m_mediaLibTree.getChildData(nSel);
                    if (item.folderType == CMediaLibTreeProvider::FT_NOW_PLAYING) {
                        SkinWndStartupInfo startInfo("MPSkin", "Playlist", "Playlist.xml", nullptr);
                        MPlayerApp::getMPSkinFactory()->activeOrCreateSkinWnd(startInfo);
                    } else {
                        addHistoryPath();
                        m_mediaLibTree.chToChild(nSel);
                        updateMediaList();
                    }
                }

                //                 if (bClose)
                //                 {
                //                     m_pSkinWnd->destroy();
                //                 }
            }
        }

        return true;
    }

    return false;
}

void CMPMediaLibCmdHandler::onDblClickMediaList() {
    CSkinListCtrl *pMediaList = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
    if (!pMediaList) {
        return;
    }

    CSkinListCtrlEventNotify notify(pMediaList, -1, -1);
    notify.cmd = CSkinListCtrlEventNotify::C_DBL_CLICK;

    onUIObjNotify(&notify);
}

void CMPMediaLibCmdHandler::reloadMedialibView() {
    m_mediaLibTree.close();
    m_mediaLibTree.init();
    m_mediaLibTree.getPath(m_vPathLatest);
    if (m_vPathLatest.size()) {
        m_mediaLibTree.chToPath(m_vPathLatest);
        updateMediaList();
    }

}

void CMPMediaLibCmdHandler::updateMediaList() {
    CSkinListCtrl *plistCtrl;
    plistCtrl = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
    if (plistCtrl) {
        CMediaLibTreeProvider::V_ITEMS vItems;

        m_mediaLibTree.enumChildren(vItems);

        if (plistCtrl->getColumnCount() == 0) {
            plistCtrl->addColumn("", 230);
        }

        plistCtrl->deleteAllItems(false);

        for (int i = 0; i < (int)vItems.size(); i++) {
            CMediaLibTreeProvider::Item &item = vItems[i];
            if (item.folderType == CMediaLibTreeProvider::FT_PLAYLIST_FILE) {
                plistCtrl->insertItem(plistCtrl->getItemCount(),
                    fileGetTitle(item.name.c_str()).c_str(), item.nImageIndex);
            } else {
                plistCtrl->insertItem(plistCtrl->getItemCount(),
                    item.name.c_str(), item.nImageIndex);
            }
        }

        if (vItems.size()) {
            plistCtrl->setItemSelectionState(0, true);
        }
        plistCtrl->invalidate();
    }
}

void CMPMediaLibCmdHandler::addHistoryPath() {
    HistroyItem item;
    CSkinListCtrl *plistCtrl;
    plistCtrl = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
    if (plistCtrl) {
        item.nSelChild = plistCtrl->getNextSelectedItem();
    } else {
        item.nSelChild = 0;
    }

    m_mediaLibTree.getPath(item.path);

    m_historyPath.push_back(item);
    if (m_historyPath.size() > 10) {
        m_historyPath.erase(m_historyPath.begin());
    }
}

void CMPMediaLibCmdHandler::backHistoryPath() {
    if (m_historyPath.size()) {
        CSkinListCtrl *plistCtrl;
        int nSelRow = m_historyPath.back().nSelChild;

        m_mediaLibTree.chToPath(m_historyPath.back().path);
        updateMediaList();
        m_historyPath.pop_back();

        plistCtrl = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
        if (plistCtrl && nSelRow >= 0 && nSelRow < plistCtrl->getItemCount()) {
            plistCtrl->setItemSelectionState(0, false);
            plistCtrl->setItemSelectionState(nSelRow, true);
            plistCtrl->makeSureRowVisible(nSelRow);
            plistCtrl->invalidate();
        }
    }
}
