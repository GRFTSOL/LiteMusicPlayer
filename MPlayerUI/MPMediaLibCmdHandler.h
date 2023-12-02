#pragma once

#ifndef MPlayerUI_MPMediaLibCmdHandler_h
#define MPlayerUI_MPMediaLibCmdHandler_h


#include "SkinDroplistCtrl.h"


class CMediaLibTreeProvider {
public:
    enum FolderType {
        FT_MEDIA_LIB,
        FT_NOW_PLAYING,
        FT_ALL_MUSIC,
        FT_ALL_ARTIST,
        FT_ALL_SONGS_BY_ARTIST,
        FT_ARTIST,
        FT_ARTIST_UNKNOWN,

        FT_ALL_ALBUM,
        FT_ALBUM,
        FT_ALBUM_UNKNOWN,
        FT_ALBUM_TOP_RATING,

        FT_ARTIST_ALBUM,
        FT_ARTIST_ALBUM_UNKNOWN,
        FT_ARTIST_ALL_MUSIC,
        FT_ARTIST_TOP_RATING,

        FT_ALL_PLAYLIST_FILES,
        FT_PLAYLIST_FILE,
        FT_MEDIA_FILE,
        FT_TOP_RATING,
        FT_TOP_PLAYED,
        FT_RECENT_PLAYED,
        FT_RECENT_ADDED,
    };

    enum ImageIndex {
        II_MEDIA_LIB,
        II_PLAYLISTS,
        II_ARTIST,
        II_ALBUM,
        II_GENRE,
        II_MUSIC,
        II_NOW_PLAYING,
    };

    struct Item {
        string                      name;
        uint8_t                     nImageIndex;
        bool                        bUptodate;
        FolderType                  folderType;
        PlaylistPtr                 playlist;
        cstr_t getValue() const {
            if (folderType == FT_ARTIST_UNKNOWN
                || folderType == FT_ARTIST_ALBUM_UNKNOWN
                || folderType == FT_ALBUM_UNKNOWN) {
                return "";
            } else {
                return name.c_str();
            }
        }
    };
    typedef vector<Item>        V_ITEMS;

    CMediaLibTreeProvider() { }
    ~CMediaLibTreeProvider() { }

    void init();
    void close();

    void update();

    bool enumChildren(V_ITEMS &vItems);

    PlaylistPtr getChildPlaylist(int nIndex);

    bool chToChild(int nIndex);
    bool chToParent();
    bool chToPath(V_ITEMS &vPath);
    bool chToPath(cstr_t szPath);

    bool getPath(V_ITEMS &vPath);

    Item getChildData(int n);

    bool isCurNodePlaylist();
    PlaylistPtr getCurNodePlaylist();

    bool isCurNodePlaylistFile();
    bool getCurNodePlaylistFile(int nChildPos, string &strPlaylistFile);

    void eraseChild(int nIndex) { m_tree.eraseChild(nIndex); }

protected:
    void addPlaylistToTree(const PlaylistPtr &playlist);

protected:
    CMediaLibrary               *m_mediaLib;
    CTree<Item>                 m_tree;

};

class CMPMediaLibCmdHandler : public ISkinCmdHandler {
public:
    CMPMediaLibCmdHandler();
    virtual ~CMPMediaLibCmdHandler();

    virtual void init(CSkinWnd *pSkinWnd);

    // if the command id is processed, return true.
    virtual bool onCommand(uint32_t nId);
    virtual bool onUIObjNotify(IUIObjNotify *pNotify);

protected:
    void onDblClickMediaList();

    void reloadMedialibView();
    void updateMediaList();

    void addHistoryPath();
    void backHistoryPath();

protected:
    struct HistroyItem {
        CMediaLibTreeProvider::V_ITEMS  path;
        int                             nSelChild;
    };

    CMediaLibTreeProvider       m_mediaLibTree;
    CMediaLibTreeProvider::V_ITEMS  m_vPathLatest;
    list<HistroyItem>               m_historyPath;


};

#endif // !defined(MPlayerUI_MPMediaLibCmdHandler_h)
