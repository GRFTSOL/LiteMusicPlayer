#pragma once

namespace LyricsDownloader
{

class WorkStatus
{
public:
    WorkStatus() {
        m_bWorking = false;
        m_bCancel = false;
        m_pNotify = nullptr;
    }

    class INotify
    {
    public:
        virtual void cancel() = 0;

    };

    bool isCancel() { return m_bCancel; }
    bool isWorking() { return m_bWorking; }
    void cancel() { m_bWorking = true; if (m_pNotify) m_pNotify->cancel(); }
    void setWorking(bool bWorking) { m_bWorking = bWorking; }

    void setNotify(INotify *pNotify) { m_pNotify = pNotify; }

    volatile bool        m_bWorking;
    volatile bool        m_bCancel;
    INotify                *m_pNotify;

};

class ILyricsToolWork
{
public:
    enum SuccessStatus
    {
        SS_NONE,
        SS_OK,
        SS_ERROR,
        SS_INFO,
    };

    class IUpdateNotify
    {
    public:
        virtual void onUpdateItemStatus(int nIndex, SuccessStatus status, cstr_t szResult) = 0;
    };

public:
    ILyricsToolWork(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : 
            m_pMediaLib(pMediaLib), m_pUpdateNotify(pUpdateNotify), m_status(status) {
        m_nLastError = ERR_OK;
    }
    virtual ~ILyricsToolWork() { }

    virtual void onBegin() { m_nLastError = ERR_OK; }
    virtual void onProcessItem(int nIndex) = 0;
    virtual void onEnd(string &strMsg) = 0;

    void updateItemStatus(int nIndex, SuccessStatus status, cstr_t szResult) { m_pUpdateNotify->onUpdateItemStatus(nIndex, status, szResult); }

protected:
    WorkStatus            &m_status;
    IMediaSource        *m_pMediaLib;
    IUpdateNotify        *m_pUpdateNotify;
    int                    m_nLastError;

};

class CLyricsDownloadWork : public ILyricsToolWork, WorkStatus::INotify
{
public:
    CLyricsDownloadWork(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : ILyricsToolWork(pMediaLib, pUpdateNotify, status)
    {
        m_listSearchItems.reserve(20);
        m_nSavedEmbeddedLyrics = m_nHavedLyrics = 0;
        m_nSavedFileLyrics = 0;
        m_nFailedCount = 0;
        m_nNotFoundCount = 0;
        m_bOnlyUseLocalLyrics = false;
        status.setNotify(this);
    }

    ~CLyricsDownloadWork()
        { m_status.setNotify(nullptr); }

    virtual void cancel()
        { m_session.close(); }

    virtual void onProcessItem(int nIndex)
    {
        // search local lyrics and save to embedded lyrics
        string strLyrSrcPath;
        if (m_pMediaLib->getBestMatchLyrics(nIndex, strLyrSrcPath))
        {
            string strMediaSrc = m_pMediaLib->getLocation(nIndex);

            if (onLocalLyricsFound(nIndex, strMediaSrc.c_str(), strLyrSrcPath.c_str()) != ERR_NOT_FOUND)
                return;
        }

        if (m_bOnlyUseLocalLyrics)
            return;

        // add to search
        MLSearchItem    item;
        uint32_t nMediaLength = 0;
        m_pMediaLib->getMediaInfo(nIndex, item.strArtist, item.strAlbum, item.strTitle, item.strMediaFile, nMediaLength);
        item.nMediaLength = nMediaLength;
        item.nIndex = nIndex;

        // filter spam info in searches.
        if (item.strArtist.empty() || !filterSpamInfo(item.strArtist, item.strTitle))
        {
            updateItemStatus(nIndex, SS_ERROR, _TL("The artist and title is incomplete"));
            return;
        }

        m_listSearchItems.push_back(item);

        if (m_listSearchItems.size() >= 20)
        {
            m_nLastError = searchAndDownload();
            if (m_nLastError != ERR_OK)
                return;
        }
    }

    virtual void onEnd(string &strMsg)
    {
        if (m_listSearchItems.size() > 0 && m_nLastError == ERR_OK)
        {
            // Continue, if no error.
            m_nLastError = searchAndDownload();
        }

        strMsg.clear();
        if (m_nLastError != ERR_OK && !m_status.isCancel())
        {
            strMsg = _TLT("Error:");
            strMsg += " ";
            strMsg += ERROR2STR_LOCAL(m_nLastError);
            strMsg += "\n\n";
        }

        if (m_nFailedCount > 0)
        {
            strMsg += CStrPrintf(_TLT("%d lyrics were failed to save."), m_nFailedCount).c_str();
            strMsg += "\n";
        }

        strMsg += CStrPrintf(_TLT("%d lyrics are saved in audio files."), m_nSavedEmbeddedLyrics).c_str();
        strMsg += "\n";
        if (m_nSavedFileLyrics > 0)
        {
            strMsg += CStrPrintf(_TLT("%d lyrics files are saved."), m_nSavedFileLyrics).c_str();
            strMsg += "\n";
        }
        if (m_nHavedLyrics > 0)
            strMsg += CStrPrintf(_TLT("%d audio files already have lyrics."), m_nHavedLyrics).c_str();

        if (m_nNotFoundCount > 0)
        {
            strMsg += "\r\n\r\n";
            strMsg += CStrPrintf(_TLT("Can't find lyrics for %d audio files."), m_nNotFoundCount).c_str();
        }
    }

protected:
    int searchAndDownload()
    {
        assert(!m_listSearchItems.empty());

        // search lyrics by batch.
        string                strMsg;
        MLMsgRetBatchSearch retLyrics;

        int nRet = g_OnlineSearch.batchSearch(m_session, m_listSearchItems, retLyrics);
        if (nRet != ERR_OK)
        {
            for (int i = 0; i < m_listSearchItems.size(); i++)
            {
                MLSearchItem &search = m_listSearchItems[i];
                updateItemStatus(search.nIndex, SS_ERROR, 
                    CStrPrintf("%s: %s", _TLT("Failed to download lyrics"), ERROR2STR_LOCAL(nRet)).c_str());
            }
            m_listSearchItems.clear();
            return nRet;
        }

        assert(retLyrics.listLyricsInfo.size() == m_listSearchItems.size());

        ListLyricsInfoLite::iterator it = retLyrics.listLyricsInfo.begin();
        for (int i = 0; i < m_listSearchItems.size() && it != retLyrics.listLyricsInfo.end();
            i++, ++it)
        {
            MLSearchItem &search = m_listSearchItems[i];
            MLLyricsInfoLite &retSearch = *it;

            if (retSearch.bufLyrContent.empty())
            {
                m_nNotFoundCount++;
                updateItemStatus(search.nIndex, SS_ERROR, _TLT("No lyrics were found."));
                continue;
            }

            onLyricsDownloaded(search.nIndex, retSearch.strSaveName.c_str(), retSearch.bufLyrContent);
        }

        m_listSearchItems.clear();
        return nRet;
    }

    virtual int onLocalLyricsFound(int nMediaIndex, cstr_t szMediaFile, cstr_t szLyrName) = 0;

    virtual void onLyricsDownloaded(int nMediaIndex, cstr_t szLyrSaveName, string &bufLyr) = 0;

protected:
    MLListSearchItems        m_listSearchItems;
    CMLClientSession        m_session;
    int                        m_nSavedEmbeddedLyrics, m_nSavedFileLyrics, m_nHavedLyrics;
    int                        m_nFailedCount, m_nNotFoundCount;
    bool                    m_bOnlyUseLocalLyrics;

};

class CSaveLyricsInITunes : public CLyricsDownloadWork
{
public:
    CSaveLyricsInITunes(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : CLyricsDownloadWork(pMediaLib, pUpdateNotify, status)
    {
    }

    virtual void onProcessItem(int nIndex)
    {
        string    strLyrics;
        if (m_pMediaLib->getEmbeddedLyrics(nIndex, strLyrics))
        {
            m_nHavedLyrics++;
            updateItemStatus(nIndex, SS_OK, _TLT("Already has lyrics"));
            return;
        }

        CLyricsDownloadWork::onProcessItem(nIndex);
    }

protected:
    void setEmbeddedLyrics(CMLData &lyrData, int nIndex)
    {
        string strLyrics;

        lyrData.toString(strLyrics, g_profile.getBool("KeepTimeStampsInTxtLyr", false) ? FT_LYRICS_LRC : FT_LYRICS_TXT, true);
        int nRet = m_pMediaLib->setEmbeddedLyrics(nIndex, strLyrics);
        if (nRet == ERR_OK)
        {
            updateItemStatus(nIndex, SS_OK, _TLT("Lyrics were saved successfully"));
            m_nSavedEmbeddedLyrics++;
        }
        else
        {
            updateItemStatus(nIndex, SS_ERROR, 
                CStrPrintf("%s: %s", _TLT("Failed to save embedded lyrics"), ERROR2STR_LOCAL(nRet)).c_str());
            m_nFailedCount++;
        }
    }

    virtual int onLocalLyricsFound(int nMediaIndex, cstr_t szMediaFile, cstr_t szLyrName)
    {
        CMLData lyrData;
        int nRet = lyrData.openLyrics(szMediaFile, 0, szLyrName, false, ED_SYSDEF);
        if (nRet != ERR_OK)
            updateItemStatus(nMediaIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
        else
            setEmbeddedLyrics(lyrData, nMediaIndex);

        return nRet;
    }

    virtual void onLyricsDownloaded(int nMediaIndex, cstr_t szLyrSaveName, string &bufLyr)
    {
        // open lyrics
        CMLData lyrData;
        int nRet = lyrData.openLyrics("", 0, (uint8_t *)bufLyr.c_str(), (int)bufLyr.size());
        if (nRet != ERR_OK)
        {
            updateItemStatus(nMediaIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
            return;
        }

        // Convert to text lyrics.
        setEmbeddedLyrics(lyrData, nMediaIndex);
    }

};

class CRemoveEmbeddedLyrics : public ILyricsToolWork
{
public:
    CRemoveEmbeddedLyrics(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : ILyricsToolWork(pMediaLib, pUpdateNotify, status)
    {
        m_nCountEmbeddedRemoved = m_nCountFailed = 0;
        m_nCountFileRemoved = 0;
    }

    virtual void onProcessItem(int nIndex)
    {
        int nRet = m_pMediaLib->removeEmbeddedLyrics(nIndex);
        if (nRet == ERR_OK)
        {
            updateItemStatus(nIndex, SS_OK, _TLT("Embedded Lyrics were removed successfully"));
            m_nCountEmbeddedRemoved++;
        }
        else
        {
            if (nRet == ERR_NOT_FOUND)
                updateItemStatus(nIndex, SS_INFO, _TLT("No embedded lyrics"));
            else
                updateItemStatus(nIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
        }
    }

    virtual void onEnd(string &strMsg)
    {
        if (m_nLastError != ERR_OK && !m_status.isCancel())
        {
            strMsg = _TLT("Error:");
            strMsg += " ";
            strMsg += ERROR2STR_LOCAL(m_nLastError);
            strMsg += "\n\n";
        }

        strMsg += CStrPrintf(_TLT("%d of %d embedded lyrics are removed."), m_nCountEmbeddedRemoved, m_pMediaLib->getRowCount()).c_str();

        if (m_nCountFailed > 0)
        {
            strMsg += "\n";
            strMsg += CStrPrintf(_TLT("%d embedded lyrics are failed to remove."), m_nCountFailed).c_str();
        }
    }

protected:
    int                        m_nCountEmbeddedRemoved, m_nCountFileRemoved, m_nCountFailed;

};

class CAdvRemoveLyrics : public CRemoveEmbeddedLyrics
{
public:
    CAdvRemoveLyrics(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : CRemoveEmbeddedLyrics(pMediaLib, pUpdateNotify, status)
    {
    }

    void setInfo(bool bRemoveAssociateLyrFile, const VecStrings &vEmbeddedLyrNames)
    {
        m_bRemoveAssociateLyrFile = bRemoveAssociateLyrFile;
        m_vEmbeddedLyrNames = vEmbeddedLyrNames;
    }

    virtual void onProcessItem(int nIndex)
    {
        string strMediaFile = m_pMediaLib->getLocation(nIndex);

        if (m_vEmbeddedLyrNames.size() > 0)
        {
            int succeededCount = 0;
            int nRet = MediaTags::removeEmbeddedLyrics(strMediaFile.c_str(), m_vEmbeddedLyrNames, &succeededCount);
            m_nCountEmbeddedRemoved += succeededCount;
            if (succeededCount > 0)
                updateItemStatus(nIndex, SS_OK, _TLT("Embedded Lyrics were removed successfully"));
            else if (nRet == ERR_OK)
                updateItemStatus(nIndex, SS_INFO, _TLT("No embedded lyrics"));
            else
                updateItemStatus(nIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
        }

        if (m_bRemoveAssociateLyrFile)
        {
            char    szLyricsFile[MAX_PATH];
            if (g_LyricSearch.getAssociatedLyrics(strMediaFile.c_str(), szLyricsFile, CountOf(szLyricsFile)))
            {
                if (lyrSrcTypeFromName(szLyricsFile) == LST_FILE)
                {
                    m_nCountFileRemoved++;
                    deleteFile(szLyricsFile);
                    updateItemStatus(nIndex, SS_OK, _TLT("Lyrics file was removed successfully"));
                }
            }

            string strLyricsFile = strMediaFile;

            fileSetExt(strLyricsFile, ".lrc");
            if (isFileExist(strLyricsFile.c_str()))
                deleteFile(strLyricsFile.c_str());

            fileSetExt(strLyricsFile, ".txt");
            if (isFileExist(strLyricsFile.c_str()))
                deleteFile(strLyricsFile.c_str());
        }

    }

    virtual void onEnd(string &strMsg)
    {
        CRemoveEmbeddedLyrics::onEnd(strMsg);

        strMsg += "\n";
        strMsg += CStrPrintf(_TLT("%d of %d lyrics files are removed."), m_nCountFileRemoved, m_pMediaLib->getRowCount()).c_str();
    }

    bool            m_bRemoveAssociateLyrFile;
    VecStrings        m_vEmbeddedLyrNames;

};

class CAdvDownloadLyrWork : public CLyricsDownloadWork
{
public:
    CAdvDownloadLyrWork(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : CLyricsDownloadWork(pMediaLib, pUpdateNotify, status)
    {
        // m_downSaveName = m_bKeepFileName ? DOWN_SAVE_NAME_KEEP : DOWN_SAVE_NAME_AS_SONG_NAME;
    }

    void setInfo(DOWN_SAVE_DIR downSaveDir, DOWN_SAVE_NAME downSaveName, bool bOnlyUseLocalLyrics, const VecStrings &vSaveEmbeddedLyrNames)
    {
        m_bOnlyUseLocalLyrics = bOnlyUseLocalLyrics;
        m_downSaveDir = downSaveDir;
        m_downSaveName = downSaveName;
        m_vSaveEmbeddedLyrNames = vSaveEmbeddedLyrNames;

        m_nSavedEmbeddedLyrSrcType = 0;
        for (size_t i = 0; i < vSaveEmbeddedLyrNames.size(); i++)
        {
            m_nSavedEmbeddedLyrSrcType |= lyrSrcTypeFromName(m_vSaveEmbeddedLyrNames[i].c_str());
        }
    }

protected:
    void saveLyrics(cstr_t szMediaSrc, cstr_t szLyrFileName, cstr_t lyrBuf, size_t nLenLyr, int nIndex)
    {
        bool bFailed = false;
        int nRet;

        // save embedded lyrics
        if (m_vSaveEmbeddedLyrNames.size() > 0 && MediaTags::isEmbeddedLyricsSupported(szMediaSrc))
        {
            // If embedded lyrics exists, do NOT overwrite it.
            VecStrings vSaveEmbeddedLyrNames;
            uint32_t lstExisted = 0;
            if (searchEmbeddedLyrics(szMediaSrc, lstExisted) == ERR_OK)
            {
                for (size_t i = 0; i < m_vSaveEmbeddedLyrNames.size(); i++)
                {
                    if (!isFlagSet(lstExisted, lyrSrcTypeFromName(m_vSaveEmbeddedLyrNames[i].c_str())))
                    {
                        vSaveEmbeddedLyrNames.push_back(m_vSaveEmbeddedLyrNames[i]);
                        m_nHavedLyrics++;
                        updateItemStatus(nIndex, SS_OK, _TLT("Already has lyrics"));
                    }
                }
            }

            string bufLyrics;
            bufLyrics.append(lyrBuf, nLenLyr);
            int succeededCount = 0;
            nRet = g_autoProcessEmbeddedLyrics.saveEmbeddedLyrics(szMediaSrc, nullptr, &bufLyrics, vSaveEmbeddedLyrNames, false, &succeededCount);
            if (nRet == ERR_OK)
            {
                if (succeededCount > 0)
                {
                    updateItemStatus(nIndex, SS_OK, _TLT("Lyrics were saved successfully"));
                    m_nSavedEmbeddedLyrics++;
                }
            }
            else
            {
                bFailed = true;
                updateItemStatus(nIndex, SS_ERROR, 
                    CStrPrintf("%s: %s", _TLT("Failed to save embedded lyrics"), ERROR2STR_LOCAL(nRet)).c_str());
            }
        }

        if (m_downSaveDir == DOWN_SAVE_NO_FILE)
            return;

        // save as lyrics file
        string strLyrFile = g_LyricsDownloader.getSaveLyricsFile(szMediaSrc,
            szLyrFileName, m_downSaveDir, m_downSaveName);

        if (!isFileExist(strLyrFile.c_str()))
        {
            nRet = g_LyricsDownloader.saveDownloadedLyrAsFile(strLyrFile, lyrBuf, nLenLyr);
            if (nRet == ERR_OK)
            {
                if (!bFailed)
                    updateItemStatus(nIndex, SS_OK, _TLT("Lyrics were saved successfully"));
                m_nSavedFileLyrics++;
            }
            else
            {
                bFailed = true;
                updateItemStatus(nIndex, SS_ERROR, 
                    CStrPrintf(_TLT("Failed to save file: %s."), strLyrFile.c_str()).c_str());
            }
        }
        else
        {
            m_nHavedLyrics++;
            if (!bFailed)
                updateItemStatus(nIndex, SS_OK, _TLT("Already has lyrics"));
        }
    }

    virtual int onLocalLyricsFound(int nMediaIndex, cstr_t szMediaFile, cstr_t szLyrName)
    {
        CMLData lyrData;
        int nRet = lyrData.openLyrics(szMediaFile, 0, szLyrName, false, ED_SYSDEF);
        if (nRet != ERR_OK)
        {
            updateItemStatus(nMediaIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
            return nRet;
        }

        if (lyrData.getLyrContentType() == LCT_TXT)
        {
            // For text only lyrics, if we want to save sync lyrics, need to try to download it again.
            if ((m_nSavedEmbeddedLyrSrcType & (LST_ID3V2_SYLT | LST_LYRICS3V2)) != 0
                && !g_LyricSearch.isAssociatedLyrics(szMediaFile))
                return ERR_NOT_FOUND;
        }

        string str;
        lyrData.toString(str, FT_LYRICS_LRC, true);
        string bufLyrFile = insertWithFileBom(str);

        saveLyrics(szMediaFile, lyrData.getSuggestedLyricsFileName().c_str(), 
            bufLyrFile.c_str(), bufLyrFile.size(), nMediaIndex);

        return ERR_OK;
    }

    virtual void onLyricsDownloaded(int nMediaIndex, cstr_t szLyrSaveName, string &bufLyr)
    {
        string strMediaFile = m_pMediaLib->getLocation(nMediaIndex);
        if (strMediaFile.empty())
        {
            assert(0);
            return;
        }

        saveLyrics(strMediaFile.c_str(), szLyrSaveName, bufLyr.c_str(), bufLyr.size(), nMediaIndex);
    }

    DOWN_SAVE_DIR    m_downSaveDir;
    DOWN_SAVE_NAME    m_downSaveName;
    VecStrings        m_vSaveEmbeddedLyrNames;
    uint32_t            m_nSavedEmbeddedLyrSrcType;

};

}
