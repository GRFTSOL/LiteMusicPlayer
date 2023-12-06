#pragma once

class CEventsDispatcher : public CEventsDispatcherBase, public Window {
public:
    int init();
    void quit();

    void dispatchSyncEventByNoUIThread(IEvent *pEvent) override;
    void dispatchUnsyncEvent(IEvent *pEvent) override;
    void dispatchUnsyncEventDelayed(IEvent* pEvent, int delayInMs) override;

protected:
    virtual LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

};
