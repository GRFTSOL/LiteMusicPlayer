// ID3v2.h: interface for the CID3v2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ID3V2_H__8F5C4F2D_B919_4876_B17B_FF2C4AD71904__INCLUDED_)
#define AFX_ID3V2_H__8F5C4F2D_B919_4876_B17B_FF2C4AD71904__INCLUDED_

#pragma once

#include "../../Utils/Utils.h"


class CID3v2Frame;


enum ID3v2EncType
{
    IET_ANSI            = 0,
    IET_UCS2LE_BOM        = 1,
    IET_UCS2BE_NO_BOM    = 2,
    IET_UTF8            = 3,
};

CHAR_ENCODING iD3v2EncTypeToCharEncoding(ID3v2EncType encType);
ID3v2EncType charEncodingToID3v2EncType(CHAR_ENCODING encoding);

enum ID3v2FrameAction
{
    IFA_NONE,
    IFA_ADD,
    IFA_DEL,
    IFA_MODIFY,
};

#define ID3_TAGID               "ID3"
#define ID3_TAGIDSIZE           (3)
#define ID3_TAGHEADERSIZE       (10)
#define ID3_LEN_SIZE            (4)
#define ID3_TAGFOOTERSIZE       (10)

#pragma pack(push)
#pragma pack(1)
/************************************************************************\
     +-----------------------------+
     |      Header (10 bytes)      |
     +-----------------------------+
     |       Extended Header       |
     | (variable length, OPTIONAL) |
     +-----------------------------+
     |   frames (variable length)  |
     +-----------------------------+
     |           Padding           |
     | (variable length, OPTIONAL) |
     +-----------------------------+
     | Footer (10 bytes, OPTIONAL) |
     +-----------------------------+
\************************************************************************/
struct ID3v2Header
{
    enum
    {
        HEADER_FLAG_UNSYNC       = 1 << 7,
        HEADER_FLAG_EXTENDED     = 1 << 6,
        HEADER_FLAG_EXPERIMENTAL = 1 << 5,    // The second bit (bit 6) indicates whether or not the header is followed by an extended header. The extended header is described in section 3.2.
        HEADER_FLAG_FOOTER       = 1 << 4    //
    };
    
    enum
    {
        EXT_HEADER_FLAG_BIT1  = 1 << 7,
        EXT_HEADER_FLAG_BIT2  = 1 << 6,
        EXT_HEADER_FLAG_BIT3  = 1 << 5,
        EXT_HEADER_FLAG_BIT4  = 1 << 4
    };

    enum ID3v2Version
    {
        ID3V2_V2 = 2,
        ID3V2_V3 = 3,
        ID3V2_V4 = 4,
    };

    char    szID[ID3_TAGIDSIZE];
    uint8_t    byMajorVer;
    uint8_t    byRevisionVer;
    uint8_t    byFlag;
    uint8_t    bySize[ID3_LEN_SIZE];
            //   The ID3v2 tag size is the sum of the uint8_t length of the extended
            //   header, the padding and the frames after unsynchronisation. If a
            //   footer is present this equals to ('total size' - 20) bytes, otherwise
            //   ('total size' - 10) bytes.

    bool isFooterFlagSet() { return (byFlag & HEADER_FLAG_FOOTER) == HEADER_FLAG_FOOTER; }
    bool isExtendedFlagSet() { return (byFlag & HEADER_FLAG_EXTENDED) == HEADER_FLAG_EXTENDED; }
    bool isUnsyncFlagSet() { return (byFlag & HEADER_FLAG_UNSYNC) == HEADER_FLAG_UNSYNC; }
    bool isExperimentalFlagSet() { return (byFlag & HEADER_FLAG_EXPERIMENTAL) == HEADER_FLAG_EXPERIMENTAL; }
};

/***********************************************************************\
The extended header contains information that is not vital to the correct
parsing of the tag information, hence the extended header is optional. 
     Extended header size   4 * %0xxxxxxx
     Number of flag bytes       $01
     Extended Flags             $xx

   Where the 'Extended header size' is the size of the whole extended
   header, stored as a 32 bit synchsafe integer. An extended header can
   thus never have a size of fewer than six bytes.
\***********************************************************************/
struct ID3v2HeaderExtended
{
    uint8_t    bySize[4];
    // uint8_t    byData[1];
};

