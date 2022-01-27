#include "MPlayerApp.h"
#include "../LyricsLib/HelperFun.h"


static cstr_t        SZ_SUPPORTED_IMG_EXT[] = { ".jpg", ".gif", ".bmp", ".png" };

bool isSupportedImageFile(cstr_t szFile)
{
    cstr_t            szExt;
    int                i;

    szExt = strrchr(szFile, '.');
    if (!szExt)
        return false;

    for (i = 0; i < CountOf(SZ_SUPPORTED_IMG_EXT); i++)
    {
        if (strcasecmp(szExt, SZ_SUPPORTED_IMG_EXT[i]) == 0)
            return true;
    }

    return false;
}

/*
bool getImageFileWithName(cstr_t szName, VecStrings &vPicFiles)
{
    string                strPicFile;
    int            i;

    for (i = 0; i < CountOf(SZ_SUPPORTED_IMG_EXT); i++)
    {
        strPicFile = szName;
        strPicFile += SZ_SUPPORTED_IMG_EXT[i];
        if (isFileExist(strPicFile.c_str()))
        {
            // Find it:)
            vPicFiles.push_back(strPicFile);
        }
    }

    return false;
}*/

bool getCurrentMediaAlbumArtInSongDir(VecStrings &vPicFiles)
{
    string        strFile;
    cstr_t        szAlbumName = g_Player.getAlbum(), szSongFile = g_Player.getSrcMedia();
    VecStrings        vOtherPicFiles;
    int            nFileCount = 0;

    if (!isFileExist(szSongFile))
        return false;

    FileFind        find;

    string strDir = fileGetPath(szSongFile);
    if (!find.openDir(strDir.c_str()))
        return false;

    string        file;
    while (find.findNext())
    {
        // enum every image file

        if (find.isCurDir())
            continue;

        nFileCount++;
        if (!isSupportedImageFile(find.getCurName()))
            continue;

        strFile = strDir;
        strFile += find.getCurName();
        string strFileTitle = fileGetTitle(find.getCurName());

        //
        // album.jpg
        //
        if (strcasecmp(strFileTitle.c_str(), szAlbumName) == 0)
        {
            vPicFiles.push_back(strFile);
            continue;
        }

        //
        // artist - album.jpg
        //
        string        strArAl;
        strArAl = g_Player.getArtist();
        if (!isEmptyString(szAlbumName))
            strArAl = formatMediaTitle(g_Player.getArtist(), szAlbumName);
        if (strArAl.empty())
        {
            strArAl = fileGetTitle(g_Player.getSrcMedia());
        }
        if (strcasecmp(strFileTitle.c_str(), strArAl.c_str()) == 0)
        {
            vPicFiles.push_back(strFile);
            continue;
        }

        //
        // Folder.jpg
        //
        if (strcasecmp(strFileTitle.c_str(), "Folder") == 0)
        {
            vPicFiles.push_back(strFile);
            continue;
        }

        // other files
        vOtherPicFiles.push_back(strFile);
    }

    if (nFileCount < 60 && vPicFiles.size() < 1)
    {
        vPicFiles.insert(vPicFiles.begin(), vOtherPicFiles.begin(), vOtherPicFiles.end());
    }

    return true;
}

CCurMediaAlbumArt::CCurMediaAlbumArt()
{
    m_bLoaded = false;
}

CCurMediaAlbumArt::~CCurMediaAlbumArt()
{
}

int CCurMediaAlbumArt::load()
{
    string        strSongFile;
    int            nRet;

    close();

    m_bLoaded = true;
    strSongFile = g_Player.getSrcMedia();

    // load id3v2 pictures...
    CID3v2IF        id3v2(ED_SYSDEF);
    nRet = id3v2.open(strSongFile.c_str(), false, false);
    if (nRet == ERR_OK)
    {
        id3v2.getPictures(m_id3v2Pic);
    }

    // load album art in song dir, with same album name.
    getCurrentMediaAlbumArtInSongDir(m_vAlbumPicFile);

    if (getPicCount() > 0)
        return ERR_OK;
    else
        return ERR_NOT_FOUND;
}

void CCurMediaAlbumArt::close()
{
    m_id3v2Pic.free();
    m_vAlbumPicFile.clear();
    m_bLoaded = false;
}

RawImageData *CCurMediaAlbumArt::loadAlbumArtByIndex(int nIndex)
{
    if (nIndex < 0)
        return nullptr;

    if (nIndex < (int)m_id3v2Pic.m_vItems.size())
    {
        ID3v2Pictures::ITEM    *pic = m_id3v2Pic.m_vItems[nIndex];
        return loadRawImageDataFromMem(pic->m_buffPic.c_str(), pic->m_buffPic.size());
    }
    else
    {
        nIndex -= (int)m_id3v2Pic.m_vItems.size();
        if (nIndex < (int)m_vAlbumPicFile.size())
            return loadRawImageDataFromFile(m_vAlbumPicFile[nIndex].c_str());
        else
            return nullptr;
    }
}

int CCurMediaAlbumArt::getPicCount()
{
    int        n = m_id3v2Pic.m_vItems.size();

    n += m_vAlbumPicFile.size();

    return n;
}

