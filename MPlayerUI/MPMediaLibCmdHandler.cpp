// MPMediaLibCmdHandler.cpp: implementation of the CMPMediaLibCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerAppBase.h"
#include "MPMediaLibCmdHandler.h"
#include "MPHelper.h"

void CMediaLibTreeProvider::init()
{
    Item        item;

    item.bUptodate = false;
    item.folderType = FT_MEDIA_LIB;
    item.name = "Media Library";
    item.nImageIndex = II_MEDIA_LIB;
    m_tree.addChild(item);

    m_tree.chToChild(0);

    item.bUptodate = false;
    item.folderType = FT_ALL_SONGS_BY_ARTIST;
    item.name = "All Songs By Artist";
    item.nImageIndex = II_MUSIC;
    m_tree.addChild(item);

    item.folderType = FT_ALL_ARTIST;
    item.name = "Artist";
    item.nImageIndex = II_ARTIST;
    m_tree.addChild(item);

    item.folderType = FT_ALL_ALBUM;
    item.name = "Album";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_ALL_PLAYLIST_FILES;
    item.name = "Playlists";
    item.nImageIndex = II_PLAYLISTS;
    m_tree.addChild(item);

    item.folderType = FT_TOP_PLAYED;
    item.name = "Top 50 played";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_TOP_RATING;
    item.name = "Top 50 rating";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_RECENT_PLAYED;
    item.name = "Recent played";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_RECENT_ADDED;
    item.name = "Recent Added";
    item.nImageIndex = II_ALBUM;
    m_tree.addChild(item);

    item.folderType = FT_NOW_PLAYING;
    item.name = "Now Playing";
    item.nImageIndex = II_NOW_PLAYING;
    m_tree.addChild(item);

    g_Player.getMediaLibrary(&m_mediaLib);
}

void CMediaLibTreeProvider::close()
{
    m_mediaLib.release();
    m_tree.clear();
}

void CMediaLibTreeProvider::update()
{

}

bool CMediaLibTreeProvider::enumChildren(V_ITEMS &vItems)
{
    for (int i = 0; i < m_tree.getChildrenCount(); i++)
    {
        vItems.push_back(m_tree.getChild(i));
    }

    return true;
}

