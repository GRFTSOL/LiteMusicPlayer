#include "SkinTypes.h"
#include "EventsDispatcherBase.h"


#define MAX_EVENT_ID        100

IEventHandler::~IEventHandler() {
    if (m_pEventDispatcher) {
        m_pEventDispatcher->unRegisterHandler(this);
    }
}

void IEventHandler::registerHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1) {
    m_pEventDispatcher = pEventDispatcher;
    m_pEventDispatcher->registerHandler(ev1, this);
}

void IEventHandler::registerHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1, EventType ev2) {
    m_pEventDispatcher = pEventDispatcher;
    m_pEventDispatcher->registerHandler(ev1, this);
    m_pEventDispatcher->registerHandler(ev2, this);
}

void IEventHandler::registerHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1, EventType ev2, EventType ev3) {
    m_pEventDispatcher = pEventDispatcher;
    m_pEventDispatcher->registerHandler(ev1, this);
    m_pEventDispatcher->registerHandler(ev2, this);
    m_pEventDispatcher->registerHandler(ev3, this);
}

void IEventHandler::registerHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1, EventType ev2, EventType ev3, EventType ev4) {
    m_pEventDispatcher = pEventDispatcher;
    m_pEventDispatcher->registerHandler(ev1, this);
    m_pEventDispatcher->registerHandler(ev2, this);
    m_pEventDispatcher->registerHandler(ev3, this);
    m_pEventDispatcher->registerHandler(ev4, this);
}

void IEventHandler::unregisterHandler() {
    if (m_pEventDispatcher) {
        m_pEventDispatcher->unRegisterHandler(this);
        m_pEventDispatcher = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////

CEventsDispatcherBase::CEventsDispatcherBase() {

}

CEventsDispatcherBase::~CEventsDispatcherBase() {

}

int CEventsDispatcherBase::init() {
    return ERR_OK;
}

void CEventsDispatcherBase::quit() {
    MutexAutolock autoLock(m_mutex);

    m_vListEventHandler.clear();
}

void CEventsDispatcherBase::registerHandler(EventType eventType, IEventHandler *pHandler) {
    assert(eventType >= 0 && eventType <= MAX_EVENT_ID);
    MutexAutolock autoLock(m_mutex);

    if (eventType >= m_vListEventHandler.size()) {
        m_vListEventHandler.resize(eventType + 1);
    }

    ListEventHandlers &listHandler = m_vListEventHandler[eventType];
    ListEventHandlers::iterator it, itEnd;

    itEnd = listHandler.end();
    for (it = listHandler.begin(); it != itEnd; ++it) {
        if (*it == pHandler) {
            return;
        }
    }
    listHandler.push_back(pHandler);
}

void CEventsDispatcherBase::unRegisterHandler(EventType eventType, IEventHandler *pHandler) {
    assert(eventType >= 0 && eventType <= MAX_EVENT_ID);
    if (eventType >= m_vListEventHandler.size()) {
        return;
    }

    MutexAutolock autoLock(m_mutex);
    ListEventHandlers &listHandler = m_vListEventHandler[eventType];
    ListEventHandlers::iterator it, itEnd;

    itEnd = listHandler.end();
    for (it = listHandler.begin(); it != itEnd; ++it) {
        if (*it == pHandler) {
            listHandler.erase(it);
            return;
        }
    }
}

void CEventsDispatcherBase::unRegisterHandler(IEventHandler *pHandler) {
    MutexAutolock autoLock(m_mutex);

    for (size_t i = 0; i < m_vListEventHandler.size(); i++) {
        ListEventHandlers &listHandler = m_vListEventHandler[i];
        ListEventHandlers::iterator it, itEnd;

        itEnd = listHandler.end();
        for (it = listHandler.begin(); it != itEnd; ++it) {
            if (*it == pHandler) {
                listHandler.erase(it);
                break;
            }
        }
    }
}

void CEventsDispatcherBase::dispatchSyncEvent(IEvent *pEvent) {
    EventType eventType = pEvent->eventType;

    if (eventType >= m_vListEventHandler.size()) {
        DBG_LOG1("No Event Handler for: %d", (int)eventType);
        // assert(0 && "No handler for this event");
        delete pEvent;
        return;
    }

    m_mutex.lock();
    ListEventHandlers listHandler = m_vListEventHandler[eventType];
    m_mutex.unlock();

    for (ListEventHandlers::iterator it = listHandler.begin(); it != listHandler.end(); ++it) {
        IEventHandler *pHandler = *it;
        pHandler->onEvent(pEvent);
    }

    delete pEvent;
}