struct ID3v2Footer
{
    uint8_t    bySize[10];
    // uint8_t    byData[1];
};


#define ID3V2_FRAMEIDLEN        4

struct ID3v2FrameHeaderV2
{
    char    szFrameID[3];
    uint8_t    bySize[3];

    uint32_t toFrameUintID() { return ((unsigned char)(szFrameID[0]) << 24) | ((unsigned char)(szFrameID[1]) << 16) | ((unsigned char)(szFrameID[2]) << 8); }
    void fromFrameUintID(uint32_t nFrameID)
    {
        szFrameID[0] = (nFrameID & 0xFF000000) >> 24;
        szFrameID[1] = (nFrameID & 0x00FF0000) >> 16;
        szFrameID[2] = (nFrameID & 0x0000FF00) >> 8;
    }
};

// A frame must be at least 1 uint8_t big, excluding the header.
struct ID3v2FrameHeaderV3
{
    char    szFrameID[ID3V2_FRAMEIDLEN];
    uint8_t    bySize[4];        // The size excluding frame header (frame size - 10).
    uint8_t    byFlags[2];

    uint32_t toFrameUintID() { return ((unsigned char)(szFrameID[0]) << 24) | ((unsigned char)(szFrameID[1]) << 16) | ((unsigned char)(szFrameID[2]) << 8) | (unsigned char)(szFrameID[3]); }
    void fromFrameUintID(uint32_t nFrameID)
    {
        szFrameID[0] = (nFrameID & 0xFF000000) >> 24;
        szFrameID[1] = (nFrameID & 0x00FF0000) >> 16;
        szFrameID[2] = (nFrameID & 0x0000FF00) >> 8;
        szFrameID[3] = (nFrameID & 0x000000FF);
    }
};

typedef int        ID3v2FrameUID_t;

#define MakeUintFrameID3(by1, by2, by3)            (((by1) << 24) | ((by2) << 16) | ((by3) << 8))
#define MakeUintFrameID4(by1, by2, by3, by4)    (((by1) << 24) | ((by2) << 16) | ((by3) << 8) | (by4))

enum ID3v2_2_GENERAL_ID
{
    ID3V2_2_COMPOSER    = MakeUintFrameID3('T', 'C', 'M'),        // TCM    : Composer
    ID3V2_2_TPOS        = MakeUintFrameID3('T', 'P', 'O'),        // TPOS :
    ID3V2_2_ARTIST        = MakeUintFrameID3('T', 'P', '1'),        // TP1 : Artist
    ID3V2_2_ALBUMARTIST    = MakeUintFrameID3('T', 'P', '2'),        // TP2 : Album artist
    ID3V2_2_ALBUM        = MakeUintFrameID3('T', 'A', 'L'),        // TAL : Album
    ID3V2_2_TITLE        = MakeUintFrameID3('T', 'T', '2'),        // TT2 : Title
    ID3V2_2_TRACK        = MakeUintFrameID3('T', 'R', 'K'),        // TRK : Track
    ID3V2_2_YEAR        = MakeUintFrameID3('T', 'R', 'D'),        // TRD : Year
    ID3V2_2_USER_TEXT    = MakeUintFrameID3('T', 'X', 'X'),        // TXX : User defined text information frame
    ID3V2_2_COMMENT        = MakeUintFrameID3('C', 'O', 'M'),        // COM : Comment
    ID3V2_2_GENRE        = MakeUintFrameID3('T', 'C', 'O'),        // TCO : Genre
    ID3V2_2_PICTURE        = MakeUintFrameID3('P', 'I', 'C'),        // PIC : pic
    ID3V2_2_ENCODED        = MakeUintFrameID3('T', 'E', 'N'),        // TEN : encoded
    ID3V2_2_URL            = MakeUintFrameID3('W', 'X', 'X'),        // WXX : url
    ID3V2_2_COPYRIGHT    = MakeUintFrameID3('T', 'C', 'R'),        // TCR : copy right
    ID3V2_2_ORIG_ARTIST    = MakeUintFrameID3('T', 'O', 'A'),        // TOA : Orig. artist
    ID3V2_2_PUBLISHER    = MakeUintFrameID3('T', 'P', 'B'),        // TPB : Publisher
    ID3V2_2_BPM            = MakeUintFrameID3('T', 'B', 'P'),        // TBP : bpm
    ID3V2_2_USLT        = MakeUintFrameID3('U', 'L', 'T'),        // ULT : unsync lyrics
    ID3V2_2_SYLT        = MakeUintFrameID3('S', 'L', 'T'),        // SLT : sync lyrics

