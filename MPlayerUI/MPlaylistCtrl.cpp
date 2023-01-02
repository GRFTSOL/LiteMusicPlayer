#include "MPlayerAppBase.h"
#include "MPlaylistCtrl.h"
#include "PlayListFile.h"


void splitKeywords(cstr_t keyword, VecStrings &vKeywords) {
    vKeywords.clear();

    auto start = keyword, p = keyword;
    while (*p) {
        while (isAlpha(*p) || isDigit(*p) || (unsigned int)(*p) >= 128) {
            p++;
        }

        if (p == start) {
            p++;
        }

        if (*start != ' ' && *start != '\t') {
            vKeywords.push_back(string());
            vKeywords.back().append(start, p);
        }

        start = p;
    }
}

static uint32_t searchWithKeywordEx(VecStrings &vKeywords, cstr_t text, CSkinListCtrl::VecTextColor &vItemClrs) {
    VecStrings keywordRemains = vKeywords;
    auto lastUnmatch = text, start = text, p = text;

    while (*p) {
        while (isAlpha(*p) || isDigit(*p)) {
            p++;
        }

        if (p == start) {
            p++;
        }

        if (*start != ' ' && *start != '\t') {
            for (VecStrings::size_type i = 0; i < keywordRemains.size(); i++) {
                if (int(p - start) == keywordRemains[i].size()
                    && strncasecmp(start, keywordRemains[i].c_str(), keywordRemains[i].size()) == 0) {
                    keywordRemains.erase(keywordRemains.begin() + i);

                    // set unmatched color
                    if (lastUnmatch != start) {
                        vItemClrs.add(int(start - lastUnmatch), CSkinListCtrl::CN_TEXT);
                    }

                    // set matched color
                    vItemClrs.add(int(p - start), CSkinListCtrl::CN_CUSTOMIZED_START);

                    lastUnmatch = p;
                    break;
                }
            }
            if (keywordRemains.empty()) {
                return 1;
            }
        }

        start = p;
    }

    if (keywordRemains.empty()) {
        return 1;
    }

    return 0;
}


static uint32_t searchWithKeyword(cstr_t keyword, cstr_t text, CSkinListCtrl::VecTextColor &vItemClrs) {
    vItemClrs.clear();

    auto dst = text;
    while (*dst) {
        auto dstStart = dst;
        auto key = keyword;

        while (*dst && toLower(*key) == toLower(*dst)) {
            key++;
            dst++;
        }

        if (*key == 0) {
            if (int(dstStart - text) > 0) {
                vItemClrs.add(int(dstStart - text), CSkinListCtrl::CN_TEXT);
            }
            vItemClrs.add(int(dst - dstStart), CSkinListCtrl::CN_CUSTOMIZED_START);
            return uint32_t(-1);
        }

        if (*dst) {
            dst++;
        }
    }

    return 0;
}

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
    m_editorSearch = nullptr;
}

CMPlaylistCtrl::~CMPlaylistCtrl() {
}

bool CMPlaylistCtrl::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinListCtrl::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, "KeywordEdit")) {
        m_idEditorSearch = szValue;
    } else {
        return false;
    }

    return true;
}

void CMPlaylistCtrl::onCreate() {
    CSkinListCtrl::onCreate();

    m_editorSearch = (CSkinEditCtrl *)m_pSkin->getUIObjectById(m_idEditorSearch, CSkinEditCtrl::className());
    if (m_editorSearch) {
        m_editorSearch->setEditNotification(this);
    }

    assert(getColumnCount() == 0);
    addColumn(_TLT("Index"), 40, CColHeader::TYPE_TEXT, false, DT_CENTER);
    addColumn(_TLT("Media Files"), 130, CColHeader::TYPE_TEXT_EX, false, DT_CENTER);
    addColumn(_TLT("Duration"), 50, CColHeader::TYPE_TEXT, false, DT_CENTER);

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_PLAYER_CUR_PLAYLIST_CHANGED, ET_PLAYER_CUR_MEDIA_CHANGED);

    updatePlaylist(false);
}

void CMPlaylistCtrl::onTimer(int nId) {
    if (!m_editorSearch) {
        return;
    }

    if (m_timerIdBeginSearch) {
        m_pSkin->unregisterTimerObject(this, m_timerIdBeginSearch);
        m_timerIdBeginSearch = 0;
    }

    string keyword = m_editorSearch->getText();

    trimStr(keyword);
    keyword = toLower(keyword.c_str());
    doSearch(keyword.c_str());
}

