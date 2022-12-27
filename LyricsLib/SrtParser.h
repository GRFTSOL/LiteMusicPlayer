#pragma once

#ifndef LyricsLib_SrtParser_h
#define LyricsLib_SrtParser_h


#include "LyricsParser.h"


class CSrtParser : public CLyricsParser {
public:
    CSrtParser(CMLData *pMLData);
    virtual ~CSrtParser();

public:
    virtual int parseFile(bool bUseSpecifiedEncoding, CharEncodingType encoding);
    virtual int saveAsFile(cstr_t file);

    virtual LYRICS_CONTENT_TYPE getLyrContentType();

protected:
    bool lyricsLineToText(LyricsLine *pLine, string &strBuff);

};

#endif // !defined(LyricsLib_SrtParser_h)
