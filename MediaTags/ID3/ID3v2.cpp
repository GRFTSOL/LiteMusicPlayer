#include "ID3v2.h"
#include <bitset>
#include "ID3v2FrameParser.h"
#include "ID3v2Frame.h"
#include "ID3Helper.h"
#include "ID3v1.h"


CharEncodingType iD3v2EncTypeToCharEncoding(ID3v2EncType encType) {
    switch (encType) {
    case IET_UCS2LE_BOM:
        return ED_UNICODE;
    case IET_UCS2BE_NO_BOM:
        return ED_UNICODE_BIG_ENDIAN;
    case IET_UTF8:
        return ED_UTF8;
    case IET_ANSI:
        return ED_SYSDEF;
    default:
        assert(0);
        return ED_SYSDEF;
    }
}

ID3v2EncType charEncodingToID3v2EncType(CharEncodingType encoding) {
    switch (encoding) {
    case ED_UNICODE:
        return IET_UCS2LE_BOM;
    case ED_UNICODE_BIG_ENDIAN:
        return IET_UCS2BE_NO_BOM;
    case ED_UTF8:
        return IET_UTF8;
    default:
        return IET_ANSI;
    }
}

cstr_t _ID3v2PicDescription[] = {
    "Other",
    "32x32 pixels 'file icon' (PNG only)",
    "Other file icon",
    "Cover (front)",
    "Cover (back)",
    "Leaflet page",
    "Media (e.g. label side of CD)",
    "Lead artist/lead performer/soloist",
    "Artist/performer",
    "Conductor",
    "Band/Orchestra",
    "Composer",
    "Lyricist/text writer",
    "Recording Location",
    "During recording",
    "During performance",
    "Movie/video screen capture",
    "A bright coloured fish",
    "Illustration",
    "Band/artist logotype",
    "Publisher/Studio logotype"
};

ID3v2SynchLyrics::ID3v2SynchLyrics() {
    frameUID = -1;
    action = IFA_NONE;
    m_bTimeStampMs = true;
    m_byContentType = CT_LYRICS;
    strcpy(m_szLanguage, "eng");
    m_bAllSyllableIsNewLine = true;
    m_encodingType = IET_UCS2LE_BOM;
}

ID3v2UnsynchLyrics::ID3v2UnsynchLyrics() {
    frameUID = -1;
    action = IFA_NONE;
    strcpy(m_szLanguage, "eng");
    m_encodingType = IET_ANSI;
}

void ID3v2UnsynchLyrics::setValueAndOptimizeEncoding(cstr_t szDescription, cstr_t szLyrics) {
    if (isAnsiStr(szDescription) && isAnsiStr(szLyrics)) {
        m_encodingType = IET_ANSI;
    } else {
        m_encodingType = IET_UCS2LE_BOM;
    }

    m_strContentDesc = szDescription;
    m_strLyrics = szLyrics;
}

/*
image/jpeg    image/tiff    image/gif    image/bmp    image/png    
*/

#define MIME_IMAGE          "image/"
void ID3v2Pictures::ITEM::mimeToPicExt(char szPicExt[4]) {
    memset(szPicExt, 0, sizeof(char) * 4);

    int nMimeImage = strlen(MIME_IMAGE);
    if (strncmp(m_strMimeType.c_str(), MIME_IMAGE, nMimeImage) == 0) {
        //
        if (strcmp(m_strMimeType.c_str() + nMimeImage, "jpeg") == 0) {
            strcpy(szPicExt, "jpg");
        } else if (strcmp(m_strMimeType.c_str() + nMimeImage, "tiff") == 0) {
            strcpy(szPicExt, "tif");
        } else {
            strncpy(szPicExt, m_strMimeType.c_str() + nMimeImage, 3);
        }
        toUpper(szPicExt);
        return;
    }
    strncpy(szPicExt, m_strMimeType.c_str(), 3);
    toUpper(szPicExt);
}

