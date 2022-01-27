#include "UtilsTypes.h"
#include "Date.h"
#include "StringEx.h"


static const int _MonthDays[12] =
{
    0,
    31, 
    31 + 28, 
    31 + 28 + 31, 
    31 + 28 + 31 + 30, 
    31 + 28 + 31 + 30 + 31, 
    31 + 28 + 31 + 30 + 31 + 30, 
    31 + 28 + 31 + 30 + 31 + 30 + 31, 
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31, 
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30, 
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31, 
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
};

inline int getDaysToYear(int year)
{
    year--;
    return year * 365 + year / 4 - year / 100 + year / 400;
}

inline int getDaysToMonth(int year, int month)
{
    assert(month >= 1 && month <= 12);
    int n = _MonthDays[month - 1];
    if (CDate::isLeapYear(year) && month > 2)
        n++;
    return n;
}

CDate::CDate()
{
    this->day = 0;
    this->month = 0;
    this->year = 0;
    this->hour = 0;
    this->minute = 0;
    this->second = 0;
    this->ms = 0;
}

CDate::CDate(int year, int month, int day)
{
    this->day = (uint8_t)day;
    this->month = (uint8_t)month;
    this->year = (uint16_t)year;
    this->hour = 0;
    this->minute = 0;
    this->second = 0;
    this->ms = 0;
}

CDate::CDate(int year, int month, int day, int hour, int minute, int second, int ms)
{
    this->day = (uint8_t)day;
    this->month = (uint8_t)month;
    this->year = (uint16_t)year;
    this->hour = (uint8_t)hour;
    this->minute = (uint8_t)minute;
    this->second = (uint8_t)second;
    this->ms = (uint16_t)ms;
}

uint32_t CDate::getTime()
{
    if (month <= 0 || month > 12)
        return 0;

    uint32_t nAllDays = getDaysToYear(year) + day - 1;

    nAllDays += getDaysToMonth(year, month);

    return (nAllDays - getDaysToYear(1970)) * SECOND_IN_ONE_DAY + ((hour * 60 + minute) * 60 + second);
}

void CDate::fromTime(uint32_t timeInMillis)
{
    ms = 0;

    second = timeInMillis % 60;
    timeInMillis /= 60;

    minute = timeInMillis % 60;
    timeInMillis /= 60;

    hour = timeInMillis % 24;
    timeInMillis /= 24;

    // Now, timeInMillis is Days.
    uint32_t daysTotal = timeInMillis + getDaysToYear(1970);

    // estimate the start year.
    year = timeInMillis / 365 + 1970;
    uint32_t daysToYear = getDaysToYear(year);
    while (daysToYear <= daysTotal)
    {
        daysToYear += 365;
        if (isLeapYear(year))
            daysToYear++;
        year++;
    }
    year--;

    int dayOfYear = daysTotal - getDaysToYear(year);
    assert(dayOfYear >= 0);
    for (month = 1; month <= 12; month++)
    {
        if ((int)dayOfYear < getDaysToMonth(year, month))
            break;
    }
    month--;

    day = dayOfYear - getDaysToMonth(year, month);
    day++;
}

uint32_t CDate::getMLDate()
{
    return (year << 16) | (month << 8) | day;
}

void CDate::fromMLDate(uint32_t mlDate)
{
    year = (uint16_t)(mlDate >> 16);
    month = (uint8_t)((mlDate >> 8) & 0xFF);
    day = (uint8_t)(mlDate & 0xFF);

    ms = hour = minute = second = 0;
}

enum
{
    S_MINUTE    = 60,
    S_HOUR        = (60 * 60),
    S_DAY        = (60 * 60 * 24),
    S_MONTH        = (60 * 60 * 24 * 31),
    S_YEAR        = (60 * 60 * 24 * 31 * 12),

    BEGIN_OF_DATE    = 1970
};

uint32_t CDate::getMLDateTime()
{
    return hour * S_HOUR + minute * 60 + second
        + (year - BEGIN_OF_DATE) * S_YEAR + (month - 1) * S_MONTH + (day - 1) * S_DAY;
}

void CDate::fromMLDateTime(uint32_t mlDateTime)
{
    year = (uint16_t)(mlDateTime / S_YEAR + BEGIN_OF_DATE);
    month = (uint8_t)((mlDateTime % S_YEAR) / S_MONTH) + 1;
    day = (uint8_t)((mlDateTime % S_MONTH) / S_DAY) + 1;
    hour = (uint8_t)((mlDateTime % S_DAY) / S_HOUR);
    minute = (uint8_t)((mlDateTime % S_HOUR) / S_MINUTE);
    second = (uint8_t)(mlDateTime % S_MINUTE);
    ms = 0;
}

string CDate::toUtcDateString()
{
    char szDate[64];
    sprintf(szDate, "%04d-%02d-%02d", year, month, day);
    return szDate;
}

string CDate::toUtcDateTimeString()
{
    char szDate[64];
    sprintf(szDate, "%04d-%02d-%02d %02d:%02d.%02d", year, month, day, hour, minute, second);
    return szDate;
}

