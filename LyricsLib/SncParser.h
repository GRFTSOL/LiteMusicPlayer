#pragma once

#ifndef LyricsLib_SncParser_h
#define LyricsLib_SncParser_h


#include "LyricsParser.h"


class CSncParser : public CLyricsParser {
public:
    CSncParser(CMLData *pMLData);
    virtual ~CSncParser();

    virtual int parseFile(bool bUseSpecifiedEncoding, CharEncodingType encoding) { return ERR_FALSE; }
    virtual int saveAsFile(cstr_t file);

    virtual LYRICS_CONTENT_TYPE getLyrContentType() { return LCT_UNKNOWN; }

protected:
    bool lyricsLineToText(LyricsLine *pLine, string &strBuff);

};

#endif // !defined(LyricsLib_SncParser_h)
