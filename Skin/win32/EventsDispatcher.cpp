#include "SkinTypes.h"
#include "EventsDispatcherBase.h"
#include "EventsDispatcher.h"


#define DISPATCH_EVENT_LPARAM   1212

#define MPWM_DISPATCH        (WM_USER + 1)
#define MPWM_QUIT            (WM_USER + 2)

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
    ::sendMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

void CEventsDispatcher::dispatchUnsyncEvent(IEvent *pEvent) {
    ::PostMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
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
    }

    return Window::wndProc(message, wParam, lParam);
}
