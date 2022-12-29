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

    void onKeyDown(uint32_t nChar, uint32_t nFlags) override;

    void sendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol) override;

    void deleteSelectedItems();
    void offsetAllSelectedItems(bool bMoveDown);

    void setNowPlaying();

    enum {
        IMAGE_NONE                  = 0,
        IMAGE_NOW_PLAYING           = 5,
    };

    // Keyword Search 的回调消息
    virtual void onTextChanged() override;
    virtual void onSpecialKey(SpecialKey key) override;

protected:
    void insertMedia(CMPAutoPtr<IPlaylist> &playlist, int index, bool redraw = false);
    void appendMedia(CMPAutoPtr<IMedia> &media, const CSkinListCtrl::VecTextColor &vItemClrs);
    void updateMediaIndex();

    void onCurrentPlaylistEvent(CEventPlaylistChanged *pEventPlaylistChanged = nullptr);

    int getItemImageIndex(int nItem) override;

    void updatePlaylist(bool isRedraw);
    void doSearch(cstr_t keyword);
    void useResultAsNowPlaying();

protected:
    int                         m_nNowPlaying;

    string                      m_idEditorSearch;
    CSkinEditCtrl               *m_editorSearch;
    int                         m_timerIdBeginSearch;

    CMPAutoPtr<IPlaylist>       m_searchResults;

};
