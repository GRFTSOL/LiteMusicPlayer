#import "../../Skin/Skin.h"
#import "MediaSource.h"
#import "MPlayerAppBase.h"
#import "../../MediaTags/MediaTags.h"
#import "../AutoProcessEmbeddedLyrics.h"


IMediaSource::~IMediaSource()
{
    for (size_t i = 0; i < m_vTracks.size(); i++)
    {
        delete m_vTracks[i];
    }
    m_vTracks.clear();
}

bool IMediaSource::getBestMatchLyrics(int nIndex, string &lyrSrcPath)
{
    uint32_t    nMediaLength;
    string artist, album, title, location;
    if (!getMediaInfo(nIndex, artist, album, title, location, nMediaLength))
        return false;

    return g_LyricSearch.getBestMatchLyrics(location.c_str(), artist.c_str(), title.c_str(), lyrSrcPath);
}

void IMediaSource::removeItem(int nIndex)
{
    if (nIndex >= 0 && nIndex < (int)m_vTracks.size())
    {
        delete m_vTracks[nIndex];
        m_vTracks.erase(m_vTracks.begin() + nIndex);
    }
}

void IMediaSource::setResult(int nIndex, cstr_t szResult)
{
    if (nIndex >= 0 && nIndex < (int)m_vTracks.size())
    {
        m_vTracks[nIndex]->result = szResult;
    }
}

int IMediaSource::getRowCount() const
{
    return (int)m_vTracks.size();
}

bool IMediaSource::isRowSelected(int row) const
{
    return m_vTracks[row]->isSelected;
}

void IMediaSource::setRowSelectionState(int nIndex, bool bSelected)
{
    m_vTracks[nIndex]->isSelected = bSelected;
}

int IMediaSource::getItemImageIndex(int row)
{
    return m_vTracks[row]->imageIndex;
}

bool IMediaSource::setItemImageIndex(int nItem, int nImageIndex, bool bRedraw)
{
    m_vTracks[nItem]->imageIndex = nImageIndex;
    if (bRedraw)
        m_pView->invalidate();

    return true;
}

cstr_t GetLyricsDescription(cstr_t szLyrics)
{
    LRC_SOURCE_TYPE    lst = lyrSrcTypeFromName(szLyrics);
    if (lst == LST_FILE)
        return szLyrics;
    return lyrSrcTypeToDesc(lst);
}

cstr_t IMediaSource::getCellText(int row, int col)
{
    Item *item = m_vTracks[row];

    if (col == COL_RESULT)
    {
        return item->result.c_str();
    }
    else if (col == COL_LYRICS)
    {
        if (item->lyrics.empty())
        {
            char        szLyrics[MAX_PATH];
            if (g_LyricSearch.getAssociatedLyrics(getLocation(row).c_str(), szLyrics, CountOf(szLyrics)))
            {
                if (strcmp(szLyrics, NONE_LYRCS) == 0)
                    item->lyrics = _TLT("No suitable lyrics for the song file");
                else
                    item->lyrics = GetLyricsDescription(szLyrics);
            }
        }
        return item->lyrics.c_str();
    }
    else
        return "";
    
    //    item.location = [[track location] UTF8String];
    //    item.album = [[track album] UTF8String];
    //    item.artist = [[track artist] UTF8String];
    //    item.title = [[track name] UTF8String];
    //
    //    item.duration = [track duration] * 1000;
    //
    
}

CRawImage *IMediaSource::getCellImage(int row, int col)
{
    return nullptr;
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
    dirStringAddSlash(strDir);

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
                    m_vTracks.push_back(new ItemF(strFile));
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
            m_vTracks.push_back(new ItemF(str));
        }
    }
}

void CFileMediaSource::addFile(cstr_t szFile)
{
    if (!isFileExist(szFile))
    {
        string str = szFile;
        m_vTracks.push_back(new ItemF(str));
    }
}

string CFileMediaSource::getLocation(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
        return "";
    
    ItemF *item = (ItemF*)m_vTracks[nIndex];

    return item->file;
}

bool CFileMediaSource::getMediaInfo(int nIndex, string &artist, string &album, string &title, string &location, uint32_t &nMediaLength)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
        return false;

    ItemF *item = (ItemF*)m_vTracks[nIndex];

    int nRet = MediaTags::getTagFast(item->file.c_str(), &artist, &title, &album,
        nullptr, nullptr, nullptr, &nMediaLength);
    if (nRet != ERR_OK || title.empty())
    {
        // Analyse file name
        analyseLyricsFileNameEx(artist, title, item->file.c_str());
    }
    nMediaLength /= 1000;
    location = item->file;

    return true;
}