    ID3V2_2_YEAR2        = MakeUintFrameID3('T', 'O', 'R'),        // TOR : Year

    ID3V2_2_LENGTH        = MakeUintFrameID3('T', 'L', 'E'),        // TLE : length of the audiofile in milliseconds
    ID3V2_2_PART_OF_SET    = MakeUintFrameID3('T', 'P', 'A'),        // TPA : Part of a set, Disknumber(iTunes)
};

enum ID3v2_3_GENERAL_ID
{
    ID3V2_3_COMPOSER    = MakeUintFrameID4('T', 'C', 'O', 'M'),        // TCOM    : Composer
    ID3V2_3_TPOS        = MakeUintFrameID4('T', 'P', 'O', 'S'),        // TPOS :
    ID3V2_3_ARTIST        = MakeUintFrameID4('T', 'P', 'E', '1'),        // TPE1 : Artist
    ID3V2_3_ALBUMARTIST    = MakeUintFrameID4('T', 'P', 'E', '2'),        // TPE2 : Album artist
    ID3V2_3_ALBUM        = MakeUintFrameID4('T', 'A', 'L', 'B'),        // TALB : Album
    ID3V2_3_TITLE        = MakeUintFrameID4('T', 'I', 'T', '2'),        // TIT2 : Title
    ID3V2_3_TRACK        = MakeUintFrameID4('T', 'R', 'C', 'K'),        // TRCK : Track
    ID3V2_3_YEAR        = MakeUintFrameID4('T', 'D', 'R', 'C'),        // TDRC : Year
    ID3V2_3_USER_TEXT    = MakeUintFrameID4('T', 'X', 'X', 'X'),        // TXXX : User defined text information frame
    ID3V2_3_COMMENT        = MakeUintFrameID4('C', 'O', 'M', 'M'),        // COMM : Comment
    ID3V2_3_GENRE        = MakeUintFrameID4('T', 'C', 'O', 'N'),        // TCON : Genre
    ID3V2_3_PICTURE        = MakeUintFrameID4('A', 'P', 'I', 'C'),        // APIC : pic
    ID3V2_3_ENCODED        = MakeUintFrameID4('T', 'E', 'N', 'C'),        // TENC : encoded
    ID3V2_3_URL            = MakeUintFrameID4('W', 'X', 'X', 'X'),        // WXXX : url
    ID3V2_3_COPYRIGHT    = MakeUintFrameID4('T', 'C', 'O', 'P'),        // TCOP : copy right
    ID3V2_3_ORIG_ARTIST    = MakeUintFrameID4('T', 'O', 'P', 'E'),        // TOPE : Orig. artist
    ID3V2_3_PUBLISHER    = MakeUintFrameID4('T', 'P', 'U', 'B'),        // TPUB : Publisher
    ID3V2_3_BPM            = MakeUintFrameID4('T', 'B', 'P', 'M'),        // TBPM : bpm
    ID3V2_3_USLT        = MakeUintFrameID4('U', 'S', 'L', 'T'),        // USLT : unsync lyrics
    ID3V2_3_SYLT        = MakeUintFrameID4('S', 'Y', 'L', 'T'),        // SYLT : sync lyrics

    ID3V2_3_YEAR2        = MakeUintFrameID4('T', 'Y', 'E', 'R'),        // TYER : Year
};

struct ID3v2FrameHdr
{
    ID3v2FrameHdr() :
    bTagAlterPreservation(false),
    bFileAlterPreservation(false),
    bReadOnly(false),
    bGroupingIdentity(false),
    bCompression(false),
    bEncryption(false),
    bUnsyncronisation(false),
    bDataLengthIndicator(false),
    frameUID(-1)
    {
        memset(byFlags, 0, sizeof(byFlags));
        //memset(szFrameID, 0, sizeof(szFrameID));
    }
    uint32_t        nFrameID;
    // char        szFrameID[5];
    int            nLen;
    uint8_t        byFlags[2];