bool CMediaLibTreeProvider::chToChild(int nIndex)
{
    if (!m_tree.chToChild(nIndex))
        return false;

    Item    &item = m_tree.getCurNodeData();

    if (item.bUptodate)
        return true;

    CMPAutoPtr<IPlaylist>    playlist;
    MLRESULT                nRet = ERR_OK;

    item.bUptodate = true;

    if (item.folderType == FT_ALL_PLAYLIST_FILES)
    {
        // nRet = m_mediaLib->getAll(&playlist);
        vector<string>    vFiles;
        Item            newitem;

        newitem.bUptodate = false;
        newitem.folderType = FT_PLAYLIST_FILE;
        newitem.nImageIndex = II_PLAYLISTS;

        // enumPlaylists("\\", nLevel, vFiles);
        enumPlaylistsFast(vFiles);
        for (int i = 0; i < (int)vFiles.size(); i++)
        {
            newitem.name = vFiles[i];
            m_tree.addChild(newitem);
        }
    }
    else if (item.folderType == FT_PLAYLIST_FILE)
    {
        g_Player.clearPlaylist();
        g_Player.addToPlaylist(item.name.c_str());
        g_Player.saveCurrentPlaylist();
    }
    else if (item.folderType == FT_ALL_SONGS_BY_ARTIST)
    {
        nRet = m_mediaLib->getAll(&playlist, MLOB_ARTIST, -1);
    }
    else if (item.folderType == FT_ALL_ARTIST)
    {
        CMPAutoPtr<IVXStr>    pvStr;
        MLRESULT            nRet;
        Item                newitem;

        newitem.bUptodate = false;
        newitem.folderType = FT_ARTIST;
        newitem.nImageIndex = II_ARTIST;

        nRet = m_mediaLib->getAllArtist(&pvStr);
        if (nRet == ERR_OK)
        {
            int        nCount = pvStr->size();
            for (int i = 0; i < nCount; i++)
            {
                newitem.name = pvStr->at(i);
                if (newitem.name.empty())
                {
                    newitem.name = "Unknown artist";
                    newitem.folderType = FT_ARTIST_UNKNOWN;
                    m_tree.addChild(newitem);
                    newitem.folderType = FT_ARTIST;
                    continue;
                }
                m_tree.addChild(newitem);
            }
        }
    }
    else if (item.folderType == FT_ALL_ALBUM)
    {
        CMPAutoPtr<IVXStr>    pvStr;
        MLRESULT            nRet;
        Item                newitem;

        newitem.bUptodate = false;
        newitem.folderType = FT_ALBUM;
        newitem.nImageIndex = II_ALBUM;

        nRet = m_mediaLib->getAllAlbum(&pvStr);
        if (nRet == ERR_OK)
        {
            int        nCount = pvStr->size();
            for (int i = 0; i < nCount; i++)
            {
                newitem.name = pvStr->at(i);
                if (newitem.name.empty())
                {
                    newitem.name = "Unknown Album";
                    newitem.folderType = FT_ALBUM_UNKNOWN;
                    m_tree.addChild(newitem);
                    newitem.folderType = FT_ALBUM;
                    continue;
                }
                m_tree.addChild(newitem);
            }
        }
    }
    else if ((item.folderType == FT_ARTIST || item.folderType == FT_ARTIST_UNKNOWN))
    {
        CMPAutoPtr<IVXStr>    pvStr;
        MLRESULT            nRet;
        Item                newitem;

        newitem.bUptodate = false;
        newitem.folderType = FT_ARTIST_ALL_MUSIC;
        newitem.nImageIndex = II_MUSIC;
        newitem.name = "All songs by artist";
        m_tree.addChild(newitem);

        newitem.bUptodate = false;
        newitem.folderType = FT_ARTIST_TOP_RATING;
        newitem.nImageIndex = II_MUSIC;
        newitem.name = "top rated songs by artist";
        m_tree.addChild(newitem);

        newitem.bUptodate = false;
        newitem.nImageIndex = II_ALBUM;
        newitem.folderType = FT_ARTIST_ALBUM;

        nRet = m_mediaLib->getAlbumOfArtist(item.getValue(), &pvStr);
        if (nRet == ERR_OK)
        {
            int        nCount = pvStr->size();
            for (int i = 0; i < nCount; i++)
            {
                newitem.name = pvStr->at(i);
                if (newitem.name.empty())
                {
                    newitem.name = "Unknown album";
                    newitem.folderType = FT_ARTIST_ALBUM_UNKNOWN;
                    m_tree.addChild(newitem);
                    newitem.folderType = FT_ARTIST_ALBUM;
                    continue;
                }
                m_tree.addChild(newitem);
            }
        }
        item.bUptodate = true;
    }
    else if (item.folderType == FT_ARTIST_ALBUM || 
        item.folderType == FT_ARTIST_ALBUM_UNKNOWN || 
        item.folderType == FT_ARTIST_TOP_RATING || 
        item.folderType == FT_ARTIST_ALL_MUSIC)
    {
        Item                &itemArtist = m_tree.getParentNodeData();

        if (item.folderType == FT_ARTIST_ALL_MUSIC)
            nRet = m_mediaLib->getByArtist(itemArtist.getValue(), &playlist, MLOB_NONE, -1);
        else if (item.folderType == FT_ARTIST_TOP_RATING)
        {
            nRet = m_mediaLib->getByArtist(itemArtist.getValue(), &playlist, MLOB_RATING, -1);
            if (nRet == ERR_OK && playlist)
                g_Player.filterLowRatingMedia(playlist);
        }
        else
            nRet = m_mediaLib->getByAlbum(itemArtist.getValue(), item.getValue(), &playlist, MLOB_NONE, -1);
    }
    else if (item.folderType == FT_ALBUM_UNKNOWN || 
        item.folderType == FT_ALBUM)
    {
        nRet = m_mediaLib->getByAlbum(item.getValue(), &playlist, MLOB_NONE, -1);
    }
    else if (item.folderType == FT_TOP_RATING)
    {
        nRet = m_mediaLib->getTopRating(16, &playlist);
        if (nRet == ERR_OK && playlist)
            g_Player.filterLowRatingMedia(playlist);
    }
    else if (item.folderType == FT_TOP_PLAYED)
    {
        nRet = m_mediaLib->getTopPlayed(16, &playlist);
    }
    else if (item.folderType == FT_RECENT_ADDED)
    {
        nRet = m_mediaLib->getRecentAdded(16, &playlist);
    }
    else if (item.folderType == FT_RECENT_PLAYED)
    {
        nRet = m_mediaLib->getRecentPlayed(16, &playlist);
    }

    if (nRet == ERR_OK && playlist.p)
    {
        addPlaylistToTree(playlist);
        item.playlist = playlist;
    }

    return true;
}

bool CMediaLibTreeProvider::chToParent()
{
    return m_tree.chToParent();
}

