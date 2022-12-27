#include <time.h>
#include "MPTime.h"


static int _monthDays[] = { // gs_nMonthFirstDayFromYearStart
   -1,
   -1 + 31, 
   -1 + 31 + 28, 
   -1 + 31 + 28 + 31, 
   -1 + 31 + 28 + 31 + 30, 
   -1 + 31 + 28 + 31 + 30 + 31, 
   -1 + 31 + 28 + 31 + 30 + 31 + 30, 
   -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31, 
   -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31, 
   -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30, 
   -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31, 
   -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
   -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
};

static int _lpMonthDays[] = {
   -1,
   -1 + 31, 
   -1 + 31 + 29, 
   -1 + 31 + 29 + 31, 
   -1 + 31 + 29 + 31 + 30, 
   -1 + 31 + 29 + 31 + 30 + 31, 
   -1 + 31 + 29 + 31 + 30 + 31 + 30, 
   -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31, 
   -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31, 
   -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30, 
   -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31, 
   -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
   -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
};


/*
    CMPTime test

void test()
{
    CMPTime        time;
    CMPTimeSpan    span(1, 0, 0, 0);
    MPTM        mptm;

    time.getCurrentTime();

    CTime        time2;
    CTimeSpan    span2(1, 0, 0, 0);
    tm            *ptm;

    time2 = CTime::getCurrentTime();

    for (int i = 0; i < 159; i++)
    {
        time += span;
        time2 += span2;
    }

    for (i = 0; i < 365 * 50; i++)
    {
        time += span;
        time2 += span2;
        ptm = time2.GetGmtTm(nullptr);
        time.getMPTM(&mptm);
        assert(ptm->tm_year == mptm.tm_year &&
            ptm->tm_mon == mptm.tm_mon &&
            ptm->tm_mday == mptm.tm_mday &&
            ptm->tm_hour == mptm.tm_hour &&
            ptm->tm_min == mptm.tm_min &&
            // ptm->tm_sec == mptm.tm_sec &&
            ptm->tm_wday == mptm.tm_wday &&
            ptm->tm_yday == mptm.tm_yday &&
            ptm->tm_isdst == mptm.tm_isdst
            );
    }
}
*/

CMPTime::CMPTime() {
    m_time = -1;
}

CMPTime::~CMPTime() {

}

void CMPTime::set(int wYear, int wMonth, int wDay, int wHour, int wMinute, int wSecond) {
    int tmpdays;

    /*
     * Do a quick range check on the year and convert it to a delta
     * off of 1900.
     */
    if ( ((long)(wYear -= 1900) < _BASE_YEAR) || ((long)wYear > _MAX_YEAR) ) {
        m_time = -1;
        return;
    }

    /*
     * Compute the number of elapsed days in the current year minus
     * one. Note the test for leap year and the would fail in the year 2100
     * if this was in range (which it isn't).
     */
    tmpdays = wDay + _monthDays[wMonth - 1];
    if ( !(wYear & 3) && (wMonth > 2) ) {
        // in a leap year, after Feb. add one day for elapsed Feb 29.
        tmpdays++;
    }

    /*
     * Compute the number of elapsed seconds since the Epoch. Note the
     * computation of elapsed leap years would break down after 2100
     * if such values were in range (fortunately, they aren't).
     */

    m_time = (long)wYear - _BASE_YEAR;

    m_time = /* 365 days for each year */
    ( ( ( ( m_time ) * 365L

        /* one day for each elapsed leap year */
        + ((long)(wYear - 1) >> 2) - (long)_LEAP_YEAR_ADJUST

        /* number of elapsed days in yr */
        + (long)tmpdays )

        /* convert to hours and add in hr */
        * 24L + (long)wHour )

        /* convert to minutes and add in mn */
        * 60L + (long)wMinute )

    /* convert to seconds and add in sec */
    * 60L + (long)wSecond;

    // m_time += _timezone;        //timezone adjustment

    if (m_time < 0) {
        m_time = -1;
    }
}

void CMPTime::getCurrentTime() {
    time_t t = time(&t);
    tm *tmCur = gmtime(&t);
    set(tmCur->tm_year + 1900, tmCur->tm_mon + 1, tmCur->tm_mday, tmCur->tm_hour, tmCur->tm_min, tmCur->tm_sec);
}

bool CMPTime::getMPTM(MPTM *ptm) {
    long caltim = m_time; /* calendar time to convert */
    int islpyr = 0; /* is-current-year-a-leap-year flag */
    int tmptim;
    int *mdays; /* pointer to days or lpdays */

    if ( caltim < 0L ) {
        return false;
    }

    /*
     * Determine years since 1970. First, identify the four-year interval
     * since this makes handling leap-years easy (note that 2000 IS a
     * leap year and 2100 is out-of-range).
     */
    tmptim = (int)(caltim / _FOUR_YEAR_SEC);
    caltim -= ((long)tmptim * _FOUR_YEAR_SEC);

    /*
     * Determine which year of the interval
     */
    tmptim = (tmptim * 4) + 70; /* 1970, 1974, 1978,...,etc. */

    if ( caltim >= _YEAR_SEC ) {

        tmptim++; /* 1971, 1975, 1979,...,etc. */
        caltim -= _YEAR_SEC;

        if ( caltim >= _YEAR_SEC ) {

            tmptim++; /* 1972, 1976, 1980,...,etc. */
            caltim -= _YEAR_SEC;

            /*
             * Note, it takes 366 days-worth of seconds to get past a leap
             * year.
             */
            if ( caltim >= (_YEAR_SEC + _DAY_SEC) ) {

                tmptim++; /* 1973, 1977, 1981,...,etc. */
                caltim -= (_YEAR_SEC + _DAY_SEC);
            } else {
                /*
                     * In a leap year after all, set the flag.
                     */
                islpyr++;
            }
        }
    }

    /*
     * tmptim now holds the value for tm_year. caltim now holds the
     * number of elapsed seconds since the beginning of that year.
     */
    ptm->tm_year = tmptim;

    /*
     * Determine days since January 1 (0 - 365). This is the tm_yday value.
     * Leave caltim with number of elapsed seconds in that day.
     */
    ptm->tm_yday = (int)(caltim / _DAY_SEC);
    caltim -= (long)(ptm->tm_yday) * _DAY_SEC;

    /*
     * Determine months since January (0 - 11) and day of month (1 - 31)
     */
    if ( islpyr ) {
        mdays = _lpMonthDays;
    } else {
        mdays = _monthDays;
    }


    for ( tmptim = 1 ; mdays[tmptim] < ptm->tm_yday ; tmptim++ ) ;

    ptm->tm_mon = --tmptim;

    ptm->tm_mday = ptm->tm_yday - mdays[tmptim];

    /*
     * Determine days since Sunday (0 - 6)
     */
    ptm->tm_wday = ((int)(m_time / _DAY_SEC) + _BASE_DOW) % 7;

    /*
     *  Determine hours since midnight (0 - 23), minutes after the hour
     *  (0 - 59), and seconds after the minute (0 - 59).
     */
    ptm->tm_hour = (int)(caltim / 3600);
    caltim -= (long)ptm->tm_hour * 3600L;

    ptm->tm_min = (int)(caltim / 60);
    ptm->tm_sec = (int)(caltim - (ptm->tm_min) * 60);

    ptm->tm_isdst = 0;

    return true;
}
