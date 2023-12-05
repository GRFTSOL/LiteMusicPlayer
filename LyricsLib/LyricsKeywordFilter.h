#pragma once

#include "../Utils/Utils.h"


//
// CLyricsKeywordFilter is to filter the lyrics search keywords.
//
// Reference the "TestCLyricsKeywordFilter()" function for more.
//
class CLyricsKeywordFilter {
public:
    CLyricsKeywordFilter();
    virtual ~CLyricsKeywordFilter();

    static void init();

    static void filter(const char *szTarg, string &strOut);
    static void filter(cwstr_t szTarg, utf16string &strOut);

#define WORD_MAX            0xFFFF

protected:
    static void xTableDelChars(const char *szChars);
    static void xTableReplaceChars(const char *szReplace, const char *szTo);
    static void ucs2TableDelChars(const WCHAR *szChars);
    static void ucs2TableReplaceChars(const WCHAR *szReplace, const WCHAR *szTo);

protected:
    static WCHAR                m_wTableUcs2[WORD_MAX];

};
