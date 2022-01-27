// ID3v2FrameParser.h: interface for the CID3v2FrameParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ID3V2FRAMEPARSER_H__749002BF_ECF1_4CCA_8869_17AFACE76F16__INCLUDED_)
#define AFX_ID3V2FRAMEPARSER_H__749002BF_ECF1_4CCA_8869_17AFACE76F16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ID3v2.h"

class CID3v2FrameParserBase  
{
public:
    CID3v2FrameParserBase(CHAR_ENCODING encoding) { m_encoding = encoding; }
    virtual ~CID3v2FrameParserBase() { }

//    virtual int parse(CID3v2Frame *pFrame) = 0;
//    virtual int toFrameData(CID3v2Frame *pFrame) = 0;
    CHAR_ENCODING        m_encoding;

};

class CID3v2FrameParserSynLyrics : public CID3v2FrameParserBase
{
public:
    enum CONTENT_TYPE
    {
        CT_OTHER        = 0,
        CT_LYRICS        = 1,
        CT_TEXT_TRANS    = 2,
        CT_MOVEMENT        = 3,
        CT_EVENT        = 4,
        CT_CHORD        = 5
    };

    CID3v2FrameParserSynLyrics(CHAR_ENCODING encoding) : CID3v2FrameParserBase(encoding) { }
    virtual ~CID3v2FrameParserSynLyrics() { }

    virtual int parseInfoOnly(CID3v2Frame *pFrame, ID3v2SynchLyrics &synchLyrics);
    virtual int parse(CID3v2Frame *pFrame, ID3v2SynchLyrics &synchLyrics);
    virtual int toFrameData(ID3v2SynchLyrics &synchLyrics, CID3v2Frame *pFrame);

    static bool isFrameLanguageSame(CID3v2Frame *pFrame, const char *szLanguage);

protected:

//    ID3v2SynchLyrics        m_Lyrics;

};

class CID3v2FrameParserUnsynLyrics : public CID3v2FrameParserBase
{
public:
    CID3v2FrameParserUnsynLyrics(CHAR_ENCODING encoding) : CID3v2FrameParserBase(encoding) { }
    virtual ~CID3v2FrameParserUnsynLyrics() { }

    virtual int parseInfoOnly(CID3v2Frame *pFrame, ID3v2UnsynchLyrics &unsynchLyrics);
    virtual int parse(CID3v2Frame *pFrame, ID3v2UnsynchLyrics &unsynchLyrics);
    virtual int toFrameData(ID3v2UnsynchLyrics &unsynchLyrics, CID3v2Frame *pFrame);

    static bool isFrameLanguageSame(CID3v2Frame *pFrame, const char *szLanguage);

protected:
    // ID3v2UnsynchLyrics        m_Lyrics;

};

class CID3v2FrameParserText : public CID3v2FrameParserBase
{
public:
    CID3v2FrameParserText(CHAR_ENCODING encoding) : CID3v2FrameParserBase(encoding) { }
    virtual ~CID3v2FrameParserText() { }

    virtual int parse(const CID3v2Frame *pFrame, ID3v2Text &text);
    virtual int toFrameData(ID3v2Text &text, CID3v2Frame *pFrame);

};

// <Header for 'User defined text information frame', ID: "TXXX">
// Text encoding     $xx
// Description       <text string according to encoding> $00 (00)
// Value             <text string according to encoding>
class CID3v2FrameParserTextUserDefined : public CID3v2FrameParserBase
{
public:
    CID3v2FrameParserTextUserDefined(CHAR_ENCODING encoding) : CID3v2FrameParserBase(encoding) { }
    virtual ~CID3v2FrameParserTextUserDefined() { }

    virtual int parse(const CID3v2Frame *pFrame, ID3v2TextUserDefined &text);
    virtual int toFrameData(ID3v2TextUserDefined &text, CID3v2Frame *pFrame);

};

//  <Header for 'Comment', ID: "COMM">
//  Text encoding          $xx
//  Language               $xx xx xx
//  Short content descrip. <text string according to encoding> $00 (00)
//  The actual text        <full text string according to encoding>
class CID3v2FrameParserComment : public CID3v2FrameParserBase
{
public:
    CID3v2FrameParserComment(CHAR_ENCODING encoding) : CID3v2FrameParserBase(encoding)
    {
        memset(m_szLanguage, 0, sizeof(m_szLanguage));
        m_EncodingType = IET_ANSI;
    }
    virtual int parse(const CID3v2Frame *pFrame);
    virtual int toFrame(CID3v2Frame *pFrame);

    void setText(cstr_t szShortDesc, cstr_t szText);

public:
    ID3v2EncType    m_EncodingType;
    char            m_szLanguage[4];
    string        m_strShortDesc, m_strText;

};

//  <Header for 'Attached picture', ID: "APIC">

// V2.2
//  Text encoding      $xx
//  Image format       $xx xx xx
//  Picture type       $xx
//  Description        <textstring> $00 (00)
//  Picture data       <binary data>
//
// V2.3
//  Text encoding      $xx
//  MIME type          <text string> $00
//  Picture type       $xx
//  Description        <text string according to encoding> $00 (00)
//  Picture data       <binary data>
class CID3v2FrameParserPic : public CID3v2FrameParserBase
{
public:
    CID3v2FrameParserPic(CHAR_ENCODING encoding) : CID3v2FrameParserBase(encoding)
    {
    }
    virtual int parse(CID3v2 *pid3v2, const CID3v2Frame *pFrame, ID3v2Pictures::ITEM *picture);
    virtual int toFrame(CID3v2 *pid3v2, CID3v2Frame *pFrame, ID3v2Pictures::ITEM *picture);

};


//  <Header for 'User defined URL link frame', ID: "WXXX">
//  Text encoding     $xx
//  Description       <text string according to encoding> $00 (00)
//  URL               <text string>
class CID3v2FrameParserUserDefinedUrl : public CID3v2FrameParserBase
{
public:
    CID3v2FrameParserUserDefinedUrl(CHAR_ENCODING encoding) : CID3v2FrameParserBase(encoding) { }
    virtual int parse(const CID3v2Frame *pFrame);
    virtual int toFrame(CID3v2Frame *pFrame);

public:
    ID3v2Text        m_textDesc;
    string            m_strUrl;

};


#endif // !defined(AFX_ID3V2FRAMEPARSER_H__749002BF_ECF1_4CCA_8869_17AFACE76F16__INCLUDED_)
