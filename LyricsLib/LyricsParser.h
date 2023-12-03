/********************************************************************
    Created  :    2002/01/03    17:12
    FileName :    LyricsParser.h
    Author   :    xhy
*********************************************************************/

#pragma once

#include "../MediaTags/LyricsData.h"


class ILyricsParser {
public:
    ILyricsParser() { }
    virtual ~ILyricsParser() { };

public:
    virtual int parseFile(cstr_t fileName, bool bUseSpecifiedEncoding, CharEncodingType encoding, RawLyrics &lyricsOut) = 0;
    virtual int saveAsFile(cstr_t file, const RawLyrics &lyrics) = 0;

};
