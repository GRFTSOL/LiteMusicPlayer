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
            vKeywords.push_back(string(start, p));
        }

        start = p;
    }
}

static uint32_t searchWithKeywordEx(const VecStrings &vKeywords, cstr_t text, CSkinListCtrl::VecTextColor &vItemClrs) {
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

        if (m_nowPlayingRow != -1) {
            makeSureRowVisible(m_nowPlayingRow);
        }

        CMPAutoPtr<IMedia> media;
        int row = g_Player.getCurrentMediaIndex();
        if (g_Player.getCurrentMedia(&media) == ERR_OK && row < getRowCount()) {
            string strTitle = g_Player.formatMediaTitle(media);
            setItemText(row, 1, strTitle.c_str());

            setItemMediaDuration(row, (media->getDuration() + 500) / 1000); // ms
        }

        invalidate();
    }
}

bool CMPlaylistCtrl::onCommand(int nId) {
    if (nId == IDC_SHOW_IN_FINDER) {
        CMPAutoPtr<IPlaylist> playlist;
        if (m_searchResults) {
            playlist = m_searchResults;
        } else {
            g_Player.getCurrentPlaylist(&playlist);
        }

        VecStrings files;
        int i = -1;
        while (true) {
            i = getNextSelectedItem(i);
            if (i >= 0 && i < (int)playlist->getCount()) {
                CMPAutoPtr<IMedia> media;
                if (playlist->getItem(i, &media) == ERR_OK) {
                    CXStr url;
                    media->getSourceUrl(&url);
                    files.push_back(url.c_str());
                }
            } else {
                break;
            }
        }

        showInFinder(files);
        return true;
    }

    return false;
}

bool CMPlaylistCtrl::onKeyDown(uint32_t code, uint32_t flags) {
    return CSkinListCtrl::onKeyDown(code, flags);
}

bool CMPlaylistCtrl::onHandleKeyDown(uint32_t code, uint32_t flags) {
    if (CSkinListCtrl::onHandleKeyDown(code, flags)) {
        return true;
    }

    if (code == VK_RETURN) {
        playItem(getNextSelectedItem(-1));
    } else if (code == VK_ESCAPE) {
        if (m_editorSearch) {
            m_editorSearch->setText("");
        }

        m_searchResults.release();
        m_categoriesResults.clear();
        updatePlaylist(true);
    } else if (code == VK_DELETE) {
        deleteSelectedItems();
    } else if (code == VK_UP || code == VK_DOWN) {
        bool ctrl = isModifierKeyPressed(MK_CONTROL, flags);
        if (ctrl) {
            offsetAllSelectedItems(code == VK_DOWN);
        }
    } else {
        return false;
    }

    return true;
}

void CMPlaylistCtrl::onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) {
    showHideEditorSearch(nPos);

    CSkinListCtrl::onVScroll(nSBCode, nPos, pScrollBar);
}

void CMPlaylistCtrl::sendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol) {
    if (cmd == CSkinListCtrlEventNotify::C_DBL_CLICK && nClickedRow != -1) {
        playItem(nClickedRow);
    }

    CSkinListCtrl::sendNotifyEvent(cmd, nClickedRow, nClickedCol);
}

void CMPlaylistCtrl::makeSureRowVisible(int nRow) {
    CSkinListCtrl::makeSureRowVisible(nRow);

    showHideEditorSearch(m_nFirstVisibleRow);
}