    bool        bTagAlterPreservation;
    bool        bFileAlterPreservation;
    bool        bReadOnly;
    bool        bGroupingIdentity;
    bool        bCompression;
    bool        bEncryption;
    bool        bUnsyncronisation;
    bool        bDataLengthIndicator;

    ID3v2FrameUID_t    frameUID;

    //uint32_t        ID;

};

#pragma pack(pop)

// #define ID3V2Size(bySize)    (bySize[3] | (bySize[2] << 7) | (bySize[1] << 14) | (bySize[0] << 21))

uint32_t synchDataToUInt(uint8_t *byData, int nLen);
void synchDataFromUInt(uint32_t value, uint8_t *byData, int nLen);
uint32_t byteDataToUInt(uint8_t *byData, int nLen);
void byteDataFromUInt(uint32_t value, uint8_t *byData, int nLen);

/*
An ID3v2 tag can be detected with the following pattern:
$49 44 33 yy yy xx zz zz zz zz
Where yy is less than $FF, xx is the 'flags' uint8_t and zz is less than $80. 
 */

class CID3v2Frame;
class CID3v2FrameFactoryBase;

class ID3v2Text
{
public:
    ID3v2Text()
    {
        m_EncodingType = IET_ANSI;
    }
    ID3v2EncType    m_EncodingType;
    string        m_str;

    void setValue(cstr_t szValue);
    void setValueAndOptimizeEncoding(cstr_t szValue);
    cstr_t getValue();

};

class ID3v2TextUserDefined
{
public:
    ID3v2TextUserDefined()
    {
        m_EncodingType = IET_ANSI;
    }
    ID3v2EncType    m_EncodingType;
    string        m_strValue, m_strDesc;

    void setValue(cstr_t szDescription, cstr_t szValue);
    void setValueAndOptimizeEncoding(cstr_t szDescription, cstr_t szValue);
    cstr_t getValue();
    cstr_t getDescription();

};

class CID3v2
{
public:
    CID3v2(CHAR_ENCODING encoding);
    virtual ~CID3v2();

    bool isID3v2_2() const { return m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V2; }

    virtual int open(cstr_t szFile, bool bModify, bool bCreate = false);
    virtual int open(FILE *fp, bool bCreate = false);
    void close();

    int save();

    // remove the whole ID3v2 tag
    int removeTagAndClose();

public:
    typedef list<CID3v2Frame*>        FRAME_LIST;
    typedef FRAME_LIST::iterator    FrameIterator;

    FrameIterator frameBegin() { return m_listFrames.begin(); }
    FrameIterator frameEnd() { return m_listFrames.end(); }

    int frameAdd(CID3v2Frame *pFrame);
    FrameIterator frameRemove(FrameIterator itToRemove);

    FrameIterator findFrameIter(uint32_t nFrameID);
    CID3v2Frame *findFrame(uint32_t nFrameID);
    CID3v2Frame *findFrameByUID(ID3v2FrameUID_t frameUID, uint32_t nFrameID);

    int removeFrame(uint32_t nFrameID);
    int removeFrameByUID(ID3v2FrameUID_t frameUID);

protected:
    long findID3v2();

    int getBeginFramePos();

    int readAllFrames();
    int readFrame(CID3v2Frame *pFrame);

public:
    ID3v2Header    m_id3v2Header;
    ID3v2Footer    m_id3v2Footer;

protected:
    int            m_nId3v2BegPos;
    size_t        m_nHeaderTotalLen;
    size_t        m_nExtendedHeaderLen;
    bool        m_bCreateNew;
    bool        m_bModify;
    bool        m_bAttach;
    FILE        *m_fp;
    CHAR_ENCODING m_Encoding;

    FRAME_LIST    m_listFrames;
    // long        m_nextFramePos;

    ID3v2FrameUID_t    frameUID;

};

//
// ID3v2 Lyrics
//
class LrcSyllable
{
public:
    LrcSyllable()
    {
        bNewLine = false;
    }

    int            nTime;
    bool        bNewLine;
    string        strText;
};

