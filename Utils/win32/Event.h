// Event.h: interface for the Event class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENT_H__DE7B3789_C417_4C03_90FF_BB9711A08C92__INCLUDED_)
#define AFX_EVENT_H__DE7B3789_C417_4C03_90FF_BB9711A08C92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WAIT_FOREVER INFINITE

class Event  
{
public:
    Event(bool bManualSet, bool bInitialStat);
    virtual ~Event();

    bool set();
    bool reset();
    bool acquire(uint32_t nTimeOut = WAIT_FOREVER);

protected:
    HANDLE        m_hObject;

};

#endif // !defined(AFX_EVENT_H__DE7B3789_C417_4C03_90FF_BB9711A08C92__INCLUDED_)
