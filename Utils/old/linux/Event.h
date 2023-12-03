#pragma once

#ifndef Utils_old_linux_Event_h
#define Utils_old_linux_Event_h

#ifndef INFINITE
#define INFINITE            0xFFFFFFFF
#endif

#define WAIT_FOREVER        INFINITE

class Event {
public:
    Event(bool bManualSet, bool bInitialStat);
    virtual ~Event();

    bool set();
    bool reset();
    bool acquire(uint32_t nTimeOut = WAIT_FOREVER);

protected:

};

#endif // !defined(Utils_old_linux_Event_h)
