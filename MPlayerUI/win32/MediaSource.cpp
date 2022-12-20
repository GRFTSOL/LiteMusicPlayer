#include "MPlayerAppBase.h"
#include "MediaSource.h"
#include "../MediaTags/MediaTags.h"
#include "../pluginiTunes/iTunesCOMInterface_i.c"
#include "AutoProcessEmbeddedLyrics.h"

bool IMediaSource::getBestMatchLyrics(int nIndex, string &lyrSrcPath)
{
    uint32_t    nMediaLength;
    string artist, album, title, location;
    if (!getMediaInfo(nIndex, artist, album, title, location, nMediaLength))
        return false;

    return g_LyricSearch.getBestMatchLyrics(location.c_str(), artist.c_str(), title.c_str(), lyrSrcPath);
}


//////////////////////////////////////////////////////////////////////////

CiTunesMediaSource::CiTunesMediaSource() : IMediaSource(MST_ITUNES)
{
}

CiTunesMediaSource::~CiTunesMediaSource()
{

}

bool CiTunesMediaSource::updateLibrary(string &err)
{
    if (m_iTunesApp == nullptr)
    {
        HRESULT hr = ::CoCreateInstance(CLSID_iTunesApp, nullptr, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&m_iTunesApp);
        if (FAILED(hr) || m_iTunesApp == nullptr)
        {
            ERR_LOG1("Failed to create iTunesApp Instance: %X", hr);
            err = _TLT("Maybe you haven't installed iTunes.");
            return false;
        }
#ifdef _DEBUG
        m_threadId = GetCurrentThreadId();
#endif
    }

    CMPAutoPtr<IITLibraryPlaylist> mainLibrary;
    HRESULT hr = m_iTunesApp->get_LibraryPlaylist(&mainLibrary);
    if (FAILED(hr) || mainLibrary == nullptr)
        return false;

    CMPAutoPtr<IITTrackCollection> tracks;
    hr = mainLibrary->get_Tracks(&tracks);
    if (FAILED(hr) || tracks == nullptr)
        return false;

    long countTracks = 0;
    hr = tracks->get_Count(&countTracks);
    if (FAILED(hr))
        return false;

    for (int i = 0; i < countTracks; i++)
    {
        CMPAutoPtr<IITTrack> track;
        HRESULT hr = tracks->get_Item(i, &track);
        if (FAILED(hr) || track == nullptr)
            continue;

        ITTrackKind kind;
        hr = track->get_Kind(&kind);
        if (FAILED(hr))
            continue;

        if (kind != ITTrackKindFile)
            continue;

        IITFileOrCDTrack *trackFile = nullptr;
        hr = track->queryInterface(IID_IITFileOrCDTrack, (void **)&trackFile);
        if (FAILED(hr) || trackFile == nullptr)
            continue;

        Item item;
        item.trackFile = trackFile;
        updateMediaInfo(item);
        m_vTracks.push_back(item);
    }

    return true;
}

int CiTunesMediaSource::getItemCount()
{
    return (int)m_vTracks.size();
}

bool CiTunesMediaSource::getLocation(int nIndex, string &location)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
    {
        assert(0);
        return false;
    }

    Item &item = m_vTracks[nIndex];
    location = item.location;

    return true;
}

bool CiTunesMediaSource::getMediaInfo(int nIndex, string &artist, string &album, string &title, string &location, uint32_t &nMediaLength)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
    {
        assert(0);
        return false;
    }

    Item &item = m_vTracks[nIndex];
    artist = item.artist;
    album = item.album;
    title = item.title;
    location = item.location;
    nMediaLength = item.duration;

    return true;
}

bool CiTunesMediaSource::updateMediaInfo(Item &item)
{
    IITFileOrCDTrack *trackFile = item.trackFile;

    bstr_s bstrLocation, bstrArtist, bstrAlbum, bstrTitle;
    HRESULT hr = trackFile->get_Location(&bstrLocation);
    if (FAILED(hr) || bstrLocation == nullptr)
        return false;

    hr = trackFile->get_Artist(&bstrArtist);
    if (SUCCEEDED(hr) && !isEmptyString(bstrArtist.c_str()))
        item.artist = bstrArtist.c_str();

    hr = trackFile->get_Album(&bstrAlbum);
    if (SUCCEEDED(hr) && !isEmptyString(bstrAlbum.c_str()))
        item.album = bstrAlbum.c_str();

    hr = trackFile->get_Name(&bstrTitle);
    if (SUCCEEDED(hr) && !isEmptyString(bstrTitle.c_str()))
        item.title = bstrTitle.c_str();

    item.location = bstrLocation.c_str();

    long duration = 0;
    hr = trackFile->get_Duration(&duration);
    item.duration = duration;

    return true;
}

