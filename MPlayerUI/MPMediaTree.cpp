#include "../Skin/Skin.h"
#include "MPMediaTree.h"
#include "MPHelper.h"


class CSortVInt : public vector<int> {
public:
    void add(int n) {
        iterator it, itEnd;
        itEnd = end();
        it = begin();
        for (; it != itEnd; ++it) {
            if (n > *it) {
                continue;
            } else if (n == *it) {
                return;
            } else {
                insert(it, n);
                return;
            }
        }
        push_back(n);
    }

};

void CMPMTNMediaLibrary::onUpdate() {
    CMPAutoPtr<IMediaLibrary> mediaLib;

    if (g_Player.getMediaLibrary(&mediaLib) != ERR_OK) {
        return;
    }

    m_playlist.release();

    if (folderType == FT_ALL_SONGS_BY_ARTIST) {
        mediaLib->getAll(&m_playlist, MLOB_ARTIST, -1);
    } else if (folderType == FT_ALL_ARTIST) {
        CMPAutoPtr<IVString> pvStr;
        MLRESULT nRet;

        nRet = mediaLib->getAllArtist(&pvStr);
        if (nRet == ERR_OK) {
            int nCount = (int)pvStr->size();
            for (int i = 0; i < nCount; i++) {
                if (isEmptyString(pvStr->at(i))) {
                    addNewNode("Unknown artist", FT_ARTIST_UNKNOWN, II_ARTIST, false);
                } else {
                    addNewNode(pvStr->at(i), FT_ARTIST, II_ARTIST, false);
                }
            }
        }
    } else if (folderType == FT_ALL_ALBUM) {
        CMPAutoPtr<IVString> pvStr;
        MLRESULT nRet;

        nRet = mediaLib->getAllAlbum(&pvStr);
        if (nRet == ERR_OK) {
            int nCount = (int)pvStr->size();
            for (int i = 0; i < nCount; i++) {
                if (isEmptyString(pvStr->at(i))) {
                    addNewNode("Unknown Album", FT_ALBUM_UNKNOWN, II_ALBUM, false);
                } else {
                    addNewNode(pvStr->at(i), FT_ALBUM, II_ALBUM, false);
                }
            }
        }
    } else if (folderType == FT_ARTIST ||
        folderType == FT_ARTIST_UNKNOWN) {
        CMPAutoPtr<IVString> pvStr;

        addNewNode("Top rated songs by artist", FT_ARTIST_TOP_RATING, II_MUSIC, false);

        if (folderType == FT_ARTIST_UNKNOWN) {
            mediaLib->getByArtist("", &m_playlist, MLOB_NONE, -1);
            mediaLib->getAlbumOfArtist("", &pvStr);
        } else {
            mediaLib->getByArtist(m_strName.c_str(), &m_playlist, MLOB_NONE, -1);
            mediaLib->getAlbumOfArtist(m_strName.c_str(), &pvStr);
        }

        int nCount = (int)pvStr->size();
        for (int i = 0; i < nCount; i++) {
            if (isEmptyString(pvStr->at(i))) {
                addNewNode("Unknown album", FT_ARTIST_ALBUM_UNKNOWN, II_ALBUM, false);
            } else {
                addNewNode(pvStr->at(i), FT_ARTIST_ALBUM, II_ALBUM, false);
            }
        }
    } else if (folderType == FT_ARTIST_ALBUM ||
        folderType == FT_ARTIST_ALBUM_UNKNOWN ||
        folderType == FT_ARTIST_TOP_RATING ||
        folderType == FT_ARTIST_ALL_MUSIC) {
        if (!m_pParent) {
            return;
        }

        string strArtist;
        CMPMTNMediaLibrary *pNodeArtist = (CMPMTNMediaLibrary *)m_pParent;
        if (pNodeArtist->folderType != FT_ARTIST_UNKNOWN) {
            strArtist = pNodeArtist->m_strName;
        }

        if (folderType == FT_ARTIST_ALL_MUSIC) {
            mediaLib->getByArtist(strArtist.c_str(), &m_playlist, MLOB_NONE, -1);
        } else if (folderType == FT_ARTIST_TOP_RATING) {
            MLRESULT nRet = mediaLib->getByArtist(strArtist.c_str(), &m_playlist, MLOB_RATING, -1);
            if (nRet == ERR_OK && m_playlist) {
                g_Player.filterLowRatingMedia(m_playlist);
            }
        } else {
            mediaLib->getByAlbum(strArtist.c_str(),
                folderType == FT_ARTIST_ALBUM_UNKNOWN ? "" : m_strName.c_str(),
                &m_playlist, MLOB_NONE, -1);
        }
    } else if (folderType == FT_ALBUM_UNKNOWN ||
        folderType == FT_ALBUM) {
        mediaLib->getByAlbum(
            folderType == FT_ALBUM_UNKNOWN ? "" : m_strName.c_str(), &m_playlist, MLOB_NONE, -1);
    }
    //////////////////////////////////////////////////////////////////////////
    // Genre
    else if (folderType == FT_ALL_GENRE) {
        CMPAutoPtr<IVString> pvStr;
        MLRESULT nRet;

        nRet = mediaLib->getAllGenre(&pvStr);
        if (nRet == ERR_OK) {
            int nCount = (int)pvStr->size();
            for (int i = 0; i < nCount; i++) {
                if (isEmptyString(pvStr->at(i))) {
                    addNewNode("Unknown Genre", FT_GENRE_UNKNOWN, II_GENRE, false);
                } else {
                    addNewNode(pvStr->at(i), FT_GENRE, II_GENRE, false);
                }
            }
        }
    } else if (folderType == FT_GENRE_UNKNOWN ||
        folderType == FT_GENRE) {
        mediaLib->getByGenre(
            folderType == FT_GENRE_UNKNOWN ? "" : m_strName.c_str(), &m_playlist, MLOB_NONE, -1);
    }
    // Year
    else if (folderType == FT_ALL_YEAR) {
        // 对year 进行分类，每10年为一个子节点
        CMPAutoPtr<IVInt> pvInt;
        MLRESULT nRet;
        CSortVInt vTenYear;
        int i;
        char szName[256];

        nRet = mediaLib->getAllYear(&pvInt);
        if (nRet == ERR_OK) {
            int nCount = (int)pvInt->size();
            for (i = 0; i < nCount; i++) {
                vTenYear.add(pvInt->at(i) / 10);
                //                 if (pvInt->at(i) == 0)
                //                     addNewNode("Unknown Genre", FT_GENRE_UNKNOWN, II_ALBUM, false);
                //                 else
                //                     addNewNode(pvStr->at(i), FT_GENRE, II_ALBUM, false);
            }
        }

        for (i = 0; i < (int)vTenYear.size(); i++) {
            CMPMTNMediaLibrary *pNewNode;
            int nDecadeYear = vTenYear[i];

            if (nDecadeYear == 0) {
                addNewNode("Unknown", FT_YEAR, II_YEAR, false);
                continue;
            }

            pNewNode = new CMPMTNMediaLibrary;
            pNewNode->setUpdated(true);
            pNewNode->folderType = FT_NOT_SET;
            pNewNode->m_nImageIndex = II_YEAR;
            sprintf(szName, "%d's", nDecadeYear * 10);
            pNewNode->m_strName = szName;
            addChildBack(pNewNode);
            for (int k = 0; k < (int)pvInt->size(); k++) {
                if (vTenYear[i] == pvInt->at(k) / 10) {
                    sprintf(szName, "%d", pvInt->at(k));
                    pNewNode->addNewNode(szName, FT_YEAR, II_YEAR, false);
                }
            }
        }
    } else if (folderType == FT_YEAR) {
        int nYear;
        nYear = atoi(m_strName.c_str());
        mediaLib->getByYear(nYear, &m_playlist, MLOB_NONE, -1);
    }
    // rating
    else if (folderType == FT_ALL_RATING) {
        int i;
        string name = "*****";

        addNewNode("Not rated", FT_NOT_RATED, II_RATING, false);

        for (i = 4; i >= 0; i--) {
            name.resize(i + 1);
            addNewNode(name.c_str(), FT_RATING_1 + i, II_RATING, false);
        }
        //        mediaLib->getTopRating(16, &m_playlist);
        //        if (nRet == ERR_OK && m_playlist)
        //            g_Player.filterLowRatingMedia(m_playlist);
    } else if (folderType == FT_NOT_RATED || folderType == FT_RATING_1 ||
        folderType == FT_RATING_2 || folderType == FT_RATING_3 ||
        folderType == FT_RATING_4 || folderType == FT_RATING_5) {
        mediaLib->getByRating(folderType - FT_NOT_RATED, &m_playlist, MLOB_RATING, -1);
    } else if (folderType == FT_TOP_PLAYED) {
        mediaLib->getTopPlayed(50, &m_playlist);
        setUpdated(false);
    } else if (folderType == FT_RECENT_ADDED) {
        mediaLib->getRecentAdded(32, &m_playlist);
        setUpdated(false);
    } else if (folderType == FT_RECENT_PLAYED) {
        mediaLib->getRecentPlayed(32, &m_playlist);
        setUpdated(false);
    }
}

