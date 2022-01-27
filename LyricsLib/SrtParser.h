// SrtParser.h: interface for the CSrtParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SRTPARSER_H__C4C41834_C58B_4D99_BD54_B42E3F58DB90__INCLUDED_)
#define AFX_SRTPARSER_H__C4C41834_C58B_4D99_BD54_B42E3F58DB90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricsParser.h"

class CSrtParser : public CLyricsParser  
{
public:
    CSrtParser(CMLData *pMLData);
    virtual ~CSrtParser();

public:
    virtual int parseFile(bool bUseSpecifiedEncoding, CHAR_ENCODING encoding);
    virtual int saveAsFile(cstr_t file);

    virtual LYRICS_CONTENT_TYPE getLyrContentType();

protected:
    bool lyricsLineToText(LyricsLine *pLine, string &strBuff);
    
};

#endif // !defined(AFX_SRTPARSER_H__C4C41834_C58B_4D99_BD54_B42E3F58DB90__INCLUDED_)
