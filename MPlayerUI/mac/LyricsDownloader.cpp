// DlgAdjustHue.cpp: implementation of the CDlgAdjustHue class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "LyricsDownloader.h"
#include "OnlineSearch.h"
#include "DownloadMgr.h"
#include "MediaSource.h"
#include "../../LyricsLib/LrcParserHelper.h"
#include "DlgAbout.h"
#include "MPlaylistCtrl.h"
#include "../../Utils/Looper.h"
#include "AutoProcessEmbeddedLyrics.h"
//#include "LyricsDownloader_dialog.h"
#include "LyricsDownloader_worker.h"

using namespace LyricsDownloader;

#define SZ_NO_SUITABLE_LYRICS        "**NL"

class CPageLyricsTool : public CSkinContainer, public ILyricsToolWork::IUpdateNotify,  IUIObjNotifyHandler, IEventHandler
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
    CPageLyricsTool()
    {
        m_filesListCtrl = nullptr;
        m_pMediaLib = nullptr;
        m_nProgress = 0;
        CID_UPDATEPROGRESS = 0, CID_STATUS = 0;
        m_pProgressCtrl = nullptr;

        m_threadWorking.create(workThreadProc, this);
        
        m_nCurrentMedia = 0;
        m_nTimerIDPlayNext = 0;
    }
    ~CPageLyricsTool()
    {
        m_WorkLooper.quit();
        m_workStatus.cancel();
        m_threadWorking.join();
    }

    enum
    {
        COL_MEDIA_FILE,
        COL_RESULT,
        COL_LYRICS,
    };

    enum
    {
        IMG_NONE,
        IMG_OK,
        IMG_ERROR,
        IMG_INFO,
    };

    struct UpdateItem
    {
        UpdateItem(int nIndex, cstr_t szResult)
        {
            this->n = nIndex;
            this->strResult = szResult;
        }
        int            n;
        string        strResult;
    };
    typedef list<UpdateItem>    ListUpdateItem;

    void onCreate();
    virtual void onDestroy();

    virtual bool onCommand(int nId);

    virtual bool onCustomCommand(int nId);

    virtual bool onOK();
    virtual bool onCancel();

    void onUpdateItemStatus(int nIndex, ILyricsToolWork::SuccessStatus status, cstr_t szResult)
    {
        m_pMediaLib->setItemImageIndex(nIndex, status, false);

        MutexAutolock lock(m_mutexMsg);
        m_listUpdateItems.push_back(UpdateItem(nIndex, szResult));
    }

