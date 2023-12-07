#pragma once

#include <functional>


class CEventsDispatcher : public CEventsDispatcherBase {
public:
    int init() override;
    void quit() override;

    void dispatchSyncEventByNoUIThread(IEvent *pEvent) override;
    void dispatchUnsyncEvent(IEvent *pEvent) override;
    void dispatchUnsyncEventDelayed(IEvent *pEvent, int delayInMs) override;

    void postExecInUIThread(std::function<void()> f) override;
    void postExecInUIThreadDelayed(std::function<void()> f, int delayInMs) override;

};

void postCommandMsgMac(class CSkinWnd *pSkinWnd, int cmd);

void postQuitMessageMac();