bool ID3v2Pictures::ITEM::picExtToMime(cstr_t szPicExt) {
    if (*szPicExt == '.') {
        szPicExt++;
    }

    m_strMimeType = MIME_IMAGE;
    if (strcasecmp(szPicExt, "jpg") == 0 || strcasecmp(szPicExt, "jpeg") == 0) {
        m_strMimeType += "jpeg";
    } else if (strcasecmp(szPicExt, "tif") == 0) {
        m_strMimeType += "tiff";
    } else if (strcasecmp(szPicExt, "png") == 0 || strcasecmp(szPicExt, "gif") == 0
        || strcasecmp(szPicExt, "bmp") == 0) {
        m_strMimeType += szPicExt;
        toLower((char *)m_strMimeType.c_str());
    } else {
        m_strMimeType.resize(0);
        return false;
    }

    return true;
}

bool ID3v2Pictures::ITEM::setImageFile(cstr_t szImage) {
    if (!picExtToMime(fileGetExt(szImage))) {
        return false;
    }

    return readFile(szImage, m_buffPic);
}

ID3v2Pictures::ITEM *ID3v2Pictures::appendNewPic() {
    ITEM *pic = new ITEM;

    pic->m_picType = PT_COVER_FRONT;
    pic->action = IFA_ADD;
    m_vItems.push_back(pic);

    return pic;
}

bool ID3v2Pictures::isModified() {
    for (size_t i = 0; i < m_vItems.size(); i++) {
        if (m_vItems[i]->action != IFA_NONE) {
            return true;
        }
    }

    return false;
}

cstr_t *ID3v2Pictures::getAllPicDescriptions() {
    return _ID3v2PicDescription;
}

int ID3v2Pictures::getPicDescriptionsCount() {
    return CountOf(_ID3v2PicDescription);
}


CID3v2::CID3v2(CharEncodingType encoding) {
    m_Encoding = encoding;
    m_bAttach = false;
    m_bCreateNew = false;
    m_fp = nullptr;
    memset(&m_id3v2Header, 0, sizeof(m_id3v2Header));
    memset(&m_id3v2Footer, 0, sizeof(m_id3v2Footer));
    m_nId3v2BegPos = 0;
    m_nHeaderTotalLen = 0;
    m_nExtendedHeaderLen = 0;
    m_bModify = false;
    frameUID = 0;
}

CID3v2::~CID3v2() {
    close();
}

void CID3v2::close() {
    if (m_fp) {
        if (!m_bAttach) {
            fclose(m_fp);
        }
        m_fp = nullptr;
    }

    for (FrameIterator it = frameBegin(); it != frameEnd(); ++it) {
        delete *it;
    }
    m_listFrames.clear();
}

int CID3v2::findID3v2() {
    // ID3v2 tag must be in the first 4k uint8_t.
    const int ID3V2_TAG_IN_RANGE = 1024 * 8;

    const int TAG_SIZE = strlen(ID3_TAGID);
    char buf[ID3V2_TAG_IN_RANGE];
    size_t size = 0;

    // Read enough data one time.
    while (size <= ID3V2_TAG_IN_RANGE) {
        int n = (int)fread(buf + size, 1, sizeof(buf) - size, m_fp);
        if (n <= 0) {
            break;
        }
        size += n;
    }

    const char *data = buf, *dataend = buf + size - TAG_SIZE - 1;
    for (; data < dataend; data++) {
        // Here we have to loop because there could be several of the first
        // (11111111) uint8_t, and we want to check all such instances until we find
        // a full match (11111111 111) or hit the end of the buffer.
        if (uint8_t(*data) == 0xFF) {
            data++;
            if (uint8_t(*data) != 0xFF && (uint8_t(*data) & 0xE0) == 0xE0) {   // 111xxxxx
                return -1;
            }
        }

        if (*data == ID3_TAGID[0]) {
            if (strncmp(data, ID3_TAGID, TAG_SIZE) == 0) {
                return int(data - buf);
            }
        }
    }

    return -1;
}

int CID3v2::open(cstr_t szFile, bool bModify, bool bCreate) {
    if (bModify) {
        m_fp = fopen(szFile, "r+b");
    } else {
        m_fp = fopen(szFile, "rb");
    }

    if (!m_fp) {
        setCustomErrorDesc(stringPrintf("%s: %s", (cstr_t)OSError(), szFile).c_str());
        return ERR_CUSTOM_ERROR;
    }

    m_bAttach = false;

    return open(m_fp, bCreate);
}

