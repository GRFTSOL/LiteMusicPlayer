#include "MPlayerAppBase.h"
#include "MPlaylistCtrl.h"


bool deleteSelectedItemsInPlaylist(IPlaylist *playlist, vector<int> &vSelIndex) {
    MLRESULT nRet = ERR_OK;
    long n;
    long count = 0;

    assert(vSelIndex.size() > 0);
    if (vSelIndex.size() == 0) {
        return false;
    }

    sort(vSelIndex.begin(), vSelIndex.end());

    count = playlist->getCount();
    if (count == 0) {
        return false;
    }

    for (n = 0; n < (long)vSelIndex.size(); n++) {
        assert(vSelIndex[n] < count && vSelIndex[n] >= 0);
        nRet = playlist->removeItem(vSelIndex[n]);
        if (nRet != ERR_OK) {
            break;
        }
    }

    return nRet == ERR_OK;
}

bool offsetAllSelectedRowInPlaylist(IPlaylist *playlist, vector<int> &vSelIndex, bool bDown) {
    MLRESULT nRet = ERR_OK;
    long n;
    long count = 0;

    assert(vSelIndex.size() > 0);
    if (vSelIndex.size() == 0) {
        return false;
    }

    sort(vSelIndex.begin(), vSelIndex.end());

    count = playlist->getCount();
    if (count == 0) {
        return false;
    }

    if (bDown) {
        for (n = (long)vSelIndex.size() - 1; n >= 0; n--) {
            assert(vSelIndex[n] < count && vSelIndex[n] >= 0);
            nRet = playlist->moveItem(vSelIndex[n], vSelIndex[n] + 1);
            if (nRet != ERR_OK) {
                break;
            }
        }
    } else {
        for (n = 0; n < (long)vSelIndex.size(); n++) {
            assert(vSelIndex[n] < count && vSelIndex[n] >= 0);
            nRet = playlist->moveItem(vSelIndex[n], vSelIndex[n] - 1);
            if (nRet != ERR_OK) {
                break;
            }
        }
    }

    return nRet == ERR_OK;
}

//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPlaylistCtrl, "Playlist")

CMPlaylistCtrl::CMPlaylistCtrl() {
    m_bHorzScrollBar = false;
    m_nNowPlaying = -1;
}

CMPlaylistCtrl::~CMPlaylistCtrl() {
}

void CMPlaylistCtrl::onCreate() {
    CSkinListCtrl::onCreate();

    assert(getColumnCount() == 0);
    addColumn(_TLT("Index"), 40, CColHeader::TYPE_TEXT, false, DT_CENTER);
    addColumn(_TLT("Media Files"), 130);
    addColumn(_TLT("Duration"), 50, CColHeader::TYPE_TEXT, false, DT_CENTER);

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_PLAYER_CUR_PLAYLIST_CHANGED, ET_PLAYER_CUR_MEDIA_CHANGED);

    CMPAutoPtr<IPlaylist> playlist;
    int nRet = g_Player.getCurrentPlaylist(&playlist);
    if (nRet != ERR_OK) {
        return;
    }

    for (int i = 0; i < playlist->getCount(); i++) {
        insertMedia(playlist, i, false);
    }
}

void CMPlaylistCtrl::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_PLAYER_CUR_PLAYLIST_CHANGED) {
        CEventPlaylistChanged *pEventPlaylistChanged = (CEventPlaylistChanged*)pEvent;
        updatePlaylist(pEventPlaylistChanged);
    } else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED) {
        setNowPlaying();

        CMPAutoPtr<IMedia> media;
        if (g_Player.getCurrentMedia(&media) == ERR_OK) {
            string strTitle = g_Player.formatMediaTitle(media);
            setItemText(g_Player.getCurrentMediaIndex(), 1, strTitle.c_str());

            auto n = (media->getDuration() + 500) / 1000; // ms
            char buf[64];
            sprintf(buf, "%d:%02d", n / 60, n % 60);
            setItemText(g_Player.getCurrentMediaIndex(), 2, buf, false);
        }

        invalidate();
    }
}

void CMPlaylistCtrl::onKeyDown(uint32_t nChar, uint32_t nFlags) {
    if (m_vRows.empty()) {
        return;
    }

    if (nChar == VK_RETURN) {
        int n = getNextSelectedItem(-1);
        if (n != -1) {
            g_Player.playMedia(n);
        }
    } else if (nChar == VK_DELETE) {
        deleteSelectedItems();
        return;
    }
    if (nChar == VK_UP || nChar == VK_DOWN) {
        bool ctrl = isModifierKeyPressed(MK_CONTROL, nFlags);
        if (ctrl) {
            offsetAllSelectedItems(nChar == VK_DOWN);
        }
    }

    CSkinListCtrl::onKeyDown(nChar, nFlags);
}

void CMPlaylistCtrl::sendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol) {
    if (cmd == CSkinListCtrlEventNotify::C_DBL_CLICK && nClickedRow != -1) {
        g_Player.playMedia(nClickedRow);
    }

    CSkinListCtrl::sendNotifyEvent(cmd, nClickedRow, nClickedCol);
}