protected:
    void updateProgress();

    void clearResult();

    void fromCurrentPlaylist();

    void updateNewMediaList();

    void onAdvDownloadAllLyrics();
    void onAdvRemoveAllLyrics();

    void onDownloadLyricsForiPod();
    void onRemoveEmbeddedLyrics();

    void postWork(ILyricsToolWork *pWork);

    static void workThreadProc(void *param)
    {
        CPageLyricsTool *pThis = (CPageLyricsTool *)param;

        pThis->m_WorkLooper.loop();
    }

    void doWork(ILyricsToolWork *pWork);

    uint32_t getEmbeddedLyricsOpt();

    void changeToMediaLib(MediaSourceType type)
    {
        if (m_pMediaLib)
        {
            if (m_pMediaLib->getType() == type)
                return;

//            g_Player.clearPlaylist();
            delete m_pMediaLib;
        }

        m_pMediaLib = newMediaSource(type);
        m_filesListCtrl->setDataSource(m_pMediaLib);
        m_filesListCtrl->notifyDataChanged();
    }

    void hideInstructions()
    {
        m_pSkin->setUIObjectVisible(UID_INSTRUCTION, false, false);
        m_pSkin->setUIObjectVisible(UID_MEDIA_INFO, true, false);
        m_pSkin->invalidateRect();
    }
    
    void playItem(int nIndex, bool playNow);

    virtual void onUIObjNotify(IUIObjNotify *pNotify);

    virtual void onEvent(const IEvent *pEvent)
    {
        if (pEvent->eventType == ET_PLAYER_STATUS_CHANGED)
        {
            CEventPlayerStatusChanged *pStatusEvent = (CEventPlayerStatusChanged *)pEvent;
            if (pStatusEvent->status == PS_STOPED)
            {
                if (m_pMediaLib != nullptr && m_nCurrentMedia < m_pMediaLib->getRowCount())
                {
                    if (m_nTimerIDPlayNext != 0)
                        m_pSkin->unregisterTimerObject(this, m_nTimerIDPlayNext);
                    m_nTimerIDPlayNext = m_pSkin->registerTimerObject(this, 300);
                }
            }
        }
    }
    
    virtual void onTimer(int nId)
    {
        if (nId != m_nTimerIDPlayNext)
        {
            CSkinContainer::onTimer(nId);
            return;
        }
        
        m_pSkin->unregisterTimerObject(this, m_nTimerIDPlayNext);
        m_nTimerIDPlayNext = 0;
        if (g_Player.getPlayerState() == PS_STOPED
            && m_pMediaLib != nullptr && m_nCurrentMedia < m_pMediaLib->getRowCount())
        {
            playItem(m_nCurrentMedia + 1, true);
        }
    }

    friend class CRunnableDoWork;
    friend class CRunnableUpdateiTunesMediaLib;

protected:
    CSkinListView            *m_filesListCtrl;

    IMediaSource            *m_pMediaLib;

    std::mutex                    m_mutexMsg;
    string                    m_strMsg, m_strStatus;
    int                        m_nProgress;
    int                        CID_UPDATEPROGRESS, CID_STATUS, CID_CANCEL_WORK, UID_INSTRUCTION;
    int                        CMD_IMPORT, CMD_DL_LYRICS, UID_MEDIA_INFO, CID_PROGRESS, CID_LISTVIEW;
    CSkinSeekCtrl            *m_pProgressCtrl;
    ListUpdateItem            m_listUpdateItems;

    CThread                    m_threadWorking;
    CLooper                    m_WorkLooper;        // Used to loop the background work
    WorkStatus                m_workStatus;

    // Handle play/next.
    int                        m_nCurrentMedia;
    int                        m_nTimerIDPlayNext;

};

UIOBJECT_CLASS_NAME_IMP(CPageLyricsTool, "Container.LyricsTool");


void CPageLyricsTool::playItem(int nIndex, bool playNow)
{
    if (m_pMediaLib == nullptr)
        return;
    
    uint32_t    nMediaLength;
    string artist, album, title, location;
    if (!m_pMediaLib->getMediaInfo(nIndex, artist, album, title, location, nMediaLength))
        return;
    
    if (isFileExist(location.c_str()))
    {
        g_Player.clearPlaylist();
        g_Player.getIMPlayer()->setCurrentMedia(location.c_str());
        
        if (playNow)
            g_Player.play();
    }
    
    m_nCurrentMedia = nIndex;
}

void CPageLyricsTool::onUIObjNotify(IUIObjNotify *pNotify)
{
    if (pNotify->nID != CID_LISTVIEW)
        return;
    
    CSkinListCtrlEventNotify *pListViewNotify = (CSkinListCtrlEventNotify*)pNotify;
    if (pListViewNotify->cmd == CSkinListCtrlEventNotify::C_DBL_CLICK)
    {
        hideInstructions();
        playItem(pListViewNotify->nClickOnRow, true);
    }
}

