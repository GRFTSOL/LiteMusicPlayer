#pragma once

#include "MediaTagTypes.hpp"


//////////////////////////////////////////////////////////////////////////
// References:
// http://atomicparsley.sourceforge.net/mpeg-4files.html
//////////////////////////////////////////////////////////////////////////

class M4aBox {
public:
    M4aBox(cstr_t szType, size_t offset, size_t size);
    virtual ~M4aBox();

    typedef vector<M4aBox*> VecBoxes;
    enum {
        TYPE_SIZE                   = 4,
        HEADER_SIZE                 = 8,
    };

    void addChild(M4aBox *box);

    bool isType(cstr_t szType);

    M4aBox *findBox(cstr_t szBox);
    M4aBox *nextBox(M4aBox *brother);

    bool deleteChild(cstr_t szType);

    void clearChildren();

    void toBuf(string &buf);

    void calculateNewSize();

    size_t getDataSize() { return m_nSize - HEADER_SIZE; }

public:
    size_t                      m_nOffset;          // The offset of the box
    size_t                      m_nSize;            // The size of the whole box.
    char                        m_szType[TYPE_SIZE + 1];
    string                      m_data;

    size_t                      m_nSizeNew;

    VecBoxes                    m_vChildren;

};

class CM4aTag {
public:
    CM4aTag();
    virtual ~CM4aTag();

    int open(cstr_t szFile, bool bModify, bool bCreate = false);
    int open(FILE *fp, bool bCreate = false);
    void close();
    int saveClose();

    int getTags(BasicMediaTags &tags);

    int listLyrics(VecStrings &vLyrNames);
    int getLyrics(string &strLyrics);

    bool hasLyrics();
    int setLyrics(cstr_t szLyrics);
    int removeLyrics();

    M4aBox &getRoot() { return m_root; }

    M4aBox *getBox(cstr_t szPath);

    uint32_t getChannelCount() { return m_channelCount; }
    uint32_t getBitsPerSample() { return m_bitsPerSample; }
    uint32_t getSampleRate() { return m_sampleRate; }
    uint32_t getBitRate() { return m_bitRate; }
    uint32_t getDuration() { return m_duration; }

protected:
    int parse(M4aBox *parent, int nExtraHeaderOffset = M4aBox::HEADER_SIZE);
    void parseSTSD(M4aBox *box);
    void parseMDHD(M4aBox *box);
    void parseMVHD(M4aBox *box);

    int adjustStco(int nAdjustedOffset);

    int read(M4aBox *parent);
    int write(string &buf, size_t offset);

    FILE                        *m_fp;
    M4aBox                      m_root;             // The virtual root box

    uint32_t                    m_channelCount = 0;
    uint32_t                    m_bitsPerSample = 0;
    uint32_t                    m_sampleRate = 0;
    uint32_t                    m_bitRate = 0;
    uint32_t                    m_duration = 0;

};
