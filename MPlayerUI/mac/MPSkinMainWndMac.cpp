#include "MPlayerApp.h"
#include "MPSkinMainWnd.h"

#ifdef _WIN32
#include "MPMsg.h"
#endif

#ifdef _MPLAYER
#include "MPHelper.h"
#endif


CMPSkinMainWnd::CMPSkinMainWnd() {
}

CMPSkinMainWnd::~CMPSkinMainWnd() {
}

void CMPSkinMainWnd::onCreate() {
    CMPSkinMainWndBase::onCreate();

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_CUR_MEDIA_INFO_CHANGED);

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
        char szCaption[512];

        if (g_Player.isMediaOpened()) {
            snprintf(szCaption, CountOf(szCaption), "%s - %s", g_Player.getFullTitle(), SZ_APP_NAME);
        } else {
            strcpy_safe(szCaption, CountOf(szCaption), getAppNameLong().c_str());
        }

        setTitle(szCaption);

        m_mlTrayIcon.updateTrayIconText(szCaption);
    } else if (pEvent->eventType == ET_UI_SETTINGS_CHANGED) {
        if (isPropertyName(pEvent->name.c_str(), "ShowIconOn")) {
            m_mlTrayIcon.updateShowIconPos();
        } else {
            CMPSkinMainWndBase::onEvent(pEvent);
        }
    } else {
        CMPSkinMainWndBase::onEvent(pEvent);
    }
}
