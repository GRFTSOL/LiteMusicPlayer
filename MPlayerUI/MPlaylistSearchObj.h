#if !defined(_MPLAYLISTSEARCHOBJ_H_)
#define _MPLAYLISTSEARCHOBJ_H_


class CMPlaylistSearchObj : public CSkinDataObj, public IEditNotification, public IUIObjNotifyHandler
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinDataObj)
public:
    CMPlaylistSearchObj();
    virtual ~CMPlaylistSearchObj();

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void onCreate();

    void onSize(int cx, int cy) { }

    void onTimer(int nId);

public:
    virtual void onTextChanged();

    // if this key is processed, return true.
    virtual void onSpecialKey(SpecialKey key);

    virtual void onUIObjNotify(IUIObjNotify *pNotify);

protected:
    void doSearch(cstr_t szText);

    void updatePlaylist();

    void addInRecentPlaylist(cstr_t szName, cstr_t szFile, int nPos);

    void useResultAsNowPlaying();

protected:
    enum
    {
        TD_BEGIN_SEARCH        = 1000,
    };

    enum PLAYLIST_NAME
    {
        PN_NOWPLAYING,
        PN_ALL_MEDIALIB,
        PN_CUSTOMIZED,
    };

    string            m_strIDEditor;
    string            m_strIDPlaylist;
    string            m_strIDPlaylistList;

    CSkinEditCtrl    *m_pCtrlEdit;
    CSkinListCtrlEx    *m_pCtrlPlaylist;
    CSkinListCtrl    *m_pCtrlPlaylistList;

    CMPAutoPtr<IPlaylist>    m_Playlist;
    CMPAutoPtr<IPlaylist>    m_PlaylistResults;

    int                m_nIDBeginSearchTimer;

    VecStrings            m_vRecentPL;

};

#endif // !defined(_MPLAYLISTSEARCHOBJ_H_)
