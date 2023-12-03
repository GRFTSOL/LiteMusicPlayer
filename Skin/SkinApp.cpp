#include "SkinTypes.h"
#include "Skin.h"
#include "SkinApp.h"
#include "api-js/SkinJsAPI.hpp"


//////////////////////////////////////////////////////////////////////////

CSkinApp * CSkinApp::m_pInstance = nullptr;

CSkinApp::CSkinApp(void) {
}

CSkinApp::~CSkinApp(void) {
}

CSkinApp *CSkinApp::getInstance() {
    assert(m_pInstance);
    return m_pInstance;
}

bool CSkinApp::init() {
    m_pEventDispatcher = newEventPatcher();
    m_pSkinFactory = newSkinFactory();

    if (!isDirExist(m_pSkinFactory->getSkinRootDir())) {
        // Sometimes, uninstallation can't be remove MiniLyrics.dll till next restart,
        // So, don't start MiniLyrics, after uninstallation.
        ERR_LOG1("Skin root folder: %s doesn't exist.", m_pSkinFactory->getSkinRootDir());
        return false;
    }

    // init base service
    m_pEventDispatcher->init();

    m_pSkinFactory->init();

    initSkinJsAPIs();

    return true;
}

int CSkinApp::loadDefaultSkin(cstr_t szDefaultSkin, bool bCreateSkinWnd) {
    bool bLoadSkinTheme = false;

    string strSkin = getDefaultSkin();
    if (strSkin.empty()) {
        strSkin = szDefaultSkin;
        bLoadSkinTheme = true;
        writeDefaultSkin(szDefaultSkin);
    }

    return m_pSkinFactory->changeSkin(strSkin.c_str(), "", "", bLoadSkinTheme, bCreateSkinWnd);
}

string CSkinApp::getDefaultSkin() {
    return g_profile.getString(m_pSkinFactory->getSkinFileName(), "DefaultSkin", "");
}

void CSkinApp::writeDefaultSkin(cstr_t szDefaultSkin) {
    g_profile.writeString(m_pSkinFactory->getSkinFileName(), "DefaultSkin", szDefaultSkin);
}

template <class _T>
void safeDelete(_T &p) {
    if (p) {
        delete p;
        p = nullptr;
    }
}

void CSkinApp::quit() {
    m_pEventDispatcher->quit();
    m_pSkinFactory->quit();

    safeDelete(m_pEventDispatcher);
    safeDelete(m_pSkinFactory);
}

void CSkinApp::postQuitMessage() {
#ifdef _WIN32
    ::postQuitMessage(0);
#else
    postQuitMessageMac();
#endif
}

int CSkinApp::showDialog(Window *pWndParent, cstr_t szDialogName) {
    SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME,
        szDialogName, pWndParent);

    return getSkinFactory()->activeOrCreateSkinWnd(skinWndStartupInfo);
}

CEventsDispatcher *CSkinApp::newEventPatcher() {
    return new CEventsDispatcher();
}

CSkinFactory *CSkinApp::newSkinFactory() {
    return new CSkinFactory(this, nullptr);
}