void CMPlaylistCtrl::showHideEditorSearch(int row) {
    if (m_header && !m_header->isVisible() && row == 0) {
        m_header->setVisible(true, false);
    } else if (m_header && m_header->isVisible() && row > 0 && !m_editorSearch->isOnFocus()) {
        m_header->setVisible(false, false);
    }
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

        setItemMediaDuration(index, (media->getDuration() + 500) / 1000); // ms
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

    setItemMediaDuration(index, (media->getDuration() + 500) / 1000); // ms
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

void CMPlaylistCtrl::setItemMediaDuration(int index, int seconds) {
    if (seconds > 0) {
        char buf[64];
        sprintf(buf, "%d:%02d", seconds / 60, seconds % 60);
        setItemText(index, 2, buf, false);
    }
}

void CMPlaylistCtrl::playItem(int index) {
    if (!m_searchResults) {
        // 点击播放列表
        if (index != -1) {
            g_Player.playMedia(index);
        }
        return;
    }

    //
    // 点击的是搜索结果，播放选中的结果
    //

    CMPAutoPtr<IMediaLibrary> mediaLib;
    g_Player.getMediaLibrary(&mediaLib);

    if (index < 0) {
        // 未选中，则播放所有的搜索结果
        set<int> ids;
        CMPAutoPtr<IPlaylist> playlist;
        g_Player.newPlaylist(&playlist);

        // 添加 categories 中的结果
        for (auto &category : m_categoriesResults) {
            CMPAutoPtr<IPlaylist> tmp;
            if (mediaLib->getMediaCategory(category, &tmp) == ERR_OK) {
                int count = tmp->getCount();
                for (int i = 0; i < count; i++) {
                    CMPAutoPtr<IMedia> m;
                    if (tmp->getItem(i, &m) == ERR_OK) {
                        auto id = m->getID();
                        if (ids.find(id) == ids.end()) {
                            ids.insert(id);
                            playlist->insertItem(-1, m);
                        }
                    }
                }
            }
        }

        // 添加单独的歌曲
        int count = m_searchResults->getCount();
        for (int i = 0; i < count; i++) {
            CMPAutoPtr<IMedia> m;
            if (m_searchResults->getItem(i, &m) == ERR_OK) {
                auto id = m->getID();
                if (ids.find(id) == ids.end()) {
                    ids.insert(id);
                    playlist->insertItem(-1, m);
                }
            }
        }

        g_Player.setCurrentPlaylist(playlist);
        g_Player.setCurrentMediaInPlaylist(0);
        g_Player.play();
        return;
    }

    if (index < (int)m_categoriesResults.size()) {
        // 选中的结果在 categories 中
        CMPAutoPtr<IPlaylist> playlist;
        if (mediaLib->getMediaCategory(m_categoriesResults[index], &playlist) == ERR_OK) {
            g_Player.setCurrentPlaylist(playlist);
            g_Player.setCurrentMediaInPlaylist(0);
            g_Player.play();
        }
    } else {
        // 选中的结果在剩余的歌曲中，播放所有的歌曲
        index -= (int)m_categoriesResults.size();

        g_Player.setCurrentPlaylist(m_searchResults);
        g_Player.setCurrentMediaInPlaylist(index);
        g_Player.play();
    }
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
    m_nowPlayingRow = -1;

    if (m_searchResults) {
        CMPAutoPtr<IMedia> media;
        if (g_Player.getCurrentMedia(&media) == ERR_OK) {
            auto id = media->getID();
            uint32_t count = m_searchResults->getCount();
            for (uint32_t i = 0; i < count; i++) {
                CMPAutoPtr<IMedia> m;
                if (m_searchResults->getItem(i, &m) == ERR_OK) {
                    if (m->getID() == id) {
                        m_nowPlayingRow = i;
                        break;
                    }
                }
            }
        }
    } else {
        m_nowPlayingRow = g_Player.getCurrentMediaIndex();
    }
}

void CMPlaylistCtrl::doSearch(cstr_t keyword) {
    // clear old results
    if (m_searchResults) {
        m_searchResults.release();
    }
    m_categoriesResults.clear();
    deleteAllItems(false);

    if (isEmptyString(keyword)) {
        updatePlaylist(true);
        return;
    }

    if (g_Player.newPlaylist(&m_searchResults) != ERR_OK) {
        return;
    }

    CSkinListCtrl::VecTextColor vItemClrs;
    VecStrings vKeywords;
    splitKeywords(keyword, vKeywords);

    VecMediaCategories allCategories;

    CMPAutoPtr<IMediaLibrary> mediaLib;
    g_Player.getMediaLibrary(&mediaLib);

    mediaLib->getMediaCategories(allCategories);

    //
    // 搜索分类列表
    //
    for (auto &category : allCategories) {
        string title = g_LangTool.toLocalString(mediaCategoryTypeToString(category.type));
        title += ": " + (category.name.empty() ? _TL("Unkown") : category.name);

        auto matchValue = searchWithKeyword(keyword, title.c_str(), vItemClrs);
        if (matchValue <= 0) {
            matchValue = searchWithKeywordEx(vKeywords, title.c_str(), vItemClrs);
        }
        if (matchValue > 0) {
            auto index = getRowCount();

            m_categoriesResults.push_back(category);

            insertItem(index, itos(index + 1).c_str(), 0, 0, false);

            int count = vItemClrs.sumCount();
            if (count < title.size()) {
                vItemClrs.add((int)title.size() - count, CSkinListView::CN_TEXT);
            }
            string temp = " " + stringPrintf(_TL("(%d Tracks)"), category.mediaCount);
            vItemClrs.add(temp.size(), CSkinListView::CN_CUSTOMIZED_START + 1);
            title += temp;

            setItemTextEx(index, 1, title.c_str(), vItemClrs);

            setItemMediaDuration(index, category.mediaDuration);
        }
    }


    //
    // search playlist with @keyword
    //
    CMPAutoPtr<IPlaylist> playlist(mediaLib->getAll());
    int count = playlist->getCount();
    for (uint32_t i = 0; i < count; i++) {
        CMPAutoPtr<IMedia> media;
        auto ret = playlist->getItem(i, &media);
        if (ret == ERR_OK) {
            string title = CPlayer::formatMediaTitle(media);

            auto matchValue = searchWithKeyword(keyword, title.c_str(), vItemClrs);
            if (matchValue <= 0) {
                matchValue = searchWithKeywordEx(vKeywords, title.c_str(), vItemClrs);
            }
            if (matchValue > 0) {
                appendMedia(media, vItemClrs);
                m_searchResults->insertItem(-1, media);
            }
        }
    }

    invalidate();
}