void CPageLyricsTool::onCreate()
{
    CSkinContainer::onCreate();

    GET_ID_BY_NAME4(CID_STATUS, CID_UPDATEPROGRESS, CID_CANCEL_WORK, UID_INSTRUCTION);
    GET_ID_BY_NAME4(UID_MEDIA_INFO, CMD_IMPORT, CMD_DL_LYRICS, CID_PROGRESS);
    GET_ID_BY_NAME(CID_LISTVIEW);

    m_filesListCtrl = (CSkinListCtrl *)getUIObjectById(ID_PLAYLIST, CSkinListView::className());
    if (!m_filesListCtrl)
        return;

    cstr_t vColName[] = { "Media Files", "Result", "Lyrics" };
    for (int i = 0; i < CountOf(vColName); i++)
        m_filesListCtrl->addColumn(_TL(vColName[i]), 200);

    m_filesListCtrl->loadColumnWidth("LyricsTool_ListWidth");

    m_pProgressCtrl = (CSkinSeekCtrl *)m_pSkin->getUIObjectById(CID_PROGRESS, CSkinSeekCtrl::className());
    assert(m_pProgressCtrl);
    if (m_pProgressCtrl)
    {
        m_pProgressCtrl->setScrollInfo(0, 100, 0, 0, 1, false);
        m_pProgressCtrl->setEnable(false);
    }

    //
    // add current media source file.
    //
    fromCurrentPlaylist();

    m_filesListCtrl->setDataSource(m_pMediaLib);
    if (!m_pMediaLib || m_pMediaLib->getRowCount() == 0)
    {
        m_pSkin->setUIObjectVisible(UID_INSTRUCTION, true, false);
        m_pSkin->setUIObjectVisible(UID_MEDIA_INFO, false, false);
    }
    
    getSkinWnd()->registerUIObjNotifyHandler(CID_LISTVIEW, this);
    
    registerHandler(CSkinApp::getInstance()->getEventPatcher(), ET_PLAYER_STATUS_CHANGED);

/*    if (!isEmptyString(g_Player.getSrcMedia()))
    {
        changeToMediaLib(MST_FILES);
        CFileMediaSource *pFileMediaLib = (CFileMediaSource *)m_pMediaLib;
        VecStrings    vFiles;
        vFiles.push_back(g_Player.getSrcMedia());
        pFileMediaLib->addFiles(vFiles);
        updateNewMediaList();
    }*/
}

void CPageLyricsTool::onDestroy()
{
    if (m_filesListCtrl)
        m_filesListCtrl->saveColumnWidth("LyricsTool_ListWidth");
}

class CRunnableUpdateiTunesMediaLib : public IRunnable
{
public:
    CRunnableUpdateiTunesMediaLib(CPageLyricsTool *pPageLyricsTool)
            : m_pPageLyricsTool(pPageLyricsTool), m_eventFinished(false, false)
        { m_bUpdateSuccessfully = false; }

    void run()
    {
        assert(m_pPageLyricsTool->m_pMediaLib);

        m_bUpdateSuccessfully = m_pPageLyricsTool->m_pMediaLib->updateLibrary(m_strErr);
        m_eventFinished.set();
    }

public:
    bool                    m_bUpdateSuccessfully;
    string                    m_strErr;
    Event                    m_eventFinished;

protected:
    CPageLyricsTool            *m_pPageLyricsTool;

};

class CRunnableDoWork : public IRunnable
{
public:
    CRunnableDoWork(CPageLyricsTool *pPageLyricsTool, ILyricsToolWork *pWork)
        : m_pPageLyricsTool(pPageLyricsTool) { m_pWork = pWork; }

    void run()
    {
        m_pPageLyricsTool->doWork(m_pWork);
    }

protected:
    ILyricsToolWork            *m_pWork;
    CPageLyricsTool            *m_pPageLyricsTool;

};

