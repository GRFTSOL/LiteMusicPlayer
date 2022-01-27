// MPMediaTree.h: interface for the CMPMediaTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPMEDIATREE_H__D940D441_053A_48DB_B6FD_DF6E2C2E99A9__INCLUDED_)
#define AFX_MPMEDIATREE_H__D940D441_053A_48DB_B6FD_DF6E2C2E99A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinTreeCtrl.h"
#include "Player.h"

enum ImageIndex
{
    II_MEDIA_LIB,
    II_PLAYLISTS,
    II_ARTIST,
    II_ALBUM,
    II_GENRE,
    II_MUSIC,
    II_NOW_PLAYING,
    II_LOCATION,
    II_YEAR,
    II_RATING,
    II_MYCOMPUTER,
    II_FOLDER_OPENED,
    II_FOLDER_CLOSED,
    II_DRIVE_REMOVABLE,
    II_DRIVE_FIXED,
    II_DRIVE_REMOTE,
    II_DRIVE_CDROM,
    II_DRIVE_RAMDISK,
};

// MGNT == MediaGuideNodeType
enum MPMediaGuideNodeType
{
    MGNT_MEDIA_LIB,
    MGNT_DISK_DEVICE,
    MGNT_PLAYLIST_FILE,
    MGNT_NOW_PLAYING,
};

class IMPMediaTreeNode : public ISkinTreeNode
{
public:

    IMPMediaTreeNode()
    {
    }

public:
    uint8_t                    folderType;
    uint8_t                    nodeType;
    CMPAutoPtr<IPlaylist>    m_playlist;

};

// MTN = Media Tree Node
// Media library
class CMPMTNMediaLibrary : public IMPMediaTreeNode
{
public:
    enum FolderType
    {
        FT_NOT_SET,
        FT_MEDIA_LIB,
        FT_NOW_PLAYING,
        FT_ALL_MUSIC,
        FT_ALL_ARTIST,
        FT_LOCATION,
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

        //
        FT_ALL_GENRE,
        FT_GENRE,
        FT_GENRE_UNKNOWN,
        FT_ALL_YEAR,
        FT_YEAR,

        FT_ALL_RATING,
        FT_NOT_RATED,
        FT_RATING_1,
        FT_RATING_2,
        FT_RATING_3,
        FT_RATING_4,
        FT_RATING_5,

        FT_ALL_PLAYLIST_FILES,
        FT_PLAYLIST_FILE,
        FT_MEDIA_FILE,
        FT_TOP_RATING,
        FT_TOP_PLAYED,
        FT_RECENT_PLAYED,
        FT_RECENT_ADDED,
    };

    CMPMTNMediaLibrary()
    {
        nodeType = MGNT_MEDIA_LIB;
        setUpdated(false);
    }

    void addNewNode(cstr_t szName, int fType, int nImageIndex, bool bUpdated)
    {
        CMPMTNMediaLibrary        *pNewNode;

        pNewNode = new CMPMTNMediaLibrary;
        pNewNode->setUpdated(bUpdated);
        pNewNode->folderType = fType;
        pNewNode->m_nImageIndex = nImageIndex;
        pNewNode->m_nExpandedImageIndex = nImageIndex;
        pNewNode->m_strName = szName;
        addChildBack(pNewNode);
    }

    virtual void onUpdate();

};

// Disk device
class CMPMTNDiskDevice : public IMPMediaTreeNode
{
public:
    CMPMTNDiskDevice()
    {
        nodeType = MGNT_DISK_DEVICE;
        setUpdated(false);
    }

    void addNewNode(cstr_t szName, cstr_t szDir, int nImageIndex, int nExpandedImageIndex)
    {
        CMPMTNDiskDevice        *pNewNode;

        pNewNode = new CMPMTNDiskDevice;
        pNewNode->m_nImageIndex = nImageIndex;
        pNewNode->m_nExpandedImageIndex = nExpandedImageIndex;
        pNewNode->m_strName = szName;
        pNewNode->m_strDir = szDir;
        dirStringAddSlash(pNewNode->m_strDir);
        addChildBack(pNewNode);
    }

    virtual void onUpdate();

    string            m_strDir;

};

// play lists
class CMPMTNPlaylists: public IMPMediaTreeNode
{
public:
    CMPMTNPlaylists()
    {
        nodeType = MGNT_PLAYLIST_FILE;
        setUpdated(false);
        folderType = FT_ALL_PLAYLISTS;
    }

    enum
    {
        FT_ALL_PLAYLISTS,
        FT_PLAYLIST_FILE,
    };

    virtual void onUpdate();

    string            m_strPlaylistFile;

};

class CMPMediaTree : public ISkinTree
{
public:
    CMPMediaTree();
    virtual ~CMPMediaTree();

    bool init();

    void close();

    bool getCurNodePlaylist(IPlaylist **playlist);

public:

};

#endif // !defined(AFX_MPMEDIATREE_H__D940D441_053A_48DB_B6FD_DF6E2C2E99A9__INCLUDED_)
