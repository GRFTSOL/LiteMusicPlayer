#include "MPlayerApp.h"
#include "PreferPageLyrics.h"
#include "DownloadMgr.h"

#ifdef _WIN32
#include "win32/LyricsOutPluginMgr.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/LyricsOutPluginMgr.h"
#endif

#ifdef _MAC_OS
#include "mac/LyricsOutPluginMgr.h"


#endif


class CPagePfLyrDownload : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfLyrDownload() : CPagePfBase(PAGE_LYR_DOWNLOAD, "CMD_LYR_DOWNLOAD") {
        CID_DOWN_DIR = 0;
        CID_C_SAVE_IN_SONG_DIR = 0;
        CID_C_SAVE_IN_DIR = 0;
        CID_C_SAVE_IN_SONG_FILE = 0;
        CID_C_ID3V2_SYLT = 0;
        CID_C_ID3V2_USLT = 0;
        CID_C_LYR3V2 = 0;
        CID_C_M4A_LYRICS = 0;
    }

    void onInitialUpdate() override {
        CPagePfBase::onInitialUpdate();

        GET_ID_BY_NAME(CID_DOWN_DIR);
        GET_ID_BY_NAME(CID_C_SAVE_IN_SONG_DIR);
        GET_ID_BY_NAME(CID_C_SAVE_IN_DIR);
        GET_ID_BY_NAME2(CID_C_SAVE_IN_SONG_FILE, CID_C_M4A_LYRICS);
        GET_ID_BY_NAME(CID_C_ID3V2_SYLT);
        GET_ID_BY_NAME(CID_C_ID3V2_USLT);
        GET_ID_BY_NAME(CID_C_LYR3V2);

        addOptBool(ET_NULL, SZ_SECT_LYR_DL, "DownSaveId3v2Sylt", true, "CID_C_ID3V2_SYLT");
        addOptBool(ET_NULL, SZ_SECT_LYR_DL, "DownSaveId3v2Uslt", true, "CID_C_ID3V2_USLT");
        addOptBool(ET_NULL, SZ_SECT_LYR_DL, "DownSaveLyrics3v2", false, "CID_C_LYR3V2");
        addOptBool(ET_NULL, SZ_SECT_LYR_DL, "DownSaveLyricsM4a", false, "CID_C_M4A_LYRICS");

        {
            // downloaded lyrics file name
            OptRadioInt opt;
            opt.set(ET_NULL, SZ_SECT_LYR_DL, "DownSaveName", DOWN_SAVE_NAME_KEEP);
            opt.addCtrlValue(getIDByName("CID_C_AS_SONG_NAME"), DOWN_SAVE_NAME_AS_SONG_NAME);
            opt.addCtrlValue(getIDByName("CID_C_KEEP_FILENAME"), DOWN_SAVE_NAME_KEEP);
            addOptRadioInt(opt);
        }

        //
        // 设置下载路径的显示
        string strDownSaveDir = g_LyricsDownloader.getDefSavePath();
        setUIObjectText(CID_DOWN_DIR, strDownSaveDir.c_str());

        // 需要保存的歌词目录
        DOWN_SAVE_DIR DownSaveDir;
        DownSaveDir = (DOWN_SAVE_DIR)g_profile.getInt(SZ_SECT_LYR_DL, "DownSaveInSongDir", DOWN_SAVE_IN_CUSTOM_DIR);
        if (DownSaveDir == DOWN_SAVE_IN_SONG_DIR) {
            enableUIObject(CID_DOWN_DIR, false, false);
            checkButton(CID_C_SAVE_IN_SONG_DIR, true);
        } else if (DownSaveDir == DOWN_SAVE_IN_CUSTOM_DIR) {
            enableUIObject(CID_DOWN_DIR, true, false);
            checkButton(CID_C_SAVE_IN_DIR, true);
        } else {
            enableUIObject(CID_DOWN_DIR, false, false);
        }

        if (g_profile.getBool(SZ_SECT_LYR_DL, "DownSaveEmbeded", false)) {
            checkButton("CID_C_SAVE_IN_SONG_FILE", true);
        } else {
            enableUIObject(CID_C_M4A_LYRICS, false, false);
            enableUIObject(CID_C_ID3V2_SYLT, false, false);
            enableUIObject(CID_C_ID3V2_USLT, false, false);
            enableUIObject(CID_C_LYR3V2, false, false);
        }

        initCheckButtons();
    }

    bool onCustomCommand(int nId)override {
        if (nId == CID_C_SAVE_IN_SONG_DIR
            || nId == CID_C_SAVE_IN_DIR) {
            DOWN_SAVE_DIR DownSaveDir;
            if (!isButtonChecked(nId)) {
                DownSaveDir = DOWN_SAVE_NO_FILE;
            } else {
                if (nId == CID_C_SAVE_IN_SONG_DIR) {
                    DownSaveDir = DOWN_SAVE_IN_SONG_DIR;
                    checkButton(CID_C_SAVE_IN_DIR, false);
                } else {
                    DownSaveDir = DOWN_SAVE_IN_CUSTOM_DIR;
                    checkButton(CID_C_SAVE_IN_SONG_DIR, false);
                }
            }

            enableUIObject(CID_DOWN_DIR, DownSaveDir == DOWN_SAVE_IN_CUSTOM_DIR);

            g_profile.writeInt(SZ_SECT_LYR_DL, "DownSaveInSongDir", DownSaveDir);
        } else if (nId == CID_C_SAVE_IN_SONG_FILE) {
            bool bCheck;
            bCheck = isButtonChecked(nId);

            g_profile.writeInt(SZ_SECT_LYR_DL, "DownSaveEmbeded", bCheck);
            enableUIObject(CID_C_M4A_LYRICS, bCheck);
            enableUIObject(CID_C_ID3V2_SYLT, bCheck);
            enableUIObject(CID_C_ID3V2_USLT, bCheck);
            enableUIObject(CID_C_LYR3V2, bCheck);
        } else if (nId == CID_DOWN_DIR) {
            CFolderDialog dlg;
            string strDownSaveDir;

            strDownSaveDir = g_LyricsDownloader.getDefSavePath();

            dlg.setInitFolder(strDownSaveDir.c_str());
            if (dlg.doBrowse(m_pSkin)) {
                if (strcasecmp(strDownSaveDir.c_str(), dlg.getFolder()) != 0) {
                    strDownSaveDir = dlg.getFolder();
                    dirStringAddSep(strDownSaveDir);

                    if (!isDirWritable(strDownSaveDir.c_str())) {
                        m_pSkin->messageOut("Can't save lyrics in the selected folder.");
                    } else {
                        CMLProfile::writeDir(SZ_SECT_LYR_DL, "LyricsDownPath", strDownSaveDir.c_str());
                        g_LyricsDownloader.setDefSavePath(strDownSaveDir.c_str());
                    }

                    setUIObjectText(nId, strDownSaveDir.c_str());
                }
            }
        } else {
            return CPagePfBase::onCustomCommand(nId);
        }

        return true;
    }

