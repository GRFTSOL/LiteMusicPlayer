#pragma once


class CEventsDispatcher : public CEventsDispatcherBase {
public:
    int init();
    void quit();

    virtual void dispatchSyncEventByNoUIThread(IEvent *pEvent);
    virtual void dispatchUnsyncEvent(IEvent *pEvent);

};

void postCustomCommandMsgMac(class CSkinWnd *pSkinWnd, int cmd);

void postQuitMessageMac();