bool CPageLyricsTool::onCommand(int nId)
{
    if (m_workStatus.isWorking() && nId != IDC_ABOUT && nId != IDC_ABOUT
         && nId != IDC_ML_HELP && nId != IDC_REGISTER && nId != IDC_WEBHOME)
         return true;

    switch (nId)
    {
    case IDC_IMPORT_ITUNES_LIB:
        {
            if (m_pMediaLib)
            {
//                g_Player.clearPlaylist();
                delete m_pMediaLib;
                m_pMediaLib = nullptr;
            }
            m_filesListCtrl->setDataSource(nullptr);

            changeToMediaLib(MST_ITUNES);

            CRunnableUpdateiTunesMediaLib rUpdate(this);

            m_WorkLooper.post(&rUpdate);
            rUpdate.m_eventFinished.acquire();

            m_filesListCtrl->setDataSource(m_pMediaLib);

            if (rUpdate.m_bUpdateSuccessfully)
                updateNewMediaList();
            else
            {
                m_pSkin->messageOut(CStrPrintf("%s\n%s", 
                    _TLT("Failed to load iTunes Library."),
                    rUpdate.m_strErr.c_str()).c_str());
            }
        }
        break;
    case IDC_ADD_FILES:
        {
            changeToMediaLib(MST_FILES);

            // static cstr_t    szSupportedExtion = "All supported files\0*.mp3;*.mp2;*.mpa;\0MPEG Audio Files (*.MP3;*.MP2;*.MPA)\0*.mp3;*.mp2;*.mpa\0\0";
            CFileDlgExtFilter        strFileExt;
            strFileExt.addExtention(_TLT("All supported files"), "*.mp3;*.mp2;*.mpa;*.m4a");
            strFileExt.addExtention(_TLT("MPEG Audio Files (*.MP3;*.MP2;*.MPA)"), "*.mp3;*.mp2;*.mpa");
            strFileExt.addExtention(_TLT("AAC Files (*.M4A)"), "*.m4a");
            CFileOpenDlg    dlg(_TLT("add Media Files"), nullptr, strFileExt.c_str(), 0, true);
            if (dlg.doModal(m_pSkin) != IDOK)
                break;

            VecStrings    vFiles;
            dlg.getOpenFile(vFiles);

            CFileMediaSource *pFileMediaLib = (CFileMediaSource *)m_pMediaLib;
            pFileMediaLib->addFiles(vFiles);

            updateNewMediaList();
        }
        break;
    case IDC_ADD_FOLDER:
        {
            changeToMediaLib(MST_FILES);

            CFolderDialog    dlg;
            string            strFolder;
            dlg.setInitFolder(g_profile.getString("LastMediaDir", ""));
            if (dlg.doBrowse(m_pSkin) == IDOK)
            {
                strFolder = dlg.getFolder();

                g_profile.writeString("LastMediaDir", strFolder.c_str());

                CFileMediaSource *pFileMediaLib = (CFileMediaSource *)m_pMediaLib;
                pFileMediaLib->addFolder(strFolder.c_str(), true);

                updateNewMediaList();
            }
        }
        break;
    case IDC_CLEAR_LIST:
        if (m_pMediaLib)
        {
            delete m_pMediaLib;
            m_pMediaLib = nullptr;
            m_filesListCtrl->setDataSource(nullptr);
            m_filesListCtrl->notifyDataChanged();
        }
        break;
    case IDC_REMOVE_SELECTED:
        {
            vector<int>        vSelected;
            int                n = -1;
            while (1)
            {
                n = m_filesListCtrl->getNextSelectedItem(n);
                if (n == -1)
                    break;
                vSelected.push_back(n);
            }

            for (int i = (int)vSelected.size() - 1; i >= 0; i--)
                m_pMediaLib->removeItem(vSelected[i]);

            m_filesListCtrl->notifyDataChanged();
        }
        break;
    case IDC_DOWNLOAD_ALL_LYRICS:
        hideInstructions();
        onAdvDownloadAllLyrics();
        break;
    case IDC_REMOVE_ALL_LYRICS:
        hideInstructions();
        onAdvRemoveAllLyrics();
        break;
    case IDC_DOWNLOAD_ALL_LYR_FOR_IPOD:
        hideInstructions();
        onDownloadLyricsForiPod();
        break;
//     case IDC_SAVE_LYRICS_TO_MP3:
//         hideInstructions();
//         break;
    case IDC_REMOVE_LRC_IN_MP3:
        hideInstructions();
        onRemoveEmbeddedLyrics();
        break;
    case IDC_EXIT:
        m_pSkin->postDestroy();
        break;
    default:
        return false;
    }

    return true;
}

