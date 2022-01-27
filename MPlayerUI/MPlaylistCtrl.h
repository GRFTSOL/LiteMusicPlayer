#pragma once

class CMPlaylistCtrl : public CSkinListCtrl, public IEventHandler
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinListCtrl)
public:
    CMPlaylistCtrl();
    virtual ~CMPlaylistCtrl();

    void onCreate();
    void onEvent(const IEvent *pEvent);

    void onKeyDown(uint32_t nChar, uint32_t nFlags);

    void sendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol);

    void deleteSelectedItems();
    void offsetAllSelectedItems(bool bMoveDown);

    void setNowPlaying();

    enum
    {
        IMAGE_NONE            = 0,
        IMAGE_NOW_PLAYING    = 5,
    };

protected:
    void updatePlaylist(CEventPlaylistChanged *pEventPlaylistChanged = nullptr);

    int getItemImageIndex(int nItem);

    int            m_nNowPlaying;

};
