// SncParser.h: interface for the CSncParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNCPARSER_H__181F4326_B130_48BC_8EC8_510BD0BBD5F7__INCLUDED_)
#define AFX_SNCPARSER_H__181F4326_B130_48BC_8EC8_510BD0BBD5F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricsParser.h"

class CSncParser : public CLyricsParser  
{
public:
    CSncParser(CMLData *pMLData);
    virtual ~CSncParser();

    virtual int parseFile(bool bUseSpecifiedEncoding, CharEncodingType encoding) { return ERR_FALSE; }
    virtual int saveAsFile(cstr_t file);

    virtual LYRICS_CONTENT_TYPE getLyrContentType() { return LCT_UNKNOWN; }

protected:
    bool lyricsLineToText(LyricsLine *pLine, string &strBuff);

};

#endif // !defined(AFX_SNCPARSER_H__181F4326_B130_48BC_8EC8_510BD0BBD5F7__INCLUDED_)