bool CPageLyricsTool::onCustomCommand(int nId)
{
    if (nId == CID_UPDATEPROGRESS)
        updateProgress();
    else if (nId == CMD_PLAYPAUSE)
    {
        hideInstructions();
        
        CMPAutoPtr<IMedia> media;
        if (g_Player.getIMPlayer()->getCurrentMedia(&media) != ERR_OK || media == nullptr)
        {
            playItem(0, false);
        }
        else
            return false;
    }
    else if (nId == CMD_NEXT || nId == CMD_PREVIOUS)
    {
        int media = m_nCurrentMedia + 1;
        if (nId == CMD_PREVIOUS)
            media = m_nCurrentMedia - 1;
        if (m_pMediaLib != nullptr && media > 0 && media < m_pMediaLib->getRowCount())
        {
            m_filesListCtrl->clearAllSelMark();
            m_pMediaLib->setRowSelectionState(media, true);
            m_filesListCtrl->makeSureRowVisible(media);
            m_filesListCtrl->invalidate();

            playItem(media, true);
            hideInstructions();
        }
    }
    else if (nId == CMD_IMPORT)
    {
        onCommand(IDC_IMPORT_ITUNES_LIB);

        if (m_pMediaLib->getRowCount() == 0)
            onCommand(IDC_ADD_FILES);
    }
    else if (nId == CMD_DL_LYRICS)
        onCommand(IDC_DOWNLOAD_ALL_LYR_FOR_IPOD);
    else if (nId == CID_CANCEL_WORK)
    {
        if (m_workStatus.isWorking())
            m_workStatus.cancel();
        else
            m_pSkin->startAnimation(getIDByName("CID_AM_HIDE_WORKING"));
    }
    else
        return CSkinContainer::onCustomCommand(nId);

    return true;
}

bool CPageLyricsTool::onOK()
{
    if (m_WorkLooper.isIdle())
    {
        CSkinContainer::onOK();
    }

    return false;
}

bool CPageLyricsTool::onCancel()
{
    if (!m_WorkLooper.isIdle())
    {
        // cancel.
        if (m_workStatus.isWorking())
            m_workStatus.cancel();

        m_pSkin->setUIObjectText(CID_STATUS, _TLT("Canceling is in progress, please wait..."), true);
    }

    return true;
}

void CPageLyricsTool::updateProgress()
{
    bool    bRedrawListCtrl = false;
    string    strMsg;

    {
        MutexAutolock lock(m_mutexMsg);
        strMsg = m_strMsg;
        m_strMsg.resize(0);

        if (m_strStatus.size())
        {
            m_pSkin->setUIObjectText(CID_STATUS, m_strStatus.c_str(), false);
            m_strStatus.clear();
        }
        if (m_nProgress != -1)
            m_pProgressCtrl->setScrollPos(m_nProgress, false);

        for (ListUpdateItem::iterator it = m_listUpdateItems.begin(); it != m_listUpdateItems.end(); ++it)
        {
            UpdateItem &item = *it;
            m_pMediaLib->setResult(item.n, item.strResult.c_str());
        }
        bRedrawListCtrl = m_listUpdateItems.size() > 0;
        m_listUpdateItems.clear();
    }

    if (bRedrawListCtrl)
        m_pSkin->invalidateRect();
    else
    {
        m_pSkin->invalidateUIObject(CID_STATUS);
        m_pProgressCtrl->invalidate();
    }

    if (m_nProgress == 100 || m_nProgress == -1)
    {
        // DONE, show message and hide working area.
        if (strMsg.size())
            m_pSkin->messageOut(strMsg.c_str());

        m_pSkin->setUIObjectText(CID_CANCEL_WORK, "OK", true);
    }
}

