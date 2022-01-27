// MPTime.h: interface for the CMPTime class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPTIME_H__FFFA74F6_6E4E_4EFD_94C1_BC2E8DE1D312__INCLUDED_)
#define AFX_MPTIME_H__FFFA74F6_6E4E_4EFD_94C1_BC2E8DE1D312__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct MPTM
{
    int tm_sec;     /* seconds after the minute - [0,59] */
    int tm_min;     /* minutes after the hour - [0,59] */
    int tm_hour;    /* hours since midnight - [0,23] */
    int tm_mday;    /* day of the month - [1,31] */
    int tm_mon;     /* months since January - [0,11] */
    int tm_year;    /* years since 1900 */
    int tm_wday;    /* days since Sunday - [0,6] */
    int tm_yday;    /* days since January 1 - [0,365] */
    int tm_isdst;   /* daylight savings time flag */
};

class CMPTimeSpan
{
public:
    CMPTimeSpan() { m_span = 0; }
    CMPTimeSpan(int nDays, int nHours, int nMinutes, int nSeconds)
    {
        set(nDays, nHours, nMinutes, nSeconds);
    }
    ~CMPTimeSpan() { }

    void set(int nDays, int nHours, int nMinutes, int nSeconds)
    {
        m_span = (((nDays * 24) + nHours) * 60 + nMinutes) * 60 + nSeconds;
    }

public:
    long            m_span;

};

class CMPTime
{
public:
    long            m_time;

    enum
    {
        _DAY_SEC           = (24L * 60L * 60L),    /* secs in a day */

        _YEAR_SEC          = (365L * _DAY_SEC),    /* secs in a year */

        _FOUR_YEAR_SEC     = (1461L * _DAY_SEC),   /* secs in a 4 year interval */

        _DEC_SEC           = 315532800L,           /* secs in 1970-1979 */

        _BASE_YEAR         = 70L,                  /* 1970 is the base year */

        _BASE_DOW          = 4,                    /* 01-01-70 was a Thursday */

        _LEAP_YEAR_ADJUST  = 17L,                  /* Leap years 1900 - 1970 */

        _MAX_YEAR          = 138L,                 /* 2038 is the max year */
    };

public:
    CMPTime();
    ~CMPTime();

    void getCurrentTime();

    bool getMPTM(MPTM *ptm);

    void set(int wYear, int wMonth, int wDay, int wHour, int wMinute, int wSecond);

    const CMPTime& operator+=(CMPTimeSpan timeSpan) { m_time += timeSpan.m_span; return *this; }
    const CMPTime& operator-=(CMPTimeSpan timeSpan) { m_time -= timeSpan.m_span; return *this; }

};

#endif // !defined(AFX_MPTIME_H__FFFA74F6_6E4E_4EFD_94C1_BC2E8DE1D312__INCLUDED_)