void CMPlaylistCtrl::deleteSelectedItems() {
    int nItem = -1;
    vector<int> vSelItems;

    while ((nItem = getNextSelectedItem(nItem)) != -1) {
        vSelItems.push_back(nItem);
    }

    if (vSelItems.empty()) {
        return;
    }

    CMPAutoPtr<IPlaylist> playlist;

    if (g_Player.getCurrentPlaylist(&playlist) == ERR_OK) {
        if (deleteSelectedItemsInPlaylist(playlist, vSelItems)) {
            nItem = vSelItems[0];
            if (nItem >= getItemCount()) {
                nItem = getItemCount() - 1;
            }
            if (nItem >= 0) {
                setItemSelectionState(nItem, true);
            }
            g_Player.setPlaylistModified(true);
        }
    }
}

void CMPlaylistCtrl::offsetAllSelectedItems(bool bMoveDown) {
    vector<int> vSelItems;
    int nItem = -1;

    while ((nItem = getNextSelectedItem(nItem)) != -1) {
        vSelItems.push_back(nItem);
    }

    CMPAutoPtr<IPlaylist> playlist;

    if (g_Player.getCurrentPlaylist(&playlist) == ERR_OK) {
        if (offsetAllSelectedRowInPlaylist(playlist, vSelItems, bMoveDown)) {
            // UpdateButtonState();
            g_Player.setPlaylistModified(true);
        }
    }
}

void CMPlaylistCtrl::insertMedia(CMPAutoPtr<IPlaylist> &playlist, int index, bool redraw) {
    CMPAutoPtr<IMedia> media;

    if (playlist->getItem(index, &media) == ERR_OK) {
        insertItem(index, itos(index + 1).c_str(), 0, 0, false);

        string strTitle = g_Player.formatMediaTitle(media);
        setItemText(index, 1, strTitle.c_str(), false);

        auto n = (media->getDuration() + 500) / 1000; // ms
        if (n > 0) {
            char buf[64];
            sprintf(buf, "%d:%02d", n / 60, n % 60);
            setItemText(index, 2, buf, false);
        }
    }

    if (redraw) {
        invalidate();
    }
}

void CMPlaylistCtrl::updateMediaIndex() {
    for (int i = 0; i < (int)m_vRows.size(); i++) {
        setItemText(i, 0, itos(i).c_str(), false);
    }

    invalidate();
}

void CMPlaylistCtrl::updatePlaylist(CEventPlaylistChanged *pEventPlaylistChanged) {
    long nCount;
    CMPAutoPtr<IPlaylist> playlist;

    if (g_Player.getCurrentPlaylist(&playlist) != ERR_OK) {
        return;
    }

    if (pEventPlaylistChanged == nullptr || pEventPlaylistChanged->action == IMPEvent::PCA_FULL_UPDATE) {
        // reload playlist
        deleteAllItems();

        nCount = playlist->getCount();
        for (int i = 0; i < nCount; i++) {
            insertMedia(playlist, i);
        }

        setNowPlaying();

        invalidate();
    } else if (pEventPlaylistChanged->action == IMPEvent::PCA_CLEAR) {
        deleteAllItems();
    } else if (pEventPlaylistChanged->action == IMPEvent::PCA_INSERT) {
        insertMedia(playlist, pEventPlaylistChanged->nIndex, false);
        updateMediaIndex();
    } else if (pEventPlaylistChanged->action == IMPEvent::PCA_MOVE) {
        int nIndex = pEventPlaylistChanged->nIndex;
        int nIndexOld = pEventPlaylistChanged->nIndexOld;
        if (nIndex >= 0 && nIndex < getItemCount()
            && nIndexOld >= 0 && nIndexOld < getItemCount()) {
            Row *temp = m_vRows[nIndex];
            m_vRows[nIndex] = m_vRows[nIndexOld];
            m_vRows[nIndexOld] = temp;
        }
        updateMediaIndex();
    } else if (pEventPlaylistChanged->action == IMPEvent::PCA_REMOVE) {
        int nSel = getNextSelectedItem();
        int nIndex = pEventPlaylistChanged->nIndex;
        if (nIndex >= 0 && nIndex < getItemCount()) {
            deleteItem(pEventPlaylistChanged->nIndex, false);
        }

        if (nSel != -1) {
            if (nSel >= getItemCount()) {
                nSel = getItemCount() - 1;
            }
            if (nSel != -1) {
                m_pSkin->enterInDrawUpdate();
                setItemSelectionState(nSel, true);
                m_pSkin->leaveInDrawUpdate();
            }
        }

        updateMediaIndex();
    } else {
        assert(0);
    }
}


int CMPlaylistCtrl::getItemImageIndex(int nItem) {
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    if (nItem >= 0 && nItem < (int)m_vRows.size()) {
        Row *row = m_vRows[nItem];
        return row->nImageIndex;
    }

    return -1;
}

void CMPlaylistCtrl::setNowPlaying() {
    // clear the old now playing item status.
    bool bOldStatusCleared = false;
    if (m_nNowPlaying >= 0 && m_nNowPlaying < (int)m_vRows.size()) {
        Row *row = m_vRows[m_nNowPlaying];
        if (row->nImageIndex == IMAGE_NOW_PLAYING) {
            row->nImageIndex = IMAGE_NONE;
            bOldStatusCleared = true;
        }
    }

    if (!bOldStatusCleared) {
        for (int i = 0; i < (int)m_vRows.size(); i++) {
            Row *row = m_vRows[i];
            if (row->nImageIndex == IMAGE_NOW_PLAYING) {
                row->nImageIndex = IMAGE_NONE;
                break;
            }
        }
    }

    if (m_vRows.size() > 0) {
        setItemImageIndex(g_Player.getCurrentMediaIndex(), IMAGE_NOW_PLAYING, false);
    }
}
