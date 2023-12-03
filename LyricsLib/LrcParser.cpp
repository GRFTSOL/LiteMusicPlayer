/********************************************************************
    Created  :    2002/01/03    17:23:02
    FileName :    LrcParser.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "LrcParser.h"
#include "CurrentLyrics.h"
#include "../MediaTags/LrcParser.h"


int CLrcParser::parseFile(cstr_t fileName, bool bUseSpecifiedEncoding, CharEncodingType encoding, RawLyrics &lyricsOut) {
    string lyrics;

    if (!isFileExist(fileName)) {
        return ERR_FILE_NOT_EXIST;
    }

    if (!readFile(fileName, lyrics)) {
        return ERR_READ_FILE;
    }

    return parseLyricsBinary(lyrics, bUseSpecifiedEncoding, encoding, lyricsOut);
}

int CLrcParser::saveAsFile(cstr_t file, const RawLyrics &lyrics) {
    bool bToTxtFormat = fileIsExtSame(file, ".txt");

    auto text = toLyricsString(lyrics, true, bToTxtFormat);

    if (!writeFile(file, text)) {
        return ERR_WRITE_FILE;
    }

    return ERR_OK;
}