void CMPlaylistCtrl::onEvent(const IEvent *pEvent) {
    if (m_searchResults) {
        return;
    }

    if (pEvent->eventType == ET_PLAYER_CUR_PLAYLIST_CHANGED) {
        CEventPlaylistChanged *pEventPlaylistChanged = (CEventPlaylistChanged*)pEvent;
        onCurrentPlaylistEvent(pEventPlaylistChanged);
    } else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED) {
        setNowPlaying();

        CMPAutoPtr<IMedia> media;
        int row = g_Player.getCurrentMediaIndex();
        if (g_Player.getCurrentMedia(&media) == ERR_OK && row < getRowCount()) {
            string strTitle = g_Player.formatMediaTitle(media);
            setItemText(row, 1, strTitle.c_str());

            auto n = (media->getDuration() + 500) / 1000; // ms
            char buf[64];
            sprintf(buf, "%d:%02d", n / 60, n % 60);
            setItemText(row, 2, buf, false);
        }

        invalidate();
    }
}

void CMPlaylistCtrl::onKeyDown(uint32_t code, uint32_t flags) {
    CSkinListCtrl::onKeyDown(code, flags);

    if (m_vRows.empty() || m_nFocusUIObj != -1) {
        return;
    }

    onHandleKeyDown(code, flags);
}

void CMPlaylistCtrl::onHandleKeyDown(uint32_t code, uint32_t flags) {
    CSkinListCtrl::onHandleKeyDown(code, flags);

    if (code == VK_RETURN) {
        if (m_searchResults) {
            useResultAsNowPlaying();
        } else {
            int n = getNextSelectedItem(-1);
            if (n != -1) {
                g_Player.playMedia(n);
            }
        }
    } else if (code == VK_DELETE) {
        deleteSelectedItems();
    } else if (code == VK_UP || code == VK_DOWN) {
        bool ctrl = isModifierKeyPressed(MK_CONTROL, flags);
        if (ctrl) {
            offsetAllSelectedItems(code == VK_DOWN);
        }
    }
}

void CMPlaylistCtrl::onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) {
    // 当垂直滚动条位置在 0 时，显示.
    if (m_header && !m_header->isVisible() && nPos == 0) {
        m_header->setVisible(true, false);
    } else if (m_header && m_header->isVisible() && nPos > 0 && !m_editorSearch->isOnFocus()) {
        m_header->setVisible(false, false);
    }

    CSkinListCtrl::onVScroll(nSBCode, nPos, pScrollBar);
}

void CMPlaylistCtrl::sendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol) {
    if (cmd == CSkinListCtrlEventNotify::C_DBL_CLICK && nClickedRow != -1) {
        if (m_searchResults) {
            useResultAsNowPlaying();
        } else {
            g_Player.playMedia(nClickedRow);
        }
    }

    CSkinListCtrl::sendNotifyEvent(cmd, nClickedRow, nClickedCol);
}

