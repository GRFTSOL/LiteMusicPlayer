#pragma once

#ifndef Utils_win32_Event_h
#define Utils_win32_Event_h


#define WAIT_FOREVER        INFINITE

class Event {
public:
    Event(bool bManualSet, bool bInitialStat);
    virtual ~Event();

    bool set();
    bool reset();
    bool acquire(uint32_t nTimeOut = WAIT_FOREVER);

protected:
    HANDLE                      m_hObject;

};

#endif // !defined(Utils_win32_Event_h)
