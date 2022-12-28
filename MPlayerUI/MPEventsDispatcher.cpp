

#include "MPlayerApp.h"
#include "MPEventsDispatcher.h"
#ifdef _MPLAYER
#include "MPVisAdapter.h"


#endif

//////////////////////////////////////////////////////////////////////////

CMPEventsDispatcher::CMPEventsDispatcher() {
#ifdef _MPLAYER
    m_pVisAdapter = nullptr;
#endif
}

void CMPEventsDispatcher::quit() {
    CEventsDispatcher::quit();

#ifdef _MPLAYER
    if (m_pVisAdapter) {
        g_Player.unregisterVis(m_pVisAdapter);
        m_pVisAdapter->release();
        m_pVisAdapter = nullptr;
    }
#endif
}

void CMPEventsDispatcher::registerHandler(EventType eventType, IEventHandler *pHandler) {
    CEventsDispatcher::registerHandler(eventType, pHandler);

#ifdef _MPLAYER
    if (eventType == ET_VIS_DRAW_UPDATE) {
        MutexAutolock lock(m_mutex);
        LIST_EVENTHANDLER &listHandler = m_vListEventHandler[eventType];

        // register vis ?
        if (listHandler.size() == 1) {
            assert(m_pVisAdapter == nullptr);
            m_pVisAdapter = new CMPVisAdapter;
            m_pVisAdapter->addRef();
            g_Player.registerVis(m_pVisAdapter);
        }
    }
#endif
}

void CMPEventsDispatcher::unRegisterHandler(EventType eventType, IEventHandler *pHandler) {
    CEventsDispatcher::unRegisterHandler(eventType, pHandler);

#ifdef _MPLAYER
    if (eventType == ET_VIS_DRAW_UPDATE) {
        MutexAutolock lock(m_mutex);
        LIST_EVENTHANDLER &listHandler = m_vListEventHandler[eventType];

        // unregister vis ?
        if (listHandler.size() == 0) {
            assert(m_pVisAdapter != nullptr);
            g_Player.unregisterVis(m_pVisAdapter);
            m_pVisAdapter->release();
            m_pVisAdapter = nullptr;
        }
    }
#endif
}

// szValue: p1="v1" p2="v2"
//    or        v1
void CMPlayerSettings::setSettings(EventType eventType, cstr_t szSectionName, cstr_t szSettingName, cstr_t szValue, bool bNotify) {
    // save setting
    g_profile.writeString(szSectionName, szSettingName, szValue);

    if (!bNotify) {
        return;
    }

    // dispatch setting event
    IEvent *pEvt = new IEvent;

    pEvt->eventType = eventType;
    pEvt->name = szSettingName;
    pEvt->strValue = szValue;

    CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(pEvt);
}

void CMPlayerSettings::setSettings(EventType eventType, cstr_t szSectionName, cstr_t szSettingName, int value, bool bNotify) {
    // save setting
    g_profile.writeInt(szSectionName, szSettingName, value);

    if (!bNotify) {
        return;
    }

    // dispatch setting event
    IEvent *pEvt = new IEvent;

    pEvt->eventType = eventType;
    pEvt->name = szSettingName;
    pEvt->strValue = stringFromInt(value);

    CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(pEvt);
}
