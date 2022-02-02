// LyricsKeywordFilter.h: interface for the CLyricsKeywordFilter class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "../Utils/Utils.h"


//
// CLyricsKeywordFilter is to filter the lyrics search keywords.
//
// Reference the "TestCLyricsKeywordFilter()" function for more.
//
class CLyricsKeywordFilter  
{
public:
    CLyricsKeywordFilter();
    virtual ~CLyricsKeywordFilter();

    static void init(cstr_t dataDir = nullptr);

    static void filter(const char *szTarg, string &strOut);
    static void filter(cwstr_t szTarg, u16string &strOut);

#define WORD_MAX        0xFFFF

protected:
    static void xTableDelChars(const char *szChars);
    static void xTableReplaceChars(const char *szReplace, const char *szTo);
    static void ucs2TableDelChars(const WCHAR *szChars);
    static void ucs2TableReplaceChars(const WCHAR *szReplace, const WCHAR *szTo);

protected:
    static WCHAR        m_wTableUcs2[WORD_MAX];

};
