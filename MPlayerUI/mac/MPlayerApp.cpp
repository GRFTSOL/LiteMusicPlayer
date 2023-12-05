#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "OnlineSearch.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "MPFloatingLyrWnd.h"
#include "LyricShowObj.h"
#include "HeadPhoneWatch.hpp"


MPlayerApp::MPlayerApp() {
    m_bRunning = false;
}

MPlayerApp::~MPlayerApp() {
}

bool MPlayerApp::isAnotherInstanceRunning() {
    return false;
}

bool MPlayerApp::setRunningFlag() {
    // Is Product already running?
    m_bRunning = true;

    return true;
}

bool MPlayerApp::init() {
    g_log.setSrcRootDir(__FILE__, 2);

    g_LangTool.setMacro(SZ_MACRO_PRODUCT_NAME, SZ_APP_NAME);
    g_LangTool.setMacro(SZ_MACRO_COMPANY_NAME, SZ_COMPANY_NAME);

    if (!_init()) {
        return false;
    }

    setupHeadPhonePlugWatch();

    return true;
}

void MPlayerApp::quit() {
    if (!m_bRunning) {
        return;
    }

    m_bRunning = false;

    _quit();
}
