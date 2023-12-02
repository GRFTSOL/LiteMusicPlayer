#include "MPlayerAppBase.h"
#include "MPCmdHandlerOfMediaGuide.h"
#include "Player.h"
#include "MPHelper.h"


CMPCmdHandlerOfMediaGuide::CMPCmdHandlerOfMediaGuide() {

}

CMPCmdHandlerOfMediaGuide::~CMPCmdHandlerOfMediaGuide() {

}

void CMPCmdHandlerOfMediaGuide::init(CSkinWnd *pSkinWnd) {
    ISkinCmdHandler::init(pSkinWnd);

    m_mediaTree.init();

    if (m_vPathLatest.size()) {
        ISkinTreeNode *pNode;
        pNode = m_mediaTree.getNode(m_vPathLatest);
        if (pNode) {
            m_mediaTree.setSelNode(pNode);
        }
    }

    CSkinTreeCtrl *pTree = (CSkinTreeCtrl*)m_pSkinWnd->getUIObjectById(ID_MG_TREE_GUIDE, CSkinTreeCtrl::className());
    if (pTree) {
        pTree->setDataSrc(&m_mediaTree);
    }

    updateMediaList();
}

// if the command id is processed, return true.
bool CMPCmdHandlerOfMediaGuide::onCommand(uint32_t nId) {
    switch (nId) {
    case ID_PLAY:
        onDblClickMediaList();
        break;
    case ID_QUEUE_UP:
        {
            CSkinListCtrl *pMediaList = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_MG_MEDIA_LIST, CSkinListCtrl::className());

            auto playlist = m_mediaTree.getCurNodePlaylist();
            if (playlist) {
                auto curPl = g_player.getNowPlaying();
                int nSel = -1;
                while (1) {
                    nSel = pMediaList->getNextSelectedItem(nSel);
                    if (nSel == -1) {
                        break;
                    }
                    auto media = playlist->getItem(nSel);
                    if (media) {
                        curPl->insertItem(-1, media);
                    }
                }
                g_player.saveNowPlaying();
            }
        }
        break;
    case ID_QUEUE_UP_ALL:
        {
            auto playlist = m_mediaTree.getCurNodePlaylist();
            if (playlist) {
                auto curPl = g_player.getNowPlaying();
                int nCount = playlist->getCount();
                for (int i = 0; i < nCount; i++) {
                    auto media = playlist->getItem(i);
                    if (media) {
                        curPl->insertItem(-1, media);
                    }
                }
                g_player.saveNowPlaying();
            }
        }
        break;
    case ID_NOWPLAYING:
        g_mpApp.getSkinFactory()->activeOrCreateSkinWnd(false, "CMPSkin", "Playlist", "Playlist");
        break;
    case ID_ADD_DIR_TO_ML:
        //
        // add Media to library
        //
        if (onCmdSongAddDirToMediaLib(m_pSkinWnd)) {
            reloadMediaGuideView();
        }
        break;
    case ID_ADD_FILE_TO_ML:
        //
        // add files to library
        //
        if (onCmdSongAddFilesToMediaLib(m_pSkinWnd)) {
            reloadMediaGuideView();
        }
        break;
    case ID_REMOVE_FROM_ML:
        {
            // remove media from media library
            CSkinListCtrl *pMediaList = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_MG_MEDIA_LIST, CSkinListCtrl::className());
            if (pMediaList) {
                int nSel = pMediaList->getNextSelectedItem(-1);
                if (nSel == -1) {
                    return true;
                }

            auto playlist = m_mediaTree.getCurNodePlaylist();
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
                            mediaLib->remove(media.get(), false);

                            playlist->removeItem(nSel);
                            pMediaList->deleteItem(nSel, true);
                        }
                    }
                    // reloadMediaGuideView();
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

bool CMPCmdHandlerOfMediaGuide::onUIObjNotify(IUIObjNotify *pNotify) {
    if (pNotify->nID == ID_MG_MEDIA_LIST) {
        if (pNotify->pUIObject->isKindOf(CSkinListCtrl::className())) {
            IMPMediaTreeNode *pNode;
            CSkinListCtrl *pMediaList = (CSkinListCtrl*)pNotify->pUIObject;
            int nSel = pMediaList->getNextSelectedItem(-1);
            if (nSel == -1) {
                return true;
            }

            pNode = (IMPMediaTreeNode*)m_mediaTree.getSelNode();
            if (!pNode) {
                return true;
            }

            CSkinListCtrlEventNotify *pListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;
            if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_DBL_CLICK ||
                pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_ENTER) {
                if (pNode->m_playlist.p) {
                    g_player.setNowPlaying(pNode->m_playlist);
                    g_player.setCurrentMediaInNowPlaying(nSel);
                    g_player.play();
                    g_player.saveNowPlaying();
                    return true;
                }
                if (pNode->nodeType == MGNT_NOW_PLAYING) {
                    g_player.setCurrentMediaInNowPlaying(nSel);
                }
            }
        }

        return true;
    } else if (pNotify->nID == ID_MG_TREE_GUIDE) {
        if (pNotify->pUIObject->isKindOf(CSkinTreeCtrl::className())) {
            //
            // Change to new node of media tree guide, update media guide view.
            //
            CSkinTreeCtrlEventNotify *pEvent = (CSkinTreeCtrlEventNotify*)pNotify;
            if (pEvent->cmd == CSkinTreeCtrlEventNotify::C_SEL_CHANGED) {
                IMPMediaTreeNode *pNode = (IMPMediaTreeNode*)m_mediaTree.getSelNode();

                // update media guide view.
                if (pNode) {
                    updateMediaList();
                }
            }
        }
        return true;
    }

    return false;
}

