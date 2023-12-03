/********************************************************************
    Created  :    2002/01/04    21:40
    FileName :    LrcParser.h
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#pragma once

#include "LyricsParser.h"


class CLrcParser : public ILyricsParser {
public:
    int parseFile(cstr_t fileName, bool bUseSpecifiedEncoding, CharEncodingType encoding, RawLyrics &lyricsOut) override;
    int saveAsFile(cstr_t file, const RawLyrics &lyrics) override;

};
