

#include "MPlayerApp.h"
#include "MPEventsDispatcher.h"
#ifdef _MPLAYER
// #include "MPVisAdapter.h"
#endif

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
