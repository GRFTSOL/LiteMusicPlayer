#pragma once

#ifndef LyricsLib_SrtParser_h
#define LyricsLib_SrtParser_h


#include "LyricsParser.h"


class CSrtParser : public ILyricsParser {
public:
    virtual int parseFile(cstr_t fileName, bool bUseSpecifiedEncoding, CharEncodingType encoding, RawLyrics &lyricsOut);
    virtual int saveAsFile(cstr_t file, const RawLyrics &lyrics);

protected:
    bool lyricsLineToText(const LyricsLine &line, string &strBuff);

};

#endif // !defined(LyricsLib_SrtParser_h)
