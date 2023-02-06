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
    auto mediaLib = g_player.getMediaLibrary();

    m_playlist = nullptr;

    if (folderType == FT_ALL_SONGS_BY_ARTIST) {
        m_playlist = mediaLib->getAll(MLOB_ARTIST, -1);
    } else if (folderType == FT_ALL_ARTIST) {
        VecStrings results = mediaLib->getAllArtist();
        for (auto &name : results) {
            if (name.empty()) {
                addNewNode("Unknown artist", FT_ARTIST_UNKNOWN, II_ARTIST, false);
            } else {
                addNewNode(name.c_str(), FT_ARTIST, II_ARTIST, false);
            }
        }
    } else if (folderType == FT_ALL_ALBUM) {
        VecStrings results = mediaLib->getAllAlbum();
        for (auto &name : results) {
            if (name.empty()) {
                addNewNode("Unknown Album", FT_ALBUM_UNKNOWN, II_ALBUM, false);
            } else {
                addNewNode(name.c_str(), FT_ALBUM, II_ALBUM, false);
            }
        }
    } else if (folderType == FT_ARTIST ||
        folderType == FT_ARTIST_UNKNOWN) {
        addNewNode("Top rated songs by artist", FT_ARTIST_TOP_RATING, II_MUSIC, false);

        VecStrings results;
        if (folderType == FT_ARTIST_UNKNOWN) {
            m_playlist = mediaLib->getByArtist("", MLOB_NONE, -1);
            results = mediaLib->getAlbumOfArtist("");
        } else {
            m_playlist = mediaLib->getByArtist(m_strName.c_str(), MLOB_NONE, -1);
            results = mediaLib->getAlbumOfArtist(m_strName.c_str());
        }

        for (auto &name : results) {
            if (name.empty()) {
                addNewNode("Unknown album", FT_ARTIST_ALBUM_UNKNOWN, II_ALBUM, false);
            } else {
                addNewNode(name.c_str(), FT_ARTIST_ALBUM, II_ALBUM, false);
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
            m_playlist = mediaLib->getByArtist(strArtist.c_str(), MLOB_NONE, -1);
        } else if (folderType == FT_ARTIST_TOP_RATING) {
            m_playlist = mediaLib->getByArtist(strArtist.c_str(), MLOB_RATING, -1);
            if (m_playlist) {
                g_player.filterLowRatingMedia(m_playlist.get());
            }
        } else {
            m_playlist = mediaLib->getByAlbum(strArtist.c_str(),
                folderType == FT_ARTIST_ALBUM_UNKNOWN ? "" : m_strName.c_str(),
                MLOB_NONE, -1);
        }
    } else if (folderType == FT_ALBUM_UNKNOWN ||
        folderType == FT_ALBUM) {
        m_playlist = mediaLib->getByAlbum(
            folderType == FT_ALBUM_UNKNOWN ? "" : m_strName.c_str(), MLOB_NONE, -1);
    }
    //////////////////////////////////////////////////////////////////////////
    // Genre
    else if (folderType == FT_ALL_GENRE) {
        VecStrings results = mediaLib->getAllGenre();
        for (auto &name : results) {
            if (name.empty()) {
                addNewNode("Unknown Genre", FT_GENRE_UNKNOWN, II_GENRE, false);
            } else {
                addNewNode(name.c_str(), FT_GENRE, II_GENRE, false);
            }
        }
    } else if (folderType == FT_GENRE_UNKNOWN ||
        folderType == FT_GENRE) {
        m_playlist = mediaLib->getByGenre(
            folderType == FT_GENRE_UNKNOWN ? "" : m_strName.c_str(), MLOB_NONE, -1);
    } else if (folderType == FT_ALL_YEAR) {
        // Year
//        // 对year 进行分类，每10年为一个子节点
//        CMPAutoPtr<IVInt> pvInt;
//        ResultCode nRet;
//        CSortVInt vTenYear;
//        int i;
//        char szName[256];
//
//        nRet = mediaLib->getAllYear(&pvInt);
//        if (nRet == ERR_OK) {
//            int nCount = (int)pvInt->size();
//            for (i = 0; i < nCount; i++) {
//                vTenYear.add(pvInt->at(i) / 10);
//                //                 if (pvInt->at(i) == 0)
//                //                     addNewNode("Unknown Genre", FT_GENRE_UNKNOWN, II_ALBUM, false);
//                //                 else
//                //                     addNewNode(pvStr->at(i), FT_GENRE, II_ALBUM, false);
//            }
//        }
//
//        for (i = 0; i < (int)vTenYear.size(); i++) {
//            CMPMTNMediaLibrary *pNewNode;
//            int nDecadeYear = vTenYear[i];
//
//            if (nDecadeYear == 0) {
//                addNewNode("Unknown", FT_YEAR, II_YEAR, false);
//                continue;
//            }
//
//            pNewNode = new CMPMTNMediaLibrary;
//            pNewNode->setUpdated(true);
//            pNewNode->folderType = FT_NOT_SET;
//            pNewNode->m_nImageIndex = II_YEAR;
//            sprintf(szName, "%d's", nDecadeYear * 10);
//            pNewNode->m_strName = szName;
//            addChildBack(pNewNode);
//            for (int k = 0; k < (int)pvInt->size(); k++) {
//                if (vTenYear[i] == pvInt->at(k) / 10) {
//                    sprintf(szName, "%d", pvInt->at(k));
//                    pNewNode->addNewNode(szName, FT_YEAR, II_YEAR, false);
//                }
//            }
//        }
    } else if (folderType == FT_YEAR) {
        int nYear;
        nYear = atoi(m_strName.c_str());
        m_playlist = mediaLib->getByYear(nYear, MLOB_NONE, -1);
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
    } else if (folderType == FT_NOT_RATED || folderType == FT_RATING_1 ||
        folderType == FT_RATING_2 || folderType == FT_RATING_3 ||
        folderType == FT_RATING_4 || folderType == FT_RATING_5) {
        m_playlist = mediaLib->getByRating(folderType - FT_NOT_RATED, MLOB_RATING, -1);
    } else if (folderType == FT_TOP_PLAYED) {
        m_playlist = mediaLib->getTopPlayed(50);
        setUpdated(false);
    } else if (folderType == FT_RECENT_ADDED) {
        m_playlist = mediaLib->getRecentAdded(32);
        setUpdated(false);
    } else if (folderType == FT_RECENT_PLAYED) {
        m_playlist = mediaLib->getRecentPlayed(32);
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
                    if (g_player.isExtAudioFile(fileGetExt(finder.getCurName()))) {
                        if (!m_playlist) {
                            m_playlist = g_player.newPlaylist();
                        }
                        strFile = m_strDir + finder.getCurName();
                        auto media = g_player.newMedia(strFile.c_str());
                        if (media) {
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
        m_playlist = loadPlaylist(m_strPlaylistFile.c_str());
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

PlaylistPtr CMPMediaTree::getCurNodePlaylist() {
    auto pNode = (IMPMediaTreeNode *)getSelNode();
    if (!pNode) {
        return nullptr;
    }

    return pNode->m_playlist;
}