void CMPMTNDiskDevice::onUpdate() {
    if (m_nImageIndex == II_MYCOMPUTER) {
        // root of my computer
#ifdef _WIN32
        char szStr[512] = "";
        int nRet;
        VecStrings vStr;
        int nImage;
        uint32_t nType;

        nRet = GetLogicalDriveStrings(CountOf(szStr), szStr);
        multiStrToVStr(szStr, vStr);
        for (int i = 0; i < (int)vStr.size(); i++) {
            nType = GetDriveType(vStr[i].c_str());
            if (nType == DRIVE_REMOVABLE) {
                nImage = II_DRIVE_REMOVABLE;
            } else if (nType == DRIVE_FIXED) {
                nImage = II_DRIVE_FIXED;
            } else if (nType == DRIVE_REMOTE) {
                nImage = II_DRIVE_REMOTE;
            } else if (nType == DRIVE_CDROM) {
                nImage = II_DRIVE_CDROM;
            } else if (nType == DRIVE_RAMDISK) {
                nImage = II_DRIVE_RAMDISK;
            } else {
                continue;
            }

            addNewNode(vStr[i].c_str(), vStr[i].c_str(), nImage, nImage);
        }
#else
#endif
    } else {
        FileFind finder;
        string strFile;

        if (finder.openDir(m_strDir.c_str())) {
            while (finder.findNext()) {
                if (finder.isCurDir()) {
                    if (strcmp(finder.getCurName(), ".") != 0 &&
                        strcmp(finder.getCurName(), "..") != 0) {
                        addNewNode(finder.getCurName(), (m_strDir + finder.getCurName()).c_str(), II_FOLDER_CLOSED, II_FOLDER_OPENED);
                    }
                } else {
                    // Is file an audio media?
                    if (g_Player.isExtAudioFile(fileGetExt(finder.getCurName()))) {
                        if (!m_playlist) {
                            if (g_Player.newPlaylist(&m_playlist) != ERR_OK) {
                                return;
                            }
                        }
                        strFile = m_strDir + finder.getCurName();
                        CMPAutoPtr<IMedia> media;
                        if (g_Player.newMedia(&media, strFile.c_str()) == ERR_OK) {
                            m_playlist->insertItem(-1, media);
                        }
                    }
                }
            }
        }
    }
}

