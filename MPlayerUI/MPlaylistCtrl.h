#pragma once

class CMPlaylistCtrl : public CSkinListCtrl, public IEventHandler
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinListCtrl)
public:
    CMPlaylistCtrl();
    virtual ~CMPlaylistCtrl();

    void onCreate() override;
    void onEvent(const IEvent *pEvent) override;

    void onKeyDown(uint32_t nChar, uint32_t nFlags) override;

    void sendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol) override;

    void deleteSelectedItems();
    void offsetAllSelectedItems(bool bMoveDown);

    void setNowPlaying();

    enum
    {
        IMAGE_NONE            = 0,
        IMAGE_NOW_PLAYING    = 5,
    };

protected:
    void insertMedia(CMPAutoPtr<IPlaylist> &playlist, int index, bool redraw = false);
    void updateMediaIndex();

    void updatePlaylist(CEventPlaylistChanged *pEventPlaylistChanged = nullptr);

    int getItemImageIndex(int nItem) override;

    int            m_nNowPlaying;

};