int CID3v2::open(FILE *fp, bool bCreate) {
#define E_RETURN(code)      { nRet = code; goto R_ERROR; }
    assert(fp);
    if (m_fp != fp) {
        m_fp = fp;
        m_bAttach = true;
    }

    int nRet;

    m_bCreateNew = false;
    frameUID = 0;

    fseek(fp, 0, SEEK_SET);

    m_nId3v2BegPos = findID3v2();
    if (m_nId3v2BegPos == -1) {
        if (bCreate) {
            // not exist id3v2 tag, create one.
            m_bCreateNew = true;

            m_nHeaderTotalLen = 0;
            m_nId3v2BegPos = 0;
            m_nExtendedHeaderLen = 0;

            memcpy(m_id3v2Header.szID, ID3_TAGID, ID3_TAGIDSIZE);
            m_id3v2Header.byMajorVer = ID3v2Header::ID3V2_V3;
            m_id3v2Header.byRevisionVer = 0;
            m_id3v2Header.byFlag = 0;

            return ERR_OK;
        } else {
            E_RETURN(ERR_NOT_FOUND_ID3V2);
        }
    }

    fseek(m_fp, m_nId3v2BegPos, SEEK_SET);

    // read Header (10 bytes)
    if (fread(&m_id3v2Header, 1, sizeof(m_id3v2Header), m_fp) != sizeof(m_id3v2Header)) {
        E_RETURN(ERR_READ_FILE);
    }

    m_nHeaderTotalLen = synchDataToUInt(m_id3v2Header.bySize, CountOf(m_id3v2Header.bySize))
    + ID3_TAGHEADERSIZE;

    if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V2) {
        // DBG_LOG1("ID3V2 v2, Len: %d", m_nHeaderTotalLen);
    } else if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V3) {
        // DBG_LOG1("ID3V2 v3, Len: %d", m_nHeaderTotalLen);
    } else if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V4) {
        // DBG_LOG1("ID3V2 v4, Len: %d", m_nHeaderTotalLen);
    } else {
        E_RETURN(ERR_NOT_SUPPORT_ID3V2_VER);
    }

    // read Extended Header (variable length, OPTIONAL)
    if (m_id3v2Header.isExtendedFlagSet()) {
        ID3v2HeaderExtended extendedHeader;

        if (fread(&extendedHeader, 1, sizeof(extendedHeader), m_fp) != sizeof(extendedHeader)) {
            E_RETURN(ERR_READ_FILE);
        }

        m_nExtendedHeaderLen = synchDataToUInt(extendedHeader.bySize, CountOf(extendedHeader.bySize));
        fseek(m_fp, m_nExtendedHeaderLen, SEEK_CUR);
    } else {
        m_nExtendedHeaderLen = 0;
    }

    // read Footer (variable length, OPTIONAL)
    if (m_id3v2Header.isFooterFlagSet()) {
        fseek(m_fp, m_nId3v2BegPos + m_nHeaderTotalLen, SEEK_SET);
        if (fread(&m_id3v2Footer, 1, sizeof(m_id3v2Footer), m_fp) != sizeof(m_id3v2Footer)) {
            E_RETURN(ERR_READ_FILE);
        }
        m_nHeaderTotalLen += ID3_TAGFOOTERSIZE;
    }

    // read all frames
    nRet = readAllFrames();
    if (nRet != ERR_OK) {
        goto R_ERROR;
    }

    return ERR_OK;

R_ERROR:
    if (m_fp) {
        fclose(m_fp);
        m_fp = nullptr;
    }
    return nRet;
}