void CMPMTNPlaylists::onUpdate() {
    if (folderType == FT_ALL_PLAYLISTS) {
        // enum all the playlist files
        vector<string> vFiles;

        enumPlaylistsFast(vFiles);
        for (int i = 0; i < (int)vFiles.size(); i++) {
            CMPMTNPlaylists *playlistNode;

            playlistNode = new CMPMTNPlaylists;
            playlistNode->m_strName = fileGetName(vFiles[i].c_str());
            playlistNode->m_nImageIndex = II_PLAYLISTS;
            playlistNode->folderType = FT_PLAYLIST_FILE;
            playlistNode->m_strPlaylistFile = vFiles[i];
            addChildBack(playlistNode);
        }
    } else if (folderType == FT_PLAYLIST_FILE) {
        //
        if (g_Player.newPlaylist(&m_playlist) != ERR_OK) {
            return;
        }
        loadPlaylist(g_Player.getIMPlayer(), m_playlist, m_strPlaylistFile.c_str());
    }
}


CMPMediaTree::CMPMediaTree() {

}

CMPMediaTree::~CMPMediaTree() {

}

bool CMPMediaTree::init() {
    IMPMediaTreeNode *pNodeChild;
    CMPMTNMediaLibrary *pMediaLibNode;

    pMediaLibNode = new CMPMTNMediaLibrary;
    pMediaLibNode->m_strName = "Media Library";
    pMediaLibNode->m_nImageIndex = II_MEDIA_LIB;
    pMediaLibNode->setExpanded(true);
    pMediaLibNode->setUpdated(true);
    m_root.addChildBack(pMediaLibNode);


    pMediaLibNode->addNewNode("Location", CMPMTNMediaLibrary::FT_LOCATION, II_LOCATION, false);
    pMediaLibNode->addNewNode("All Songs By Artist", CMPMTNMediaLibrary::FT_ALL_SONGS_BY_ARTIST, II_MUSIC, false);
    pMediaLibNode->addNewNode("Artist", CMPMTNMediaLibrary::FT_ALL_ARTIST, II_ARTIST, false);
    pMediaLibNode->addNewNode("Album", CMPMTNMediaLibrary::FT_ALL_ALBUM, II_ALBUM, false);
    pMediaLibNode->addNewNode("Genre", CMPMTNMediaLibrary::FT_ALL_GENRE, II_GENRE, false);
    pMediaLibNode->addNewNode("Year", CMPMTNMediaLibrary::FT_ALL_YEAR, II_YEAR, false);
    pMediaLibNode->addNewNode("Rating", CMPMTNMediaLibrary::FT_ALL_RATING, II_RATING, false);
    pMediaLibNode->addNewNode("Top 50 played", CMPMTNMediaLibrary::FT_TOP_PLAYED, II_ALBUM, false);
    pMediaLibNode->addNewNode("Recent played", CMPMTNMediaLibrary::FT_RECENT_PLAYED, II_ALBUM, false);
    pMediaLibNode->addNewNode("Recent Added", CMPMTNMediaLibrary::FT_RECENT_ADDED, II_ALBUM, false);

    pNodeChild = new CMPMTNPlaylists;
    pNodeChild->m_strName = "Playlists";
    pNodeChild->m_nImageIndex = II_PLAYLISTS;
    m_root.addChildBack(pNodeChild);

    CMPMTNDiskDevice *pDeviceNode;
    pDeviceNode = new CMPMTNDiskDevice;
    pDeviceNode->setUpdated(false);
    pDeviceNode->m_strName = "My Computer";
    pDeviceNode->m_nImageIndex = II_MYCOMPUTER;
    m_root.addChildBack(pDeviceNode);

    return true;
}

void CMPMediaTree::close() {

}

bool CMPMediaTree::getCurNodePlaylist(IPlaylist **playlist) {
    IMPMediaTreeNode *pNode;

    pNode = (IMPMediaTreeNode *)getSelNode();
    if (!pNode) {
        return false;
    }

    if (pNode->m_playlist.p) {
        *playlist = pNode->m_playlist;
        (*playlist)->addRef();

        return true;
    }

    return false;
}
