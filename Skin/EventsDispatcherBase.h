#pragma once

#include <mutex>


class CEventsDispatcherBase;
typedef uint32_t EventType;

class IEvent {
public:
    virtual ~IEvent() {}
    EventType                   eventType;
    string                      name;
    string                      strValue;
};

class IEventHandler {
public:
    IEventHandler() { m_pEventDispatcher = nullptr; }

    virtual ~IEventHandler();
    virtual void onEvent(const IEvent *pEvent) = 0;

    void registerHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1);
    void registerHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1, EventType ev2);
    void registerHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1, EventType ev2, EventType ev3);
    void registerHandler(CEventsDispatcherBase *pEventDispatcher, EventType ev1, EventType ev2, EventType ev3, EventType ev4);

    void unregisterHandler();

    CEventsDispatcherBase       *m_pEventDispatcher;

};

class CEventsDispatcherBase {
public:
    CEventsDispatcherBase();
    virtual ~CEventsDispatcherBase();

    virtual int init();
    virtual void quit();

    virtual void registerHandler(EventType eventType, IEventHandler *pHandler);
    virtual void unRegisterHandler(EventType eventType, IEventHandler *pHandler);
    virtual void unRegisterHandler(IEventHandler *pHandler);

    // CEventsDispatcher will delete pEvent, after pass through it to all handlers.
    void dispatchSyncEvent(IEvent *pEvent);
    void dispatchSyncEvent(EventType eventType) {
        IEvent *pEvent = new IEvent;
        pEvent->eventType = eventType;
        dispatchSyncEvent(pEvent);
    }

    virtual void dispatchSyncEventByNoUIThread(IEvent *pEvent) = 0;
    virtual void dispatchUnsyncEvent(IEvent *pEvent) = 0;
    void dispatchUnsyncEvent(EventType eventType) {
        IEvent *pEvent = new IEvent;
        pEvent->eventType = eventType;
        dispatchUnsyncEvent(pEvent);
    }

protected:
    typedef list<IEventHandler *>            LIST_EVENTHANDLER;
    typedef vector<LIST_EVENTHANDLER>        VecListEventHandler;
    VecListEventHandler         m_vListEventHandler;
    std::mutex                  m_mutex;

};