protected:
    int                         CID_DOWN_DIR, CID_C_SAVE_IN_SONG_DIR, CID_C_SAVE_IN_DIR;
    int                         CID_C_SAVE_IN_SONG_FILE, CID_C_M4A_LYRICS;
    int                         CID_C_ID3V2_SYLT, CID_C_ID3V2_USLT, CID_C_LYR3V2;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfLyrDownload, "PreferPage.LyrDownload")

//////////////////////////////////////////////////////////////////////////

class CPagePfLyrSaveOpt : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfLyrSaveOpt() : CPagePfBase(PAGE_LYR_SAVE_OPTIONS, "CMD_LYR_SAVE_OPT") {
    }

    void onInitialUpdate() override {
        CPagePfBase::onInitialUpdate();

        addOptBool(ET_NULL, SZ_SECT_UI, "RemoveExtraBlankLines", true, "CID_C_REMOVE_EXTRA_BLANK_LINES");

        initCheckButtons();
    }

};

UIOBJECT_CLASS_NAME_IMP(CPagePfLyrSaveOpt, "PreferPage.LyrSaveOptions")

//////////////////////////////////////////////////////////////////////////

class CPagePfLyrOutputPlugin : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfLyrOutputPlugin() : CPagePfBase(PAGE_LYR_OUTPUT_PLUGINS, "CMD_LYR_OUTPUT_PLUGIN") {
        m_pListPlugins = nullptr;
        CID_LO_PLUGIN_LIST = 0;
    }

    void onInitialUpdate() override {
        CPagePfBase::onInitialUpdate();

        GET_ID_BY_NAME(CID_LO_PLUGIN_LIST);

        m_pSkin->setUIObjectProperty("CID_GET_PLUG", SZ_PN_LINK, getStrName(SN_HTTP_DLPLUGIN));

        m_pSkin->registerUIObjNotifyHandler(CID_LO_PLUGIN_LIST, this);

        m_pListPlugins = (CSkinListCtrl *)getUIObjectById(CID_LO_PLUGIN_LIST, CSkinListCtrl::className());
        if (!m_pListPlugins) {
            return;
        }

        m_pListPlugins->addColumn("Plug-ins", 100);

        vector<string> vPlugins;
        g_lyrOutPlguinMgr.getLoadedPlugins(vPlugins);
        for (int i = 0; i < (int)vPlugins.size(); i++) {
            m_pListPlugins->insertItem(i, vPlugins[i].c_str());
        }

        enablePluginBtns();
    }

    bool onCustomCommand(int nId) override {
        if (!m_pListPlugins) {
            return CPagePfBase::onCustomCommand(nId);
        }

        if (nId == getIDByName("CID_UNINST_PLUG")) {
            int n = m_pListPlugins->getNextSelectedItem();
            if (n != -1) {
                g_lyrOutPlguinMgr.uninstPlugin(n, m_pSkin);
            }
        } else if (nId == getIDByName("CID_CONFIGURE")) {
            int n = m_pListPlugins->getNextSelectedItem();
            if (n != -1) {
                g_lyrOutPlguinMgr.configurePlugin(n, m_pSkin);
            }
        } else if (nId == getIDByName("CID_ABOUT")) {
            int n = m_pListPlugins->getNextSelectedItem();
            if (n != -1) {
                g_lyrOutPlguinMgr.aboutPlugin(n, m_pSkin);
            }
        } else {
            return CPagePfBase::onCustomCommand(nId);
        }

        return true;
    }

    void onUIObjNotify(IUIObjNotify *pNotify) override {
        if (pNotify->nID == CID_LO_PLUGIN_LIST && pNotify->isKindOf(CSkinListCtrl::className())) {
            CSkinListCtrlEventNotify *pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
            if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_SEL_CHANGED) {
                enablePluginBtns();
            }
        }
    }

