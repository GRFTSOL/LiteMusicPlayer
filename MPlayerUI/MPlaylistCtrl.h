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

    bool onCommand(int nId) override;

    bool onKeyDown(uint32_t nChar, uint32_t nFlags) override;
    bool onHandleKeyDown(uint32_t nChar, uint32_t nFlags) override;

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

    void setItemMediaDuration(int index, int seconds);

    void playItem(int index);

    void updatePlaylist(bool isRedraw);
    void doSearch(cstr_t keyword);
    void showHideEditorSearch(int row);

protected:
    string                      m_idEditorSearch;
    CSkinEditCtrl               *m_editorSearch;
    int                         m_timerIdBeginSearch;

    PlaylistPtr                 m_searchResults;
    VecMediaCategories          m_categoriesResults;

};