bool CMediaLibTreeProvider::chToPath(V_ITEMS &vPath)
{
    m_tree.chToRoot();

    for (int i = 0; i < (int)vPath.size(); i++)
    {
        int        nCount = m_tree.getChildrenCount();
        int        k;
        for (k = 0; k < nCount; k++)
        {
            if (strcmp(m_tree.getChild(k).name.c_str(), vPath[i].name.c_str()) == 0)
            {
                chToChild(k);
                break;
            }
        }
        if (k == nCount)
            return false;
    }

    return true;
}
// 
// bool CMediaLibTreeProvider::chToPath(int nIndex)
// {
//     return m_tree.chToChild(nIndex);
// }

bool CMediaLibTreeProvider::getPath(V_ITEMS &vPath)
{
    m_tree.getPath(vPath);

    for (int i = 0; i < (int)vPath.size(); i++)
        vPath[i].playlist.release();

    return true;
}

CMediaLibTreeProvider::Item CMediaLibTreeProvider::getChildData(int n)
{
    return m_tree.getChild(n);
}

bool CMediaLibTreeProvider::isCurNodePlaylist()
{
    return m_tree.getCurNodeData().playlist.p != nullptr;
}

bool CMediaLibTreeProvider::getCurNodePlaylist(IPlaylist **playlist)
{
    Item    &item = m_tree.getCurNodeData();
    if (item.playlist.p)
    {
        *playlist = item.playlist;
        (*playlist)->addRef();

        return true;
    }

    return false;
}

bool CMediaLibTreeProvider::isCurNodePlaylistFile()
{
    Item    &item = m_tree.getCurNodeData();
    if (item.folderType == FT_ALL_PLAYLIST_FILES)
        return true;

    return false;
}

bool CMediaLibTreeProvider::getCurNodePlaylistFile(int nChildPos, string &strPlaylistFile)
{
    if (nChildPos >= 0 && nChildPos < m_tree.getChildrenCount())
        strPlaylistFile = m_tree.getChild(nChildPos).name;
    else
        strPlaylistFile.resize(0);

    return true;
}

void CMediaLibTreeProvider::addPlaylistToTree(IPlaylist *playlist)
{
    Item                newitem;

    newitem.bUptodate = true;
    newitem.folderType = FT_MEDIA_FILE;
    newitem.nImageIndex = II_MUSIC;

    int        nCount = playlist->getCount();
    for (int i = 0; i < nCount; i++)
    {
        CMPAutoPtr<IMedia>    pMedia;
        if (playlist->getItem(i, &pMedia) == ERR_OK)
        {
            newitem.name = g_Player.formatMediaTitle(pMedia);
            m_tree.addChild(newitem);
        }
    }
}


CMPMediaLibCmdHandler::CMPMediaLibCmdHandler()
{

}

CMPMediaLibCmdHandler::~CMPMediaLibCmdHandler()
{

}

void CMPMediaLibCmdHandler::init(CSkinWnd *pSkinWnd)
{
    ISkinCmdHandler::init(pSkinWnd);
    
    m_mediaLibTree.init();

    if (m_vPathLatest.size())
    {
        m_mediaLibTree.chToPath(m_vPathLatest);
    }

    reloadMedialibView();
    updateMediaList();
}