protected:
    void enablePluginBtns() {
        assert(m_pListPlugins);
        bool bEnable = (m_pListPlugins->getNextSelectedItem() != -1);

        enableUIObject("CID_CONFIGURE", bEnable);
        enableUIObject("CID_ABOUT", bEnable);
        enableUIObject("CID_UNINST_PLUG", bEnable);
    }

    CSkinListCtrl               *m_pListPlugins;
    int                         CID_LO_PLUGIN_LIST;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfLyrOutputPlugin, "PreferPage.LyrOutputPlugin")

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfLyricsRoot, "PreferPage.LyricsRoot")

CPagePfLyricsRoot::CPagePfLyricsRoot() : CPagePfBase(PAGE_UNKNOWN, "CMD_ROOT_LYRICS") {
}

void CPagePfLyricsRoot::onInitialUpdate() {
    CPagePfBase::onInitialUpdate();

    checkToolbarDefaultPage("CID_TOOLBAR_LYR");
}

void registerPfLyricsPages(CSkinFactory *pSkinFactory) {
    AddUIObjNewer2(pSkinFactory, CPagePfLyrDownload);
    AddUIObjNewer2(pSkinFactory, CPagePfLyrSaveOpt);
    AddUIObjNewer2(pSkinFactory, CPagePfLyrOutputPlugin);
    AddUIObjNewer2(pSkinFactory, CPagePfLyricsRoot);
}
