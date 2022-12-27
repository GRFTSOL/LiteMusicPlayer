#pragma once

class CEventsDispatcher : public CEventsDispatcherBase, public Window {
public:
    int init();
    void quit();

    virtual void dispatchSyncEventByNoUIThread(IEvent *pEvent);
    virtual void dispatchUnsyncEvent(IEvent *pEvent);

protected:
    virtual LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

};
