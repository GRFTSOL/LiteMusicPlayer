#pragma once

#include "../Utils/Utils.h"


enum LyrTagType {
    LTT_UNKNOWN                 = 0,
    LTT_ARTIST                  = 1,
    LTT_TITLE                   = 1 << 1,
    LTT_ALBUM                   = 1 << 2,
    LTT_BY                      = 1 << 3,
    LTT_OFFSET                  = 1 << 4,
    LTT_MEDIA_LENGTH            = 1 << 5,
    LTT_ID                      = 1 << 6,
    LTT_ENCODING                = 1 << 7,
    LTT_CONTENT_TYPE            = 1 << 8, // Check Time tag:[00:01.01], get LYRICS_CONTENT_TYPE
    LTT_MD5                     = 1 << 9, // get Md5 of lyrics content, NOT including properties.
    LTT_ALL_TAG                 = 0xFFFFFFFF & ~(LTT_CONTENT_TYPE | LTT_MD5),
    LTT_ALL                     = 0xFFFFFFFF,
};

// TXT < LRC < karaoke , precision of timestamps is higher
enum LYRICS_CONTENT_TYPE {
    LCT_UNKNOWN                 = 0,
    LCT_TXT                     = 1 << 1,
    LCT_LRC                     = 1 << 2,
    LCT_KARAOKE                 = 1 << 4, // karaoke content, full synced.

    LCT_ALL                     = LCT_TXT | LCT_LRC | LCT_KARAOKE,
};

void setDefaultLyricsEncoding(CharEncodingType encoding);

string convertLyricsToUtf8(uint8_t *text, size_t len, bool useSpecifiedEncoding, CharEncodingType encoding);

bool isTxtTagAtTailOfFile(LyrTagType tagType);

string formtLrcTimeTag(int time, bool isLongTimeFmt);
void formatPlayTime(int nmsTime, char szBuf[]);

bool compressLyrics(string &buf);

#define    SZ_LRC_AR        "ar:"
#define    SZ_LRC_TI        "ti:"
#define    SZ_LRC_AL        "al:"
#define    SZ_LRC_BY        "by:"
#define    SZ_LRC_ENCODING  "Encoding:"
#define    SZ_LRC_OFFSET    "offset:"
#define    SZ_LRC_LENGTH    "length:"
#define    SZ_LRC_ID        "id:"

#define    SZ_TXT_AR        "Artist:"
#define    SZ_TXT_TI        "Title:"
#define    SZ_TXT_AL        "Album:"
#define    SZ_TXT_ENCODING  "encoding:"
#define    SZ_TXT_OFFSET    "offset:"
#define    SZ_TXT_LENGTH    "length:"
#define    SZ_TXT_ID        "ID:"

struct LyricsProperties {
    LyricsProperties() {
        m_nTimeOffset = 0;
        m_lyrContentType = LCT_UNKNOWN;
    }

    bool equal(const LyricsProperties &other) const;
    void clear();

    void setOffsetTime(int nOffsetTime);
    void setOffsetTime(cstr_t szOfffsetTime) { m_strTimeOffset = szOfffsetTime; m_nTimeOffset = atoi(szOfffsetTime); }
    cstr_t getOffsetTimeStr() { return m_strTimeOffset.c_str(); }
    int getOffsetTime() { return m_nTimeOffset; }

    void setTxtLyrScrollEvents(cstr_t szTxtLyrScrollEvents) { m_strTimeOffset = szTxtLyrScrollEvents; }
    cstr_t getTxtLyrScrollEvents() { return m_strTimeOffset.c_str(); }

    void setMediaLength(int nMediaLength);
    int getMediaLengthInt() const;

    string                      m_strArtist;
    string                      m_strTitle;
    string                      m_strAlbum;
    string                      m_strBy;
    string                      m_strMediaLength;
    string                      m_strId;
    LYRICS_CONTENT_TYPE         m_lyrContentType;

protected:
    // offset time of timestamps in lyrics, in ms
    int                         m_nTimeOffset;
    string                      m_strTimeOffset;    // Also, it will store TxtLyrScrollEvents

};

class CLyrTagNameValueMap {
public:
    CLyrTagNameValueMap(LyricsProperties &prop, uint32_t tagMask = LTT_ALL_TAG);

    size_t getCount() const { return m_vItems.size(); }
    LyrTagType getTagType(int nIndex);
    const string &getLrcName(int nIndex);
    const string &getTxtName(int nIndex);
    void setValue(int nIndex, cwstr_t szValue, int nLen);
    void setValue(int nIndex, cstr_t szValue, int nLen);
    const string &getValue(int nIndex);
    void clearValue(int nIndex);

    bool shouldReadLyrContentType()
        { return m_prop.m_lyrContentType == LCT_UNKNOWN && isFlagSet(m_tagMask, LTT_CONTENT_TYPE); }
    void setLyrContentType(LYRICS_CONTENT_TYPE lyrContentType)
        { m_prop.m_lyrContentType = lyrContentType; }

    bool shouldReadMd5()
        { return isFlagSet(m_tagMask, LTT_MD5); }

    string getLrcTagString(int nIndex, bool bIncLineRet = true);
    string getTxtTagString(int nIndex, bool bIncLineRet = true);

    // Return value is the index of tag, -1, if no tag found.
    int parseTxtTag(cstr_t szText, int nLen);
    int parseLrcTag(cstr_t szText, int nLen);

    void remove(int nIndex);

    struct Item {
        string                      strLrcName;
        string                      strTxtName;
        LyrTagType                  tagType;
        string                      *pstrValue;
    };

protected:
    void onSetValue(int nIndex);

protected:
    uint32_t                    m_tagMask;
    vector<Item>                m_vItems;
    LyricsProperties            &m_prop;
    string                      m_strOffset;

};

class CLrcTag : public LyricsProperties {
public:
    CLrcTag(uint32_t tagMask = LTT_ALL_TAG) { m_tagMask = tagMask; }

    void setTagMask(uint32_t tagMask = LTT_ALL_TAG) { m_tagMask = tagMask; }

    // Used for writing, compare for changed tags, and save them.
    CLrcTag(LyricsProperties &orgProp, LyricsProperties &newProp, uint32_t tagMask = LTT_ALL_TAG);

    int readFromFile(cstr_t szFile);
    bool readFromText(uint8_t *str, size_t len, bool bUseSpecifiedEncoding = false, CharEncodingType encoding = ED_SYSDEF);
    bool readFromText(const char *str, size_t len);

    int writeToFile(cstr_t szFile, bool bLrcTag);
    void writeToBuffer(string &bufData, bool bLrcTag);
    void writeToText(string &str, bool bLrcTag);

    bool isTagChanged() { return m_tagMask != 0; }

    cstr_t getMD5() { return m_md5OfLyrContent.c_str(); }

protected:
    string                      m_md5OfLyrContent;
    uint32_t                    m_tagMask;

};
