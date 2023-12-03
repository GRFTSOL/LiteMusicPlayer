#pragma once

#ifndef LyricsLib_SncParser_h
#define LyricsLib_SncParser_h


#include "LyricsParser.h"


class CSncParser : public ILyricsParser {
public:
    virtual int parseFile(cstr_t fileName, bool bUseSpecifiedEncoding, CharEncodingType encoding, RawLyrics &lyricsOut) { return ERR_FALSE; }
    virtual int saveAsFile(cstr_t file, const RawLyrics &lyrics);

protected:
    bool lyricsLineToText(LyricsLine &line, int offsetTime, string &strBuff);

};

#endif // !defined(LyricsLib_SncParser_h)