bool CiTunesMediaSource::getEmbeddedLyrics(int nIndex, string &lyrics)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
    {
        assert(0);
        return false;
    }

    assert(m_threadId == GetCurrentThreadId());

    IITFileOrCDTrack *trackFile = m_vTracks[nIndex].trackFile;

    bstr_s bstrLyrics;
    HRESULT hr = trackFile->get_Lyrics(&bstrLyrics);
    if (FAILED(hr) || isEmptyString(bstrLyrics.c_str()))
        return false;

    lyrics = bstrLyrics.c_str();

    return true;
}

int CiTunesMediaSource::setEmbeddedLyrics(int nIndex, string &lyrics)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
    {
        assert(0);
        return ERR_FALSE;
    }

    assert(m_threadId == GetCurrentThreadId());

    IITFileOrCDTrack *trackFile = m_vTracks[nIndex].trackFile;

    bstr_s    bstrLyrics(lyrics.c_str());
    HRESULT hr = trackFile->put_Lyrics(bstrLyrics.c_str());
    if (FAILED(hr))
    {
        setCustomErrorDesc(OSError(hr).Description());
        return ERR_CUSTOM_ERROR;
    }

    return ERR_OK;
}

int CiTunesMediaSource::removeEmbeddedLyrics(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
    {
        assert(0);
        return ERR_FALSE;
    }

    assert(m_threadId == GetCurrentThreadId());

    IITFileOrCDTrack *trackFile = m_vTracks[nIndex].trackFile;

    bstr_s bstrLocation;
    HRESULT hr = trackFile->get_Location(&bstrLocation);
    if (FAILED(hr))
        return false;

    bstr_s bstrLyrics;
    hr = trackFile->get_Lyrics(&bstrLyrics);
    if (FAILED(hr))
    {
        ERR_LOG1("Failed to get lyrics: %s", OSError(hr).Description());
        return ERR_NOT_FOUND;
    }

    if (isEmptyString(bstrLyrics.c_str()))
        return ERR_NOT_FOUND;

    bstr_s    bstrLyricsEmpty(L"");
    hr = trackFile->put_Lyrics(bstrLyricsEmpty.c_str());
    if (FAILED(hr))
    {
        setCustomErrorDesc(OSError(hr).Description());
        return ERR_CUSTOM_ERROR;
    }

    return ERR_OK;
}

void CiTunesMediaSource::removeItem(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
    {
        assert(0);
        return;
    }

    m_vTracks[nIndex].trackFile->release();
    m_vTracks.erase(m_vTracks.begin() + nIndex);
}

//////////////////////////////////////////////////////////////////////////

void CFileMediaSource::addFolder(cstr_t szFolder, bool bIncludeSubFolder)
{
    FileFind        find;
    string            strDir;
    string            strFile;

    if (!find.openDir(szFolder))
        return;

    strDir = szFolder;
    dirStringAddSep(strDir);

    while (find.findNext())
    {
        if (find.isCurDir() && bIncludeSubFolder)
        {
            strFile = strDir + find.getCurName();
            addFolder(strFile.c_str(), bIncludeSubFolder);
        }
        else
        {
            if (MediaTags::isMediaTypeSupported(find.getCurName()))
            {
                strFile = strDir + find.getCurName();
                if (!isFileExist(strFile.c_str()))
                {
                    m_vFiles.push_back(Item(strFile));
                }
            }
        }
    }
}

void CFileMediaSource::addFiles(const VecStrings &vFiles)
{
    for (uint32_t i = 0; i < vFiles.size(); i++)
    {
        if (!isFileExist(vFiles[i].c_str()))
        {
            string    str = vFiles[i];
            m_vFiles.push_back(Item(str));
        }
    }
}

void CFileMediaSource::addFile(cstr_t szFile)
{
    if (!isFileExist(szFile))
    {
        string str = szFile;
        m_vFiles.push_back(Item(str));
    }
}

void CFileMediaSource::close()
{
    m_vFiles.clear();
}

int CFileMediaSource::getItemCount()
{
    return (int)m_vFiles.size();
}

bool CFileMediaSource::getLocation(int nIndex, string &location)
{
    if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
        return false;

    location = m_vFiles[nIndex].file;

    return true;
}

bool CFileMediaSource::getMediaInfo(int nIndex, string &artist, string &album, string &title, string &location, uint32_t &nMediaLength)
{
    if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
        return false;

    Item &item = m_vFiles[nIndex];

    if (!item.bInfoAvailable)
    {
        int nRet = MediaTags::getTagFast(item.file.c_str(), &item.artist, &item.title, &item.album,
            nullptr, nullptr, nullptr, &item.nMediaLength);
        if (nRet != ERR_OK || item.title.empty())
        {
            // Analyse file name
            analyseLyricsFileNameEx(item.artist, item.title, item.file.c_str());
        }
        item.bInfoAvailable = true;
        item.nMediaLength /= 1000;
    }

    location = item.file;
    artist = item.artist;
    album = item.album;
    title = item.title;
    nMediaLength = item.nMediaLength;

    return true;
}