struct ID3v2SynchLyrics
{
    enum CONTENT_TYPE
    {
        CT_OTHER        = 0,
        CT_LYRICS        = 1,
        CT_TEXT_TRANS    = 2,
        CT_MOVEMENT        = 3,
        CT_EVENT        = 4,
        CT_CHORD        = 5
    };

    ID3v2SynchLyrics();

    // This Uid identified the position of the frame in the id3v2 tag.
    // It's used for update the tag.
    ID3v2FrameUID_t        frameUID;
    ID3v2FrameAction    action;

    bool                m_bAllSyllableIsNewLine;

    ID3v2EncType        m_encodingType;
    bool                m_bTimeStampMs;
    uint8_t                m_byContentType;

    char                m_szLanguage[10];
    string            m_strContentDesc;

    typedef list< LrcSyllable >        LIST_SYLRC;

    LIST_SYLRC            m_vSynLyrics;
};

struct ID3v2UnsynchLyrics
{
    ID3v2UnsynchLyrics();

    void setValueAndOptimizeEncoding(cstr_t szDescription, cstr_t szLyrics);

    // This Uid identified the position of the frame in the id3v2 tag.
    // It's used for update the tag.
    ID3v2FrameUID_t        frameUID;
    ID3v2FrameAction    action;

    ID3v2EncType        m_encodingType;
    char                m_szLanguage[10];
    string            m_strContentDesc, m_strLyrics;
};

class ID3v2Pictures
{
public:
    //    Picture type:
    enum PicType
    {
        PT_OTHER,        //          $00  Other
        PT_STD_ICON,    //       $01  32x32 pixels 'file icon' (PNG only)
        PT_OTHER_ICON,    //       $02  Other file icon
        PT_COVER_FRONT,    //       $03  Cover (front)
        PT_COVER_BACK,    //       $04  Cover (back)
        PT_LEAFLET_PAGE,//       $05  Leaflet page
        PT_MEDIA,        //       $06  Media (e.g. label side of CD)
        PT_LEAD_ARTIST,    //       $07  Lead artist/lead performer/soloist
        PT_ARTIST,        //       $08  Artist/performer
        PT_CONDUCTOR,    //       $09  Conductor
        PT_BAND,        //       $0A  Band/Orchestra
        PT_COMPOSER,    //       $0B  Composer
        PT_LYRICIST,    //       $0C  Lyricist/text writer
        PT_RECORD_LOC,    //       $0D  Recording Location
        PT_DURING_LOC,    //       $0E  During recording
        PT_DURING_PERF,    //       $0F  During performance
        PT_MOVIE_CAPTURE,//       $10  Movie/video screen capture
        PT_COLORED_FISH,//       $11  A bright coloured fish
        PT_ILLUSTRATION,//       $12  Illustration
        PT_BAND_LOGO,    //       $13  Band/artist logotype
        PT_PUBLISHER_LOGO,//       $14  Publisher/Studio logotype
    };

    struct ITEM
    {
        ITEM()
        {
            action = IFA_NONE;
            frameUID = -1;
        }
        ID3v2FrameUID_t        frameUID;
        ID3v2FrameAction    action;
        string                m_strMimeType;    // for pic
        PicType                m_picType;
        ID3v2Text            m_text;
        string            m_buffPic;

        void mimeToPicExt(char szPicExt[4]);
        bool picExtToMime(cstr_t szPicExt);
        bool setImageFile(cstr_t szImage);
    };
    typedef vector<ITEM*>            V_ITEMS;

    virtual ~ID3v2Pictures()
    {
        free();
    }

    int getCount() const { return (int)m_vItems.size(); }
    ITEM *appendNewPic();
    bool isModified();

    void free()
    {
        for (V_ITEMS::iterator it = m_vItems.begin();
            it != m_vItems.end(); it++)
            delete *it;
        m_vItems.clear();
    }

    static cstr_t *getAllPicDescriptions();
    static int getPicDescriptionsCount();

public:
    V_ITEMS                m_vItems;

};

#include "ID3v2Frame.h"

#endif // !defined(AFX_ID3V2_H__8F5C4F2D_B919_4876_B17B_FF2C4AD71904__INCLUDED_)
