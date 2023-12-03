#pragma once

#ifndef MediaTags_ID3_ID3v2Frame_h
#define MediaTags_ID3_ID3v2Frame_h


#include "ID3v2.h"


class CID3v2Frame {
public:
    CID3v2Frame(uint32_t frameID);
    CID3v2Frame(const ID3v2FrameHdr &frameHdr, const char *data, int len);
    virtual ~CID3v2Frame();

    uint32_t getFrameID() const { return m_framehdr.nFrameID; }
    int renderFrame(const ID3v2Header *pHeader, string &buff);

    // string *getData() { return &m_frameData; }
    //    char *data() { return m_frameData.data(); }
    //    size_t size() { return m_frameData.size(); }

#ifdef DEBUG
    virtual void debugOutput();
#endif

protected:
    int renderFrameHdr(const ID3v2Header *pHeader, string &buff, uint32_t lengthData);

public:
    ID3v2FrameHdr               m_framehdr;
    string                      m_frameData;

};

#endif // !defined(MediaTags_ID3_ID3v2Frame_h)