// if the command id is processed, return true.
bool CMPMediaLibCmdHandler::onCommand(int nId)
{
    switch (nId)
    {
    case IDC_PLAY:
        onDblClickMediaList();
        break;
    case IDOK:
        m_pSkinWnd->destroy();
        break;
    case IDC_QUEUE_UP:
        {
            CSkinListCtrl    *pMediaList =  (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
            int        nSel = pMediaList->getNextSelectedItem(-1);
            if (nSel == -1)
                break;

            CMPAutoPtr<IPlaylist>    playlist;
            if (m_mediaLibTree.getCurNodePlaylist(&playlist))
            {
                CMPAutoPtr<IMedia>    media;
                if (playlist->getItem(nSel, &media) == ERR_OK)
                {
                    CMPAutoPtr<IPlaylist>    curPl;
                    if (g_Player.getCurrentPlaylist(&curPl) == ERR_OK)
                    {
                        curPl->insertItem(-1, media);
                    }
                }
            }
        }
        break;
    case IDC_NOWPLAYING:
        {
            SkinWndStartupInfo    startInfo("MPSkin", "Playlist", "Playlist.xml", nullptr);
            CMPlayerAppBase::getMPSkinFactory()->activeOrCreateSkinWnd(startInfo);
        }
        break;
    case IDC_ADD_DIR_TO_ML:
        //
        // add Media to library
        //
        if (onCmdSongAddDirToMediaLib(m_pSkinWnd))
            reloadMedialibView();
        break;
    case IDC_REMOVE_FROM_ML:
        {
            CSkinListCtrl    *pMediaList =  (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
            if (pMediaList)
            {
                int        nSel = pMediaList->getNextSelectedItem(-1);
                if (nSel == -1)
                    return true;

                CMPAutoPtr<IPlaylist>        playlist;

                if (m_mediaLibTree.getCurNodePlaylist(&playlist))
                {
                    vector<int>                vIndex;
                    CMPAutoPtr<IMedia>        media;
                    while (nSel != -1)
                    {
                        vIndex.push_back(nSel);
                        nSel = pMediaList->getNextSelectedItem(nSel);
                    }
                    for (int i = vIndex.size() - 1; i >= 0; i--)
                    {
                        nSel = vIndex[i];
                        if (playlist->getItem(nSel, &media) == ERR_OK)
                        {
                            CMPAutoPtr<IMediaLibrary>    mediaLib;
                            if (g_Player.getMediaLibrary(&mediaLib) == ERR_OK)
                                mediaLib->remove(&(media.p), false);
                            playlist->removeItem(nSel);
                            pMediaList->deleteItem(nSel, true);
                            m_mediaLibTree.eraseChild(nSel);
                        }
                    }
                    reloadMedialibView();
                }
            }
        }
        break;
    default:
        return false;
    }

    return true;
}

bool CMPMediaLibCmdHandler::onCustomCommand(int nID)
{
    switch (nID)
    {
    case CMD_ML_BACK:
        {
            backHistoryPath();
        }
        break;
    default:
        return false;
    }

    return true;
}

bool CMPMediaLibCmdHandler::onUIObjNotify(IUIObjNotify *pNotify)
{
    if (pNotify->nID == ID_ML_MEDIA_LIST)
    {
        if (pNotify->pUIObject->isKindOf(CSkinListCtrl::className()))
        {
            CSkinListCtrl    *pMediaList =  (CSkinListCtrl*)pNotify->pUIObject;
            int        nSel = pMediaList->getNextSelectedItem(-1);
            if (nSel == -1)
                return true;

            CSkinListCtrlEventNotify    *pListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;
            if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_DBL_CLICK ||
                pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_ENTER ||
                pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_CLICK)
            {
                if (m_mediaLibTree.isCurNodePlaylist())
                {
                    if (pListCtrlNotify->cmd != CSkinListCtrlEventNotify::C_CLICK)
                    {
                        // play current music...
                        CMPAutoPtr<IPlaylist>    playlist;
                        if (m_mediaLibTree.getCurNodePlaylist(&playlist))
                        {
                            g_Player.setCurrentPlaylist(playlist);
                            g_Player.setCurrentMediaInPlaylist(nSel);
                            g_Player.play();
                        }
                    }
                }
                else if (m_mediaLibTree.isCurNodePlaylistFile())
                {
                    if (pListCtrlNotify->cmd != CSkinListCtrlEventNotify::C_CLICK)
                    {
                        // play selected playlist
                        string        strFile;
                        if (m_mediaLibTree.getCurNodePlaylistFile(nSel, strFile))
                        {
                            g_Player.clearPlaylist();
                            g_Player.addToPlaylist(strFile.c_str());
                            g_Player.saveCurrentPlaylist();
                            g_Player.play();
                        }
                    }
                }
                else
                {
                    CMediaLibTreeProvider::Item    item;

                    item = m_mediaLibTree.getChildData(nSel);
                    if (item.folderType == CMediaLibTreeProvider::FT_NOW_PLAYING)
                    {
                        SkinWndStartupInfo    startInfo("MPSkin", "Playlist", "Playlist.xml", nullptr);
                        CMPlayerAppBase::getMPSkinFactory()->activeOrCreateSkinWnd(startInfo);
                    }
                    else
                    {
                        addHistoryPath();
                        m_mediaLibTree.chToChild(nSel);
                        updateMediaList();
                    }
                }

//                 if (bClose)
//                 {
//                     m_pSkinWnd->destroy();
//                 }
            }
        }

        return true;
    }
/*    else if (pNotify->nID == CMD_ML_GUIDE)
    {
        if (pNotify->pUIObject->isKindOf(CSkinDroplistCtrl::className()))
        {
            //
            // Change to new path.
            //
            CSkinDroplistCtrl        *pCtrl = (CSkinDroplistCtrl*)pNotify->pUIObject;
            CSkinDroplistCtrlEventNotify    *pDroplistEvent = (CSkinDroplistCtrlEventNotify*)pNotify;
            if (pDroplistEvent->cmd == CSkinDroplistCtrlEventNotify::C_SEL_CHANGED)
            {
                vector<string>        vPath;
                CMediaLibTreeProvider::V_ITEMS    vPathNew;
                pCtrl->getSelPath(vPath);
                m_mediaLibTree.getPath(vPathNew);
                assert(vPathNew.size() >= vPath.size());
                if (vPathNew.size() > vPath.size())
                {
                    vPathNew.resize(vPath.size());
                    addHistoryPath();
                    m_mediaLibTree.chToPath(vPathNew);
                    updateMediaList();
                }
            }
        }
        return true;
    }*/

    return false;
}

void CMPMediaLibCmdHandler::onDblClickMediaList()
{
    CSkinListCtrl    *pMediaList = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
    if (!pMediaList)
        return;

    CSkinListCtrlEventNotify    notify(pMediaList, -1, -1);
    notify.cmd = CSkinListCtrlEventNotify::C_DBL_CLICK;

    onUIObjNotify(&notify);
}

void CMPMediaLibCmdHandler::reloadMedialibView()
{
    m_mediaLibTree.close();
    m_mediaLibTree.init();
    m_mediaLibTree.getPath(m_vPathLatest);
    if (m_vPathLatest.size())
    {
        m_mediaLibTree.chToPath(m_vPathLatest);
        updateMediaList();
    }

}

void CMPMediaLibCmdHandler::updateMediaList()
{
    CSkinListCtrl    *plistCtrl;
    plistCtrl = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
    if (plistCtrl)
    {
        CMediaLibTreeProvider::V_ITEMS    vItems;

        m_mediaLibTree.enumChildren(vItems);

        if (plistCtrl->getColumnCount() == 0)
            plistCtrl->addColumn("", 230);

        plistCtrl->deleteAllItems(false);

        for (int i = 0; i < (int)vItems.size(); i++)
        {
            CMediaLibTreeProvider::Item    &item = vItems[i];
            if (item.folderType == CMediaLibTreeProvider::FT_PLAYLIST_FILE)
            {
                plistCtrl->insertItem(plistCtrl->getItemCount(), 
                    fileGetTitle(item.name.c_str()).c_str(), item.nImageIndex);
            }
            else
                plistCtrl->insertItem(plistCtrl->getItemCount(), 
                    item.name.c_str(), item.nImageIndex);
        }

        if (vItems.size())
            plistCtrl->setItemSelectionState(0, true);
        plistCtrl->invalidate();
    }

/*    CSkinDroplistCtrl    *pDroplist;
    pDroplist = (CSkinDroplistCtrl*)m_pSkinWnd->getUIObjectById(CMD_ML_GUIDE, CSkinDroplistCtrl::className());
    if (pDroplist)
    {
        CMediaLibTreeProvider::V_ITEMS    vItems;

        m_mediaLibTree.getPath(vItems);

        pDroplist->clear();

        for (int i = 0; i < (int)vItems.size(); i++)
        {
            pDroplist->append(vItems[i].nImageIndex, vItems[i].name.c_str());
        }

        pDroplist->invalidate();
    }*/
}

void CMPMediaLibCmdHandler::addHistoryPath()
{
    HistroyItem        item;
    CSkinListCtrl    *plistCtrl;
    plistCtrl = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
    if (plistCtrl)
        item.nSelChild = plistCtrl->getNextSelectedItem();
    else
        item.nSelChild = 0;

    m_mediaLibTree.getPath(item.path);

    m_historyPath.push_back(item);
    if (m_historyPath.size() > 10)
        m_historyPath.erase(m_historyPath.begin());
}

void CMPMediaLibCmdHandler::backHistoryPath()
{
    if (m_historyPath.size())
    {
        CSkinListCtrl    *plistCtrl;
        int                nSelRow = m_historyPath.back().nSelChild;

        m_mediaLibTree.chToPath(m_historyPath.back().path);
        updateMediaList();
        m_historyPath.pop_back();

        plistCtrl = (CSkinListCtrl*)m_pSkinWnd->getUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::className());
        if (plistCtrl && nSelRow >= 0 && nSelRow < plistCtrl->getItemCount())
        {
            plistCtrl->setItemSelectionState(0, false);
            plistCtrl->setItemSelectionState(nSelRow, true);
            plistCtrl->makeSureRowVisible(nSelRow);
            plistCtrl->invalidate();
        }
    }
}
