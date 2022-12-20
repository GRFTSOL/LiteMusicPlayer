/********************************************************************
    Created  :    2002/01/03    17:12
    FileName :    LyricsParser.h
    Author   :    xhy
*********************************************************************/

#pragma once

class CLyricsParser  
{
public:
    CLyricsParser(CMLData *pMLData) : m_pMLData(pMLData)
    {
    }
    virtual ~CLyricsParser() { };

public:
    virtual int parseFile(bool bUseSpecifiedEncoding, CharEncodingType encoding) = 0;
    virtual int saveAsFile(cstr_t file) = 0;

    virtual LYRICS_CONTENT_TYPE getLyrContentType() = 0;

public:
    CMLData                *m_pMLData;
    
};