void TryToRecoverDmagedM4aFiles(cstr_t szFile)
{
    // Bug fixes for adding ID3v2 tag to m4a files. This bug was fixed after: 7.6.43.
    if (!fileIsExtSame(szFile, ".m4a"))
        return;

    CID3v2IF        id3v2(ED_SYSDEF);

    int nRet = id3v2.open(szFile, true, false);
    if (nRet == ERR_OK)
    {
        nRet = id3v2.removeTagAndClose();
        LOG2(LOG_LVL_INFO, "Remove ID3v2 Tag from damaged M4A file: %s, Result: %d", szFile, nRet);
    }
}

bool CFileMediaSource::getEmbeddedLyrics(int nIndex, string &lyrics)
{
    lyrics.clear();

    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
        return false;

    ItemF *item = (ItemF*)m_vTracks[nIndex];

    if (!isFileExist(item->file.c_str()))
        return false;

    if (MediaTags::isID3v2TagSupported(item->file.c_str()))
    {
        // ID3v2 lyrics
        CID3v2IF        id3v2(ED_SYSDEF);
        int                nRet;

        nRet = id3v2.open(item->file.c_str(), false, false);
        if (nRet == ERR_OK)
        {
            ID3v2UnsynchLyrics unsyncLyrics;
            nRet = id3v2.getUnsyncLyrics(unsyncLyrics);
            if (nRet == ERR_OK)
                lyrics = unsyncLyrics.m_strLyrics;
            id3v2.close();
        }
    }
    else if (MediaTags::isM4aTagSupported(item->file.c_str()))
    {
        CM4aTag    tag;
        int nRet = tag.open(item->file.c_str(), false);
        if (nRet != ERR_OK)
            return false;

        nRet = tag.getLyrics(lyrics);
    }

    return lyrics.size() > 0;
}

// save lyrics as id3v2 unsync lyrics.
int CFileMediaSource::setEmbeddedLyrics(int nIndex, string &lyrics)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
        return ERR_FALSE;

    ItemF *item = (ItemF*)m_vTracks[nIndex];


    if (!MediaTags::isEmbeddedLyricsSupported(item->file.c_str()))
        return ERR_NOT_SUPPORT;

    TryToRecoverDmagedM4aFiles(item->file.c_str());

    VecStrings vLyrNames;
    string bufLyrics = insertWithFileBom(lyrics);
    if (MediaTags::isID3v2TagSupported(item->file.c_str()))
        vLyrNames.push_back(SZ_SONG_ID3V2_USLT);
    else if (MediaTags::isM4aTagSupported(item->file.c_str()))
        vLyrNames.push_back(SZ_SONG_M4A_LYRICS);
    else
        return ERR_NOT_SUPPORT;

    return g_autoProcessEmbeddedLyrics.saveEmbeddedLyrics(item->file.c_str(), nullptr, &bufLyrics, vLyrNames);
}

int CFileMediaSource::removeEmbeddedLyrics(int nIndex)
{
    if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
        return ERR_FALSE;
    ItemF *item = (ItemF*)m_vTracks[nIndex];

    int nRemovedCount = 0;
    VecStrings vLyrNames;
    // vLyrNames.push_back(SZ_SONG_ID3V2_SYLT);
    if (MediaTags::isID3v2TagSupported(item->file.c_str()))
        vLyrNames.push_back(SZ_SONG_ID3V2_USLT);
    else if (MediaTags::isM4aTagSupported(item->file.c_str()))
    {
        TryToRecoverDmagedM4aFiles(item->file.c_str());

        vLyrNames.push_back(SZ_SONG_M4A_LYRICS);
    }
    else
        return ERR_NOT_SUPPORT;

    return MediaTags::removeEmbeddedLyrics(item->file.c_str(), vLyrNames, &nRemovedCount);
}

bool CFileMediaSource::isFileExist(cstr_t szFile)
{
    for (int i = 0; i < (int)m_vTracks.size(); i++)
    {
        if (strcmp(szFile, ((ItemF *)m_vTracks[i])->file.c_str()) == 0)
            return true;
    }

    return false;
}

cstr_t CFileMediaSource::getCellText(int row, int col)
{
    if (col != COL_MEDIA_FILE)
        return IMediaSource::getCellText(row, col);

    Item *item = m_vTracks[row];
    string &file = ((ItemF*)m_vTracks[row])->file;

    if (item->title.empty())
    {
        item->title = fileGetTitle(file.c_str());
    }
    
    return item->title.c_str();
}

IMediaSource *newMediaSource(MediaSourceType type) {
    return new CFileMediaSource();
}