void CMPCmdHandlerOfMediaGuide::onDblClickMediaList() {
    //    CSkinListCtrl    *pMediaList = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_MG_MEDIA_LIST, CSkinListCtrl::className());
    //    if (!pMediaList)
    //        return;
    //
    //    CSkinListCtrlEventNotify notify(pMediaList);
    //    notify.cmd = CSkinListCtrlEventNotify::C_DBL_CLICK;
    //
    //    onUIObjNotify(&notify);
}

void CMPCmdHandlerOfMediaGuide::reloadMediaGuideView() {
    m_mediaTree.close();
    m_mediaTree.init();
    m_mediaTree.getCurPath(m_vPathLatest);

    if (m_vPathLatest.size()) {
        ISkinTreeNode *pNode;
        pNode = m_mediaTree.getNode(m_vPathLatest);
        if (pNode) {
            m_mediaTree.setSelNode(pNode);
        }
    }
    updateMediaList();
}

void CMPCmdHandlerOfMediaGuide::updateMediaList() {
    IMPMediaTreeNode *pNode = (IMPMediaTreeNode*)m_mediaTree.getSelNode();

    CSkinListCtrl *plistCtrl;
    plistCtrl = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_MG_MEDIA_LIST, CSkinListCtrl::className());
    if (!plistCtrl) {
        return;
    }

    if (plistCtrl->getColumnCount() == 0) {
        plistCtrl->addColumn("", 230);
    }

    plistCtrl->deleteAllItems(false);

    // update media guide view.
    if (pNode == nullptr || pNode->m_playlist.p == nullptr) {
        plistCtrl->invalidate();
        return;
    }

    int nCount = pNode->m_playlist->getCount();
    string str;

    for (int i = 0; i < nCount; i++) {
        auto media = pNode->m_playlist->getItem(i);
        if (media) {
            str = g_player.formatMediaTitle(media.get());

            plistCtrl->insertItem(plistCtrl->getItemCount(),
                str.c_str(), II_MUSIC);
        }
    }

    if (plistCtrl->getItemCount()) {
        plistCtrl->setItemSelectionState(0, true);
    }
    plistCtrl->invalidate();
}

void CMPCmdHandlerOfMediaGuide::addHistoryPath() {
    HistroyItem item;
    CSkinListCtrl *plistCtrl;
    plistCtrl = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_MG_MEDIA_LIST, CSkinListCtrl::className());
    if (plistCtrl) {
        item.nSelChild = plistCtrl->getNextSelectedItem();
    } else {
        item.nSelChild = 0;
    }

    m_mediaTree.getCurPath(item.path);

    m_historyPath.push_back(item);
    if (m_historyPath.size() > 10) {
        m_historyPath.erase(m_historyPath.begin());
    }
}

void CMPCmdHandlerOfMediaGuide::backHistoryPath() {
    if (m_historyPath.size()) {
        CSkinListCtrl *plistCtrl;
        int nSelRow = m_historyPath.back().nSelChild;
        ISkinTreeNode *pNode;

        plistCtrl = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_MG_MEDIA_LIST, CSkinListCtrl::className());
        pNode = m_mediaTree.getNode(m_historyPath.back().path);
        if (plistCtrl && pNode) {
            m_mediaTree.setSelNode(pNode);

            updateMediaList();

            if (plistCtrl && nSelRow >= 0 && nSelRow < plistCtrl->getItemCount()) {
                plistCtrl->setItemSelectionState(0, false);
                plistCtrl->setItemSelectionState(nSelRow, true);
                plistCtrl->makeSureRowVisible(nSelRow);
                plistCtrl->invalidate();
            }

            CSkinTreeCtrl *pTree = (CSkinTreeCtrl*)m_pSkinWnd->getUIObjectById(ID_MG_TREE_GUIDE, CSkinTreeCtrl::className());
            if (pTree) {
                pTree->invalidate();
            }
        }
        m_historyPath.pop_back();
    }
}
