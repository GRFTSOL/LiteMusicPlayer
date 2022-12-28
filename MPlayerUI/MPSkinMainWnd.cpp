#include "MPlayerApp.h"
#include "MPSkinMainWnd.h"
#include "DlgAbout.h"
#include "VersionUpdate.h"


//////////////////////////////////////////////////////////////////////

CMPSkinMainWndBase::CMPSkinMainWndBase() {
}

CMPSkinMainWndBase::~CMPSkinMainWndBase() {
}

void CMPSkinMainWndBase::onCreate() {
    CMPSkinWnd::onCreate();

    updateCaptionText();
}

void CMPSkinMainWndBase::onDestroy() {
    IEventHandler::unregisterHandler();

#ifdef _WIN32
    // save iconic status
    g_profile.writeInt("Minimized", isIconic());
#endif

    CMPSkinWnd::onDestroy();
}

void CMPSkinMainWndBase::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_UI_SETTINGS_CHANGED) {
        if (isPropertyName(pEvent->name.c_str(), "topmost")) {
            CMPlayerAppBase::getMPSkinFactory()->topmostAll(isTRUE(pEvent->strValue.c_str()));
        } else {
            CMPSkinWnd::onEvent(pEvent);
        }
    } else {
        CMPSkinWnd::onEvent(pEvent);
    }
}

void CMPSkinMainWndBase::updateCaptionText() {
    setCaptionText(_TL(SZ_APP_NAME));
}