void tryToRecoverDmagedM4aFiles(cstr_t szFile)
{
    // Bug fixes for adding ID3v2 tag to m4a files. This bug was fixed after: 7.6.43.
    if (!fileIsExtSame(szFile, ".m4a"))
        return;

    CID3v2IF        id3v2(ED_SYSDEF);

    int nRet = id3v2.open(szFile, true, false);
    if (nRet == ERR_OK)
    {
        nRet = id3v2.removeTagAndClose();
        LOG2(LOG_LVL_INFO, "remove ID3v2 Tag from damaged M4A file: %s, Result: %d", szFile, nRet);
    }
}

bool CFileMediaSource::getEmbeddedLyrics(int nIndex, string &lyrics)
{
    lyrics.clear();

    if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
        return false;

    Item &item = m_vFiles[nIndex];

    if (!isFileExist(item.file.c_str()))
        return false;

    if (MediaTags::isID3v2TagSupported(item.file.c_str()))
    {
        // ID3v2 lyrics
        CID3v2IF        id3v2(ED_SYSDEF);
        int                nRet;

        nRet = id3v2.open(item.file.c_str(), false, false);
        if (nRet == ERR_OK)
        {
            ID3v2UnsynchLyrics unsyncLyrics;
            nRet = id3v2.getUnsyncLyrics(unsyncLyrics);
            if (nRet == ERR_OK)
                lyrics = unsyncLyrics.m_strLyrics;
            id3v2.close();
        }
    }
    else if (MediaTags::isM4aTagSupported(item.file.c_str()))
    {
        CM4aTag    tag;
        int nRet = tag.open(item.file.c_str(), false);
        if (nRet != ERR_OK)
            return false;

        nRet = tag.getLyrics(lyrics);
    }

    return lyrics.size() > 0;
}

// save lyrics as id3v2 unsync lyrics.
int CFileMediaSource::setEmbeddedLyrics(int nIndex, string &lyrics)
{
    if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
        return ERR_FALSE;

    Item &item = m_vFiles[nIndex];


    if (!MediaTags::isEmbeddedLyricsSupported(item.file.c_str()))
        return ERR_NOT_SUPPORT;

    tryToRecoverDmagedM4aFiles(item.file.c_str());

    VecStrings vLyrNames;
    string bufLyrics = insertWithFileBom(lyrics);
    if (MediaTags::isID3v2TagSupported(item.file.c_str()))
        vLyrNames.push_back(SZ_SONG_ID3V2_USLT);
    else if (MediaTags::isM4aTagSupported(item.file.c_str()))
        vLyrNames.push_back(SZ_SONG_M4A_LYRICS);
    else
        return ERR_NOT_SUPPORT;

    return g_autoProcessEmbeddedLyrics.saveEmbeddedLyrics(item.file.c_str(), nullptr, &bufLyrics, vLyrNames);
}

int CFileMediaSource::removeEmbeddedLyrics(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
        return ERR_FALSE;
    Item &item = m_vFiles[nIndex];

    int nRemovedCount = 0;
    VecStrings vLyrNames;
    // vLyrNames.push_back(SZ_SONG_ID3V2_SYLT);
    if (MediaTags::isID3v2TagSupported(item.file.c_str()))
        vLyrNames.push_back(SZ_SONG_ID3V2_USLT);
    else if (MediaTags::isM4aTagSupported(item.file.c_str()))
    {
        tryToRecoverDmagedM4aFiles(item.file.c_str());

        vLyrNames.push_back(SZ_SONG_M4A_LYRICS);
    }
    else
        return ERR_NOT_SUPPORT;

    return MediaTags::removeEmbeddedLyrics(item.file.c_str(), vLyrNames, &nRemovedCount);
}

void CFileMediaSource::removeItem(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
        return;

    m_vFiles.erase(m_vFiles.begin() + nIndex);
}

void CFileMediaSource::expandMediaFiles(VecStrings &vFiles)
{
    for (int i = 0; i < (int)vFiles.size(); ++i)
    {
        if (MediaTags::isMediaTypeSupported(vFiles[i].c_str())
            && !isFileExist(vFiles[i].c_str()))
        {
            m_vFiles.push_back(Item(vFiles[i]));
        }
    }
}

bool CFileMediaSource::isFileExist(cstr_t szFile)
{
    for (int i = 0; i < (int)m_vFiles.size(); i++)
    {
        if (strcmp(szFile, m_vFiles[i].file.c_str()) == 0)
            return true;
    }

    return false;
}
