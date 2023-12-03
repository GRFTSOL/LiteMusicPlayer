#pragma once

#ifndef Utils_mac_Event_h
#define Utils_mac_Event_h


#ifndef INFINITE
#define INFINITE            0xFFFFFFFF
#endif

#ifndef WAIT_FOREVER
#define WAIT_FOREVER        0xFFFFFFFF
#endif

class _NSConditionInternal;

class Event {
public:
    Event(bool bManualSet, bool isSingaled);
    virtual ~Event();

    bool set();
    bool reset();
    bool acquire(uint32_t nTimeOut = WAIT_FOREVER);

protected:
    bool                        m_bMaualset;

    _NSConditionInternal        *m_nsCondition;

};

#endif // !defined(Utils_mac_Event_h)
