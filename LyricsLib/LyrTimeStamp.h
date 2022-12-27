#pragma once

class CLyrTimeStamp {
public:
    static bool isTimeStamps(cstr_t szTimeStamps);

    static bool parse(cstr_t szTimeStamps, CMLData *pMLData);
    static void toString(string &strTimeStamps, int nOffsetTime, CLyricsLines &lyrLines);

};