int CID3v2::save() {
    string buff;
    int nRet;

    for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it) {
        CID3v2Frame *pFrame = *it;

        nRet = pFrame->renderFrame(&m_id3v2Header, buff);
        if (nRet != ERR_OK) {
            return nRet;
        }
    }

    auto nNewHeaderTotalLen = (int)buff.size() + ID3_TAGHEADERSIZE + m_nExtendedHeaderLen;
    if (m_id3v2Header.isFooterFlagSet()) {
        nNewHeaderTotalLen += ID3_TAGFOOTERSIZE;
    }

    if (nNewHeaderTotalLen > (int)m_nHeaderTotalLen) {
        if (m_id3v2Header.isFooterFlagSet()) {
            // assert(GetPaddingLen() == 0);
            // must not have padding
        } else {
            // add more 512 to 1024 uint8_t for backup modify or add frame...
            nNewHeaderTotalLen += 1024;
            nNewHeaderTotalLen -= nNewHeaderTotalLen % 512;
        }
    } else if (nNewHeaderTotalLen < (int)m_nHeaderTotalLen) {
        if (m_id3v2Header.isFooterFlagSet()) {
            // assert(GetPaddingLen() == 0);
            // must not have padding
        } else {
            // don't move mp3 data, keep padding
            nNewHeaderTotalLen = m_nHeaderTotalLen;
        }
    }

    if (nNewHeaderTotalLen != m_nHeaderTotalLen) {
        // move mp3 data, alloc or free header size.
        nRet = fileMoveEndData(m_fp, m_nHeaderTotalLen, nNewHeaderTotalLen);
        if (nRet != ERR_OK) {
            return nRet;
        }

        m_nHeaderTotalLen = nNewHeaderTotalLen;
    }

    auto nPaddingSize = nNewHeaderTotalLen - buff.size() - m_nExtendedHeaderLen - ID3_TAGHEADERSIZE;
    if (m_id3v2Header.isFooterFlagSet()) {
        nPaddingSize -= ID3_TAGFOOTERSIZE;
    }
    assert(nPaddingSize >= 0);

    {
        // set header new size
        auto nSizeInHeader = nNewHeaderTotalLen - ID3_TAGHEADERSIZE;
        if (m_id3v2Header.isFooterFlagSet()) {
            nSizeInHeader -= ID3_TAGFOOTERSIZE;
        }
        synchDataFromUInt((uint32_t)nSizeInHeader, m_id3v2Header.bySize, CountOf(m_id3v2Header.bySize));
    }

    if (!m_bCreateNew) {
        // compare whether the id3v2 data has been modified?
        int nResult;
        nRet = fileDataCmp(m_fp, m_nId3v2BegPos, &m_id3v2Header, sizeof(m_id3v2Header), nResult);
        if (nRet != ERR_OK) {
            return nRet;
        }

        if (nResult == 0) {
            nRet = fileDataCmp(m_fp, getBeginFramePos(),
                buff.c_str(), (int)buff.size(), nResult);
            if (nRet != ERR_OK) {
                return nRet;
            }
            if (nResult == 0) {
                //
                // 如果padding data 不为0，需要reset.
                if (nPaddingSize > 0) {
                    size_t n = 4;
                    char temp[4];
                    if (nPaddingSize < 4) {
                        n = nPaddingSize;
                    }
                    fseek(m_fp, getBeginFramePos() + buff.size(), SEEK_SET);
                    if (fread(temp, 1, n, m_fp) != n) {
                        return ERR_READ_FILE;
                    }
                    if (memcmp(temp, "\0\0\0\0", n) != 0) {
                        return fileDataReset(m_fp, getBeginFramePos() + buff.size(), nPaddingSize, 0);
                    }
                }
                return ERR_OK;
            }
        }
    }

    // must have least one frame...
    if (m_bCreateNew && buff.size() == 0) {
        return ERR_OK;
    }

    // save id3info
    if (fseek(m_fp, m_nId3v2BegPos, SEEK_SET) != 0) {
        return ERR_SEEK_FILE;
    }

    // header
    if (fwrite(&m_id3v2Header, 1, sizeof(m_id3v2Header), m_fp) != sizeof(m_id3v2Header)) {
        return ERR_WRITE_FILE;
    }

    // extended header
    // not modified, position not changed
    if (m_id3v2Header.isExtendedFlagSet()) {
        if (fseek(m_fp, m_nExtendedHeaderLen, SEEK_CUR) != 0) {
            return ERR_SEEK_FILE;
        }
    }

    // frames
    if (fwrite(buff.c_str(), 1, buff.size(), m_fp) != buff.size()) {
        return ERR_WRITE_FILE;
    }

    // padding
    nRet = fileDataReset(m_fp, ftell(m_fp), nPaddingSize, 0);
    if (nRet != ERR_OK) {
        return nRet;
    }

    // footer
    if (m_id3v2Header.isFooterFlagSet()) {
        if (fwrite(&m_id3v2Footer, 1, sizeof(m_id3v2Footer), m_fp) != sizeof(m_id3v2Footer)) {
            return ERR_WRITE_FILE;
        }
    }

    return ERR_OK;
}

