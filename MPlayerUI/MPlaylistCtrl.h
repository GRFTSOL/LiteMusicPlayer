#pragma once

class CMPlaylistCtrl : public CSkinListCtrl, public IEventHandler, public IEditNotification {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinListCtrl)
public:
    CMPlaylistCtrl();
    virtual ~CMPlaylistCtrl();

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    void onCreate() override;
    void onTimer(int nId) override;
    void onEvent(const IEvent *pEvent) override;

    bool onCommand(uint32_t nId) override;

    bool onKeyDown(uint32_t nChar, uint32_t nFlags) override;
    bool onHandleKeyDown(uint32_t nChar, uint32_t nFlags) override;

    bool onRButtonUp(uint32_t nFlags, CPoint point) override;
    void onLanguageChanged() override;

    void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override;

    void sendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol) override;

    void makeSureRowVisible(int nRow) override;

    void deleteSelectedItems();
    void offsetAllSelectedItems(bool bMoveDown);

    void setNowPlaying();

    enum {
        IMAGE_NONE                  = 0,
        IMAGE_NOW_PLAYING           = 5,
    };

    // Keyword Search 的回调消息
    virtual void onEditorTextChanged() override;
    virtual bool onEditorKeyDown(uint32_t code, uint32_t flags) override;
    virtual void onEditorMouseWheel(int wheelDistance, int mkeys, CPoint pt) override;
    virtual void onEditorKillFocus() override;

protected:
    void insertMedia(const PlaylistPtr &playlist, int index, bool redraw = false);
    void appendMedia(const MediaPtr &media, const CSkinListCtrl::VecTextColor &vItemClrs);
    void updateMediaIndex();

    void onCurrentPlaylistEvent(CEventPlaylistChanged *pEventPlaylistChanged = nullptr);

    void setItemMediaDuration(int index, int ms);

    void updatePlaylist(bool isRedraw);
    void doSearch(cstr_t keyword);
    void showHideEditorSearch(int row);

    PlaylistPtr getSelected();
    PlaylistPtr getSelectedSearchResult(MediaPtr &firstSelectedOut);
    PlaylistPtr getAllSearchResult(MediaPtr &firstSelectedOut);

    void loadMenu();
    void popupSearchResultMenu(CPoint pt);
    void popupContexMenu(CPoint pt);

protected:
    // Context menu
    string                      m_menuCtxName;
    SkinMenuPtr                 m_menuCtx;
    CMenu                       m_ctxSubmenuAddToPlaylist;

    // Search result menu.
    string                      m_menuSearchResultName;
    SkinMenuPtr                 m_menuSearchResult;
    CMenu                       m_submenuAddResultToPlaylist;
    CMenu                       m_submenuAddSelectedToPlaylist;

    // Popup menu playlist names.
    VecPlaylistBriefs            m_playlistNames;

    string                      m_idEditorSearch;
    CSkinEditCtrl               *m_editorSearch;
    int                         m_timerIdBeginSearch;

    PlaylistPtr                 m_searchResults;
    VecMediaCategories          m_categoriesResults;

};
