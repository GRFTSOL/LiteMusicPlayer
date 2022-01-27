
#pragma once

class CDate
{
public:
    uint8_t    day, month;
    uint16_t    year;
    uint8_t    hour;
    uint8_t    minute, second;
    uint16_t    ms;

    enum {
        SECOND_IN_ONE_DAY = 60 * 60 * 24,
        MILLIS_IN_ONE_DAY = 1000 * SECOND_IN_ONE_DAY,
    };

public:
    CDate();
    CDate(int year, int month, int day);
    CDate(int year, int month, int day, int hour, int minute, int second, int ms);

    // Returns this Date as a second value. 
    // The value is the number of seconds since Jan. 1, 1970, midnight GMT.
    uint32_t getTime();
    void fromTime(uint32_t timeInMillis);

    // Returns the MiniLyrics server date format as the uint32_t value.
    uint32_t getMLDate();
    void fromMLDate(uint32_t mlDate);

    // Returns the MiniLyrics server date time format as the uint32_t value.
    uint32_t getMLDateTime();
    void fromMLDateTime(uint32_t mlDateTime);

    string toUtcDateString();
    string toUtcDateTimeString();

    bool fromString(cstr_t szDate);

    bool isLeapYear() const { return isLeapYear(year); }

    static bool isLeapYear(int year);

    static CDate getCurrentDate();

    static CDate getLocalTime();

};