int CID3v2::removeTagAndClose() {
    if (m_nHeaderTotalLen <= 0) {
        return ERR_NOT_FOUND_ID3V2;
    }

    int nRet = fileMoveEndData(m_fp, m_nId3v2BegPos + m_nHeaderTotalLen, m_nId3v2BegPos);
    if (nRet == ERR_OK) {
        close();
    }

    return nRet;
}

int CID3v2::readAllFrames() {
    int nRet;
    CID3v2Frame *pFrame;

    auto nNextFramePos = getBeginFramePos();
    fseek(m_fp, nNextFramePos, SEEK_SET);

    pFrame = new CID3v2Frame(0);
    while ((nRet = readFrame(pFrame)) == ERR_OK) {
#ifdef DEBUG
        //        pFrame->debugOutput();
#endif
        m_listFrames.push_back(pFrame);
        pFrame = new CID3v2Frame(0);
    }

    // free latest allocated mem
    delete pFrame;

    if (nRet == ERR_EOF) {
        return ERR_OK;
    }

    return nRet;
}

int CID3v2::readFrame(CID3v2Frame *pFrame) {
    ID3v2FrameHdr &frame = pFrame->m_framehdr;
    string &buffData = pFrame->m_frameData;

    frame.frameUID = frameUID;
    frameUID++;

    auto nNextFramePos = ftell(m_fp);
    if (nNextFramePos > (int)m_nHeaderTotalLen) {
        // DBG_LOG2("read out of the len, %d, %d", ftell(m_fp), m_nHeaderTotalLen);
        return ERR_EOF;
    } else if (nNextFramePos == m_nHeaderTotalLen) {
        // DBG_LOG1("End of read frame %d", m_nHeaderTotalLen);
        return ERR_EOF;
    }

    if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V2) {
        ID3v2FrameHeaderV2 fh2;
        auto nRet = fread(&fh2, 1, sizeof(fh2), m_fp);
        if (nRet != sizeof(fh2)) {
            return ERR_READ_FILE;
        }
        frame.nFrameID = fh2.toFrameUintID();
        frame.nLen = byteDataToUInt(fh2.bySize, CountOf(fh2.bySize));
    } else if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V3) {
        ID3v2FrameHeaderV3 fh3;
        auto nRet = fread(&fh3, 1, sizeof(fh3), m_fp);
        if (nRet != sizeof(fh3)) {
            return ERR_READ_FILE;
        }
        frame.nFrameID = fh3.toFrameUintID();
        frame.nLen = byteDataToUInt(fh3.bySize, CountOf(fh3.bySize));

        assert(CountOf(frame.byFlags) == CountOf(fh3.byFlags));
        memcpy(frame.byFlags, fh3.byFlags, CountOf(fh3.byFlags));

        {
            // first uint8_t of flags
            std::bitset<8> flags(fh3.byFlags[0]);
            frame.bTagAlterPreservation = flags[7];
            frame.bFileAlterPreservation = flags[6];
            frame.bReadOnly = flags[5];
        }
        {
            // second uint8_t of flags
            std::bitset<8> flags(fh3.byFlags[1]);
            frame.bCompression = flags[7];
            frame.bEncryption = flags[6];
            frame.bGroupingIdentity = flags[5];
        }
    } else if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V4) {
        ID3v2FrameHeaderV3 fh3;
        auto nRet = fread(&fh3, 1, sizeof(fh3), m_fp);
        if (nRet != sizeof(fh3)) {
            return ERR_READ_FILE;
        }
        frame.nFrameID = fh3.toFrameUintID();
        frame.nLen = synchDataToUInt(fh3.bySize, CountOf(fh3.bySize));

        assert(CountOf(frame.byFlags) == CountOf(fh3.byFlags));
        memcpy(frame.byFlags, fh3.byFlags, CountOf(fh3.byFlags));

        {
            // first uint8_t of flags
            std::bitset<8> flags(fh3.byFlags[0]);
            frame.bTagAlterPreservation = flags[6];
            frame.bFileAlterPreservation = flags[5];
            frame.bReadOnly = flags[4];
        }
        {
            // second uint8_t of flags
            std::bitset<8> flags(fh3.byFlags[1]);
            frame.bGroupingIdentity = flags[6];
            frame.bCompression = flags[3];
            frame.bEncryption = flags[2];
            frame.bUnsyncronisation = flags[1];
            frame.bDataLengthIndicator = flags[0];
        }
    } else {
        return ERR_NOT_SUPPORT_ID3V2_VER;
    }

    if (frame.nFrameID <= 0xFFFFFF) {
        // DBG_LOG2("Encounter End Frame: %d, %d", ftell(m_fp), m_nHeaderTotalLen);
        return ERR_EOF;
    }

    // Check for IsValidFrame length?
    if (frame.nLen < 0 || ftell(m_fp) + frame.nLen > getBeginFramePos() + (int)m_nHeaderTotalLen) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    buffData.resize(frame.nLen);
    if (fread((char *)buffData.data(), 1, frame.nLen, m_fp) != frame.nLen) {
        buffData.clear();
        return ERR_READ_FILE;
    }

    return ERR_OK;
}