void CMPlaylistCtrl::deleteSelectedItems() {
    if (m_searchResults) {
        return;
    }

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
    if (m_searchResults) {
        return;
    }

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

void CMPlaylistCtrl::onEditorTextChanged() {
    if (!m_editorSearch) {
        return;
    }

    if (m_timerIdBeginSearch != 0) {
        m_pSkin->unregisterTimerObject(this, m_timerIdBeginSearch);
    }

    // 输入文字之后启动定时器，更新搜索结果
    m_timerIdBeginSearch = m_pSkin->registerTimerObject(this, 1000);
}

void CMPlaylistCtrl::onEditorKeyDown(uint32_t code, uint32_t flags) {
    if (!m_editorSearch) {
        return;
    }

    onHandleKeyDown(code, flags);
}

void CMPlaylistCtrl::onEditorMouseWheel(int wheelDistance, int mkeys, CPoint pt) {
    onMouseWheel(wheelDistance, mkeys, pt);
}

void CMPlaylistCtrl::onEditorKillFocus() {
    if (m_header) {
        if (m_pVertScrollBar && m_pVertScrollBar->getScrollPos() > 0) {
            m_header->setVisible(false, false);
            invalidate();
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

void CMPlaylistCtrl::appendMedia(CMPAutoPtr<IMedia> &media, const CSkinListCtrl::VecTextColor &vItemClrs) {
    auto index = getRowCount();

    insertItem(index, itos(index + 1).c_str(), 0, 0, false);

    string strTitle = g_Player.formatMediaTitle(media);
    setItemTextEx(index, 1, strTitle.c_str(), vItemClrs);

    auto n = (media->getDuration() + 500) / 1000; // ms
    if (n > 0) {
        char buf[64];
        sprintf(buf, "%d:%02d", n / 60, n % 60);
        setItemText(index, 2, buf, false);
    }
}

void CMPlaylistCtrl::updateMediaIndex() {
    for (int i = 0; i < (int)m_vRows.size(); i++) {
        setItemText(i, 0, itos(i).c_str(), false);
    }

    invalidate();
}

void CMPlaylistCtrl::onCurrentPlaylistEvent(CEventPlaylistChanged *pEventPlaylistChanged) {
    if (pEventPlaylistChanged == nullptr || pEventPlaylistChanged->action == IMPEvent::PCA_FULL_UPDATE) {
        // reload playlist
        updatePlaylist(true);
    } else if (pEventPlaylistChanged->action == IMPEvent::PCA_CLEAR) {
        deleteAllItems();
    } else if (pEventPlaylistChanged->action == IMPEvent::PCA_INSERT) {
        CMPAutoPtr<IPlaylist> playlist;
        if (g_Player.getCurrentPlaylist(&playlist) != ERR_OK) {
            return;
        }

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

void CMPlaylistCtrl::updatePlaylist(bool isRedraw) {
    CMPAutoPtr<IPlaylist> playlist;

    if (m_searchResults) {
        playlist = m_searchResults;
    } else {
        if (g_Player.getCurrentPlaylist(&playlist) != ERR_OK) {
            return;
        }
    }

    deleteAllItems(false);

    int count = playlist->getCount();
    for (int i = 0; i < count; i++) {
        insertMedia(playlist, i);
    }

    setNowPlaying();

    if (isRedraw) {
        invalidate();
    }
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

void CMPlaylistCtrl::doSearch(cstr_t keyword) {
    // clear old results
    if (m_searchResults) {
        m_searchResults.release();
    }

    deleteAllItems(false);

    CMPAutoPtr<IMediaLibrary> mediaLib;
    g_Player.getMediaLibrary(&mediaLib);

    CMPAutoPtr<IPlaylist> playlist(mediaLib->getAll());
    // g_Player.getCurrentPlaylist(&playlist);

    if (isEmptyString(keyword)) {
        updatePlaylist(true);
        return;
    }

    if (g_Player.newPlaylist(&m_searchResults) != ERR_OK) {
        return;
    }

    //
    // search playlist with @keyword
    //

    CSkinListCtrl::VecTextColor vItemClrs;
    VecStrings vKeywords;
    splitKeywords(keyword, vKeywords);

    for (uint32_t i = 0; i < playlist->getCount(); i++) {
        CMPAutoPtr<IMedia> media;
        auto ret = playlist->getItem(i, &media);
        if (ret == ERR_OK) {
            string str = CPlayer::formatMediaTitle(media);

            auto matchValue = searchWithKeyword(keyword, str.c_str(), vItemClrs);
            if (matchValue > 0) {
                appendMedia(media, vItemClrs);
                m_searchResults->insertItem(-1, media);
            } else {
                matchValue = searchWithKeywordEx(vKeywords, str.c_str(), vItemClrs);
                if (matchValue > 0) {
                    appendMedia(media, vItemClrs);
                    m_searchResults->insertItem(-1, media);
                }
            }
        }
    }

    invalidate();
}

void CMPlaylistCtrl::useResultAsNowPlaying() {
    if (!m_searchResults || m_searchResults->getCount() == 0) {
        return;
    }

    int selected = getNextSelectedItem();
    if (selected == -1) {
        selected = 0;
    }

    g_Player.setCurrentPlaylist(m_searchResults);
    g_Player.setCurrentMediaInPlaylist(selected);
    g_Player.play();

    // string keyword = m_editorSearch->getText();
    // trimStr(keyword);

    // string strPlaylistFile = getAppDataDir();
    // strPlaylistFile += fileNameFilterInvalidChars(keyword.c_str());
    // strPlaylistFile += ".m3u";
    // savePlaylistAsM3u(m_searchResults, strPlaylistFile.c_str());
}