void CPageLyricsTool::clearResult()
{
    for (int i = 0; i < m_filesListCtrl->getRowCount(); i++)
    {
        m_pMediaLib->setItemImageIndex(i, IMG_NONE, false);
        m_pMediaLib->setResult(i, "");
    }
}

void CPageLyricsTool::fromCurrentPlaylist()
{
    CMPAutoPtr<IPlaylist> playlist;

    MLRESULT ret = g_Player.getCurrentPlaylist(&playlist);
    if (ret != ERR_OK)
        return;

    int count = playlist->getCount();
    if (count == 0)
        return;

    changeToMediaLib(MST_FILES);

    for (int i = 0; i < count; i++)
    {
        CMPAutoPtr<IMedia> media;
        string location;

        ret = playlist->getItem(i, &media);
        if (ret != ERR_OK)
            return;

        CXStr url;
        if (media->getSourceUrl(&url) != ERR_OK)
            return;

        CFileMediaSource *pFileMediaLib = (CFileMediaSource *)m_pMediaLib;
        pFileMediaLib->addFile(url.c_str());
    }
}

void CPageLyricsTool::updateNewMediaList()
{
    if (!m_pMediaLib)
        return;

    CUIObject *pObjInst = m_pSkin->getUIObjectById(UID_INSTRUCTION);
    if (pObjInst != nullptr && pObjInst->isVisible())
    {
        pObjInst->setProperty(SZ_PN_TEXT, _TLM("Please click here to download all lyrics."));
        pObjInst->setProperty(SZ_PN_LINK, "cmd://CMD_DL_LYRICS");
        pObjInst->getParent()->recalculateUIObjSizePos(pObjInst);
    }

    m_filesListCtrl->notifyDataChanged();
}

void CPageLyricsTool::onAdvDownloadAllLyrics()
{
//    clearResult();
//
//    if (!m_pMediaLib || m_pMediaLib->getRowCount() == 0)
//    {
//        m_pSkin->messageOut(_TLT("Please add media files in the list."));
//        return;
//    }
//
//    CDlgAdvDownloadPrompt    dlg;
//
//    if (dlg.doModal(m_pSkin) != IDOK)
//        return;
//
//    m_pSkin->setUIObjectText(CID_STATUS, _TLT("Downloading lyrics is in progress..."), true);
//
//    CAdvDownloadLyrWork *pWork = new CAdvDownloadLyrWork(m_pMediaLib, this, m_workStatus);
//    pWork->setInfo(dlg.m_downSaveDir, dlg.m_downSaveName, dlg.m_bOnlyUseLocalLyrics, dlg.m_vSaveEmbeddedLyrNames);
//    postWork(pWork);
}

void CPageLyricsTool::onAdvRemoveAllLyrics()
{
//    clearResult();
//
//    if (!m_pMediaLib || m_pMediaLib->getRowCount() == 0)
//    {
//        m_pSkin->messageOut(_TLT("Please add media files in the list."));
//        return;
//    }
//
//    CDlgAdvRemovePrompt    dlg;
//
//    if (dlg.doModal(m_pSkin) != IDOK)
//        return;
//
//    int nRet = m_pSkin->messageOut(_TLT("Are you sure that you want to remove the embedded lyrics of all the files?"), MB_ICONQUESTION | MB_OKCANCEL);
//    if (nRet != IDOK)
//        return;
//
//    m_pSkin->setUIObjectText(CID_STATUS, _TLT("Removing embedded lyrics in media files is in progress..."), true);
//
//    CAdvRemoveLyrics *pWork = new CAdvRemoveLyrics(m_pMediaLib, this, m_workStatus);
//    pWork->setInfo(dlg.m_bRemoveAssociateLyrFile, dlg.m_vEmbeddedLyrNames);
//    postWork(pWork);
}

