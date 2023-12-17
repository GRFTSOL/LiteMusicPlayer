#include "../MPlayerApp.h"
#include "../MPSkinMainWnd.h"

#ifdef _WIN32
#include "MPMsg.h"
#endif


CMPSkinMainWnd::CMPSkinMainWnd() {
}

CMPSkinMainWnd::~CMPSkinMainWnd() {
}

void CMPSkinMainWnd::onCreate() {
    CMPSkinMainWndBase::onCreate();

    registerHandler(MPlayerApp::getEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_CUR_MEDIA_INFO_CHANGED, ET_PLAYER_STATUS_CHANGED);

    m_mlTrayIcon.init(this);
}

void CMPSkinMainWnd::onDestroy() {
    m_mlTrayIcon.quit();

    CMPSkinMainWndBase::onDestroy();
}

void CMPSkinMainWnd::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED
        || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED) {
        // update main window caption
        string caption;

        if (g_player.isMediaOpened()) {
            caption = stringPrintf("%s - %s", g_player.getFullTitle(), SZ_APP_NAME);
        } else {
            caption = getAppNameLong();
        }

        setTitle(caption.c_str());
        m_mlTrayIcon.updateTrayIconText(caption.c_str());
    } else if (pEvent->eventType == ET_UI_SETTINGS_CHANGED) {
        if (isPropertyName(pEvent->name.c_str(), "ShowIconOn")) {
            m_mlTrayIcon.updateShowIconPos();
        } else {
            CMPSkinMainWndBase::onEvent(pEvent);
        }
    } else {
        CMPSkinMainWndBase::onEvent(pEvent);
    }

    if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED || pEvent->eventType == ET_PLAYER_STATUS_CHANGED) {
        m_mlTrayIcon.updatePlayerSysTrayIcon();
    }
}
