

#include "../MPlayerApp.h"
#include "MPMsgWnd.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "OnlineSearch.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "MPFloatingLyrWnd.h"
#include "MPlayerAppGtk2.h"


int main(int argc, char *argv[]) {
    if (!MPlayerApp::getInstance()->init(argc, argv)) {
        return 0;
    }

    gtk_main();

    MPlayerApp::getInstance()->quit();

    return 0;
}


MPlayerApp::MPlayerApp() {
}

MPlayerApp::~MPlayerApp() {
}

bool MPlayerApp::init(int argc, char *argv[]) {

    char szWorkingFolder[MAX_PATH];
    string strMLIniFile, strMLLogFile;

    //
    // init Base frame, save log and settings in app dir
    strMLIniFile = getAppDataDir();
    strMLIniFile += SZ_PROFILE_NAME;
    setFileNoReadOnly(strMLIniFile.c_str());

    if (!isFileExist(strMLIniFile.c_str())) {
        g_bEngEdition = true;
    }

    g_log.setSrcRootDir(__FILE__, 2);

    strMLLogFile = getAppDataDir();
    strMLLogFile += "log.txt";
    initBaseFrameWork(argc, argv, strMLLogFile.c_str(), strMLIniFile.c_str(), SZ_SECT_UI);

    MLWidgetInit(argc, argv);

    int nRet = CResourceMgr::load("resource.xml");
    if (nRet != ERR_OK) {
        ERR_LOG1("load resouce Error: %s", (cstr_t)Error2Str(nRet));
        return false;
    }

#if (defined _WIN32 && defined _DEBUG_OUTPUT)
    DBG_LOG1("Working Folder: %s", getAppResourceDir());
    DBG_LOG1("App Data Folder: %s", getAppDataDir());
#endif

    LOG1(LOG_LVL_EVENT_IMP, "start %s", getAppNameLong().c_str());

    {
        //
        // 检查版本
        //
#ifdef _DEBUG
        CProfile file;
        string strEng;
        file.init("install.ini", "INSTALL");
        strEng = file.getString("Edition", "");
        if (strcmp(strEng.c_str(), "english") == 0) {
            g_bEngEdition = true;
        } else if (strcmp(strEng.c_str(), "chinese") == 0) {
            g_bEngEdition = false;
        } else {
            #endif
        }
        {
            // 根据 location 来设置中英文版本差别
        }
    }

    return _init();
}

void MPlayerApp::quit() {
    _quit();

    gtk_main_quit();
}