void CPageLyricsTool::onDownloadLyricsForiPod()
{
    clearResult();

    if (!m_pMediaLib || m_pMediaLib->getRowCount() == 0)
    {
        m_pSkin->messageOut(_TLT("Please add media files in the list."));
        return;
    }

    m_pSkin->setUIObjectText(CID_STATUS, _TLT("Downloading lyrics for iPod is in progress..."), true);

    postWork(new CSaveLyricsInITunes(m_pMediaLib, this, m_workStatus));
}

void CPageLyricsTool::onRemoveEmbeddedLyrics()
{
    clearResult();

    if (!m_pMediaLib || m_pMediaLib->getRowCount() == 0)
    {
        m_pSkin->messageOut(_TLT("Please add media files in the list."));
        return;
    }

    int nRet = m_pSkin->messageOut(_TLT("Are you sure that you want to remove the embedded lyrics of all the files?"), MB_ICONQUESTION | MB_OKCANCEL);
    if (nRet != IDOK)
        return;

    m_pSkin->setUIObjectText(CID_STATUS, _TLT("Removing embedded lyrics in media files is in progress..."), true);

    postWork(new CRemoveEmbeddedLyrics(m_pMediaLib, this, m_workStatus));
}

void CPageLyricsTool::postWork(ILyricsToolWork *pWork)
{
    if (!m_WorkLooper.isIdle())
    {
        DBG_LOG0("Working looper is NOT idle now. Can't post work.");
        return;
    }

    m_pProgressCtrl->setScrollPos(0, false);

    m_pSkin->setUIObjectText(CID_CANCEL_WORK, "cancel", false);
    m_pSkin->startAnimation(getIDByName("CID_AM_SHOW_WORKING"));

    CRunnableDoWork    *pRunnable = new CRunnableDoWork(this, pWork);
    m_WorkLooper.postFreeAfterRun(pRunnable);
}

void CPageLyricsTool::doWork(ILyricsToolWork *pWork)
{
    if (!m_pMediaLib)
        return;

    m_nProgress = 0;

    pWork->onBegin();
    int nCount = m_pMediaLib->getRowCount();
    for (int i = 0; i < nCount; i++)
    {
        if (m_workStatus.isCancel())
            break;

        pWork->onProcessItem(i);

        m_nProgress = i * 100 / nCount;
        if (m_nProgress != 100)
            m_pSkin->postCustomCommandMsg(CID_UPDATEPROGRESS);
    }

    string    strMsg;
    pWork->onEnd(strMsg);

    m_nProgress = 100;

    {
        MutexAutolock lock(m_mutexMsg);
        m_strStatus = _TLT("Finished.");
        m_strMsg = strMsg;
        if (pWork)
        {
            delete pWork;
            pWork = nullptr;
        }
    }

    m_pSkin->postCustomCommandMsg(CID_UPDATEPROGRESS);
}

uint32_t CPageLyricsTool::getEmbeddedLyricsOpt()
{
    uint32_t        uEmbeddedLyricsTypes = 0;

    if (isButtonChecked("CID_C_ID3V2_SYLT"))
        uEmbeddedLyricsTypes |= LST_ID3V2_SYLT;
    if (isButtonChecked("CID_C_ID3V2_USLT"))
        uEmbeddedLyricsTypes |= LST_ID3V2_USLT;
    if (isButtonChecked("CID_C_LYR3V2"))
        uEmbeddedLyricsTypes |= LST_LYRICS3V2;

    return uEmbeddedLyricsTypes;
}

void registerLyricsToolPage(CSkinFactory *pSkinFactory)
{
    AddUIObjNewer2(pSkinFactory, CPageLyricsTool);
}