bool CDate::fromString(cstr_t szDate)
{
    // year
    szDate = readInt_t(szDate, year);
    if (*szDate != '-' && *szDate != '/')
        return false;
    szDate++;

    // month
    szDate = readInt_t(szDate, month);
    if (*szDate != '-' && *szDate != '/')
        return false;
    szDate++;

    // day
    szDate = readInt_t(szDate, day);

    while (*szDate == ' ')
        szDate++;

    if (!*szDate)
    {
        ms = hour = minute = second = 0;
        return true;
    }

    // hour
    szDate = readInt_t(szDate, hour);
    if (*szDate != ':')
        return false;
    szDate++;

    // minute
    szDate = readInt_t(szDate, minute);
    if (*szDate != ':' && *szDate != '.')
        return false;
    szDate++;

    // second
    readInt_t(szDate, second);

    ms = 0;
    return true;
}

CDate CDate::getCurrentDate()
{
#ifdef _WIN32
    SYSTEMTIME    sysTime;

    GetSystemTime(&sysTime);

    return CDate(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
#else
    time_t        ltime;
    tm            *ptm;

    time(&ltime);
    ptm = gmtime(&ltime);

    return CDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, 0);
#endif
}

CDate CDate::getLocalTime()
{
#ifdef _WIN32
    SYSTEMTIME    sysTime;

    ::getLocalTime(&sysTime);

    return CDate(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
#else
    time_t        ltime;
    tm            *ptm;

    time(&ltime);
    ptm = localtime(&ltime);

    return CDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, 0);
#endif
}

bool CDate::isLeapYear(int year)
{
    if (year % 4 != 0)
        return false;

    if (year % 400 == 0)
        return true;
    if (year % 100 == 0)
        return false;

    return true;
}


//////////////////////////////////////////////////////////////////////////
// CPPUnit test
#ifdef _CPPUNIT_TEST

#include <time.h>

IMPLEMENT_CPPUNIT_TEST_REG(CDate)

class CTestCaseCBaseDate : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCaseCBaseDate);
    CPPUNIT_TEST(testFromTime);
    CPPUNIT_TEST(testFromString);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testFromTime()
    {
        time_t    timeCur = 0;
        time(&timeCur);
        tm *tmCur = gmtime(&timeCur);
        CDate t;
        t.fromTime((uint32_t)timeCur);

        CPPUNIT_ASSERT(tmCur->tm_year + 1900 == t.year);
        CPPUNIT_ASSERT(tmCur->tm_mon + 1 == t.month);
        CPPUNIT_ASSERT(tmCur->tm_mday == t.day);
        CPPUNIT_ASSERT(tmCur->tm_hour == t.hour);
        CPPUNIT_ASSERT(tmCur->tm_min == t.minute);
        CPPUNIT_ASSERT(tmCur->tm_sec == t.second);
        CPPUNIT_ASSERT(0 == t.ms);
        CPPUNIT_ASSERT(timeCur == t.getTime());

        t.month = 12;
        CDate t2;
        t2.fromTime(t.getTime());
        CPPUNIT_ASSERT(t2.month = t.month);
        CPPUNIT_ASSERT(t2.getTime() == t.getTime());

        for (int year = 1999; year < 2001; year++)
        {
            CDate    date(year, 1, 1);
            uint32_t time = date.getTime();

            CDate    date2;
            date2.fromTime(time);

            CPPUNIT_ASSERT(date2.year == year);
        }

        int vYear[] = { 1970, 1988, 2000, 2100 };

        for (int i = 0; i < CountOf(vYear); i++)
        {
            int year = vYear[i];
            for (int month = 1; month <= 12; month++)
            {
                int dayToMonthStart = getDaysToMonth(year, month);
                int dayToMOnthEnd;
                if (month == 12)
                    dayToMOnthEnd = dayToMonthStart + 31;
                else
                    dayToMOnthEnd = getDaysToMonth(year, month + 1);
                for (int day = 1; day <= dayToMOnthEnd - dayToMonthStart; day++)
                {
                    CDate    date(year, month, day);
                    uint32_t time = date.getTime();

                    CDate    date2;
                    date2.fromTime(time);

                    CPPUNIT_ASSERT(date2.year == year);
                    CPPUNIT_ASSERT(date2.month == month);
                    CPPUNIT_ASSERT(date2.day == day);
                }
            }
        }
    }

    void testFromString()
    {
        cstr_t vDate[] = { "2010-01-27", "1970-12-31", "2010-10-27" };

        for (int i = 0; i < CountOf(vDate); i++)
        {
            CDate    date;

            date.fromString(vDate[i]);
            string str = date.toUtcDateString();
            CPPUNIT_ASSERT(str == vDate[i]);
        }

        cstr_t vDateTime[] = { "2010-01-27 10:12.01", "1970-12-31 10:02.15", "2010-10-27 10:22.59" };

        for (int i = 0; i < CountOf(vDateTime); i++)
        {
            CDate    date;

            date.fromString(vDateTime[i]);
            string str = date.toUtcDateTimeString();
            CPPUNIT_ASSERT(str == vDateTime[i]);
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCBaseDate);

#endif // _CPPUNIT_TEST
