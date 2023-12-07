#include "SkinTypes.h"
#include "EventsDispatcherBase.h"
#include "EventsDispatcher.h"


#define DISPATCH_EVENT_LPARAM   1212

#define MPWM_DISPATCH        (WM_USER + 1)
#define MPWM_QUIT            (WM_USER + 2)

#define TIMER_EXEC_TASK     100

/*
class CEventDispatcherWnd : public Window  
{
public:
    void dispatchSyncEventByNoUIThread(IEvent *pEvent);
    void dispatchUnsyncEvent(IEvent *pEvent);

    virtual LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

};

void CEventDispatcherWnd::dispatchSyncEventByNoUIThread(IEvent *pEvent)
{
    ::sendMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

void CEventDispatcherWnd::dispatchUnsyncEvent(IEvent *pEvent)
{
    ::PostMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

LRESULT CEventDispatcherWnd::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case MPWM_DISPATCH:
        {
            if (lParam == DISPATCH_EVENT_LPARAM)
            {
                IEvent *pEvent = (IEvent *)wParam;
                g_eventDispather.dispatchSyncEvent(pEvent);
                return 0;
            }
            break;
        }
    }

    return Window::wndProc(message, wParam, lParam);
}
*/
//////////////////////////////////////////////////////////////////////

int CEventsDispatcher::init() {
    if (!Window::createEx("MPMsg", "MPMsg", 0, 0, 1, 1, nullptr, 0)) {
        return ERR_FALSE;
    }

    return CEventsDispatcherBase::init();
}

void CEventsDispatcher::quit() {
    CEventsDispatcherBase::quit();

    Window::destroy();
}

void CEventsDispatcher::dispatchSyncEventByNoUIThread(IEvent *pEvent) {
    ::SendMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

void CEventsDispatcher::dispatchUnsyncEvent(IEvent *pEvent) {
    ::PostMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

void CEventsDispatcher::dispatchUnsyncEventDelayed(IEvent* pEvent, int delayInMs) {
    postExecInUIThreadDelayed([m_hWnd, pEvent]() {
        ::PostMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
    }, delayInMs);
}

void CEventsDispatcher::postExecInUIThread(std::function<void()> f) {
    std::lock_guard autolock(_mutex);
    _execItems.push_back({f, 0});
}

void CEventsDispatcher::postExecInUIThreadDelayed(std::function<void()> f, int delayInMs) {
    auto now = getTickCount() + delayInMs;

    std::lock_guard autolock(_mutex);
    _execItems.push_back({f, now});
    if (now < _nextTime) {
        ::SetTimer(m_hWnd, TIMER_EXEC_TASK, delayInMs, nullptr);
    }
}

void CEventsDispatcher::onExecTimer() {
    KillTimer(m_hWnd, TIMER_EXEC_TASK);

    while (true) {
        std::function<void()> func;
        bool ended;

        {
            std::lock_guard autolock(_mutex);
            auto now = getTickCount();
            for (auto it = _execItems.begin(); it != _execItems.end();) {
                if ((*it).execTime <= now) {
                    // Need to execute
                    func = (*it).func;
                    break;
                    it = _execItems.erase(it);
                } else {
                    ++it;
                }
            }

            ended = _execItems.empty();
        }

        if (func) {
            func();
            if (ended) {
                break;
            }
        } else {
            break;
        }
    }
}

LRESULT CEventsDispatcher::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case MPWM_DISPATCH:
        {
            if (lParam == DISPATCH_EVENT_LPARAM) {
                IEvent *pEvent = (IEvent *)wParam;
                dispatchSyncEvent(pEvent);
                return 0;
            }
            break;
        }
    case WM_TIMER:
        {
            if (wParam == TIMER_EXEC_TASK) {
                onExecTimer();
            }
            break;
        }
    }

    return Window::wndProc(message, wParam, lParam);
}
