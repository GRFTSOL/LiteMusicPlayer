// Event.h: interface for the CEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENT_H__DE7B3789_C417_4C03_90FF_BB9711A08C92__INCLUDED_)
#define AFX_EVENT_H__DE7B3789_C417_4C03_90FF_BB9711A08C92__INCLUDED_


#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif

#ifndef WAIT_FOREVER
#define WAIT_FOREVER 0xFFFFFFFF
#endif

class _NSConditionInternal;

class Event  
{
public:
    Event(bool bManualSet, bool isSingaled);
    virtual ~Event();

    bool set();
    bool reset();
    bool acquire(uint32_t nTimeOut = WAIT_FOREVER);

protected:
    bool    m_bMaualset;
    
    _NSConditionInternal *m_nsCondition;

};

#endif // !defined(AFX_EVENT_H__DE7B3789_C417_4C03_90FF_BB9711A08C92__INCLUDED_)