int CID3v2::frameAdd(CID3v2Frame *pFrame) {
    m_listFrames.push_back(pFrame);

    return ERR_OK;
}

CID3v2::FrameIterator CID3v2::frameRemove(FrameIterator itToRemove) {
    return m_listFrames.erase(itToRemove);
}

CID3v2::FrameIterator CID3v2::findFrameIter(uint32_t nFrameID) {
    for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it) {
        CID3v2Frame *pFrame = *it;
        if (pFrame->m_framehdr.nFrameID == nFrameID) {
            return it;
        }
    }

    return m_listFrames.end();
}

CID3v2Frame *CID3v2::findFrame(uint32_t nFrameID) {
    FrameIterator it = findFrameIter(nFrameID);
    if (it == m_listFrames.end()) {
        return nullptr;
    }
    return *it;
}

CID3v2Frame *CID3v2::findFrameByUID(ID3v2FrameUID_t frameUID, uint32_t nFrameID) {
    if (frameUID == -1) {
        return nullptr;
    }

    FrameIterator it;
    for (it = m_listFrames.begin(); it != m_listFrames.end(); ++it) {
        CID3v2Frame *pFrame = *it;
        if (pFrame->m_framehdr.frameUID == frameUID && pFrame->m_framehdr.nFrameID == nFrameID) {
            return pFrame;
        }
    }

    return nullptr;
}

int CID3v2::removeFrame(uint32_t nFrameID) {
    CID3v2Frame *pFrame = nullptr;
    FrameIterator it;

    for (it = frameBegin(); it != frameEnd(); ++it) {
        pFrame = *it;

        if (pFrame->m_framehdr.nFrameID == nFrameID) {
            frameRemove(it);
            return ERR_OK;
        }
    }

    return ERR_NOT_FOUND;
}

int CID3v2::removeFrameByUID(ID3v2FrameUID_t frameUID) {
    CID3v2Frame *pFrame = nullptr;
    FrameIterator it;

    for (it = frameBegin(); it != frameEnd(); ++it) {
        pFrame = *it;

        if (pFrame->m_framehdr.frameUID == frameUID) {
            frameRemove(it);
            return ERR_OK;
        }
    }

    return ERR_NOT_FOUND;
}

int CID3v2::getBeginFramePos() {
    return (int)(m_nId3v2BegPos + ID3_TAGHEADERSIZE + m_nExtendedHeaderLen);
}
