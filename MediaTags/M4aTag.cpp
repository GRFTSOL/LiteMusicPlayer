#include "M4aTag.h"
#include "ID3/ID3v2.h"
#include "ID3/ID3Helper.h"
#include "MediaTags.h"


#define SZ_TYPE_MOOV        "moov"
#define SZ_TYPE_TRACK       "trak"
#define SZ_TYPE_UDTA        "udta"
#define SZ_TYPE_META        "meta"
#define SZ_TYPE_ILST        "ilst"
#define SZ_TYPE_LYRICS      "\xA9lyr"
#define SZ_TYPE_DATA        "data"


M4aBox *getBox(M4aBox *parent, cstr_t szPath) {
    VecStrings vPath;
    strSplit(szPath, '.', vPath);

    for (uint32_t i = 0; i < vPath.size(); i++) {
        parent = parent->findBox(vPath[i].c_str());
        if (!parent) {
            return nullptr;
        }
    }

    return parent;
}

bool canBoxHasChild(char szType[]) {
    cstr_t vList[] = { SZ_TYPE_MOOV, SZ_TYPE_TRACK, SZ_TYPE_UDTA, SZ_TYPE_META, SZ_TYPE_ILST, };

    for (int i = 0; i < CountOf(vList); i++) {
        if (strncmp(szType, vList[i], M4aBox::TYPE_SIZE) == 0) {
            return true;
        }
    }

    return false;
}

// https://developer.apple.com/library/archive/documentation/QuickTime/QTFF/Metadata/Metadata.html

// Data Atom Structure
/**
 * Type Indicator
 *    The type indicator, as defined in Type Indicator.
 * Locale Indicator
 *    The locale indicator, as defined in Locale Indicator.
 * Value
 *    An array of bytes containing the value of the metadata.
 */
class CM4aBoxParser {
public:
    CM4aBoxParser(M4aBox *box) : m_data(box->m_data) { }

    enum DataType {
        DT_CUSTOMIZE                = 0,
        DT_UTF8_STRING              = 1,
    };

    DataType getDataType() {
        if (m_data.size() < 4) {
            return DT_CUSTOMIZE;
        }

        return (DataType)uint32FromBE((uint8_t *)m_data.c_str());
    }

    string parseAsUtf8() {
        assert(getDataType() == DT_UTF8_STRING);
        if (getDataType() != DT_UTF8_STRING) {
            return "";
        }

        auto p = (cstr_t)m_data.c_str();
        auto end = p + m_data.size();

        // 4 bytes type, 4 bytes locale
        p += 8;

        string str;
        if (p < end) {
            str.assign(p, (size_t)(end - p));
        }
        return str;
    }

    vector<uint16_t> parseAsUint16Arr() {
        assert(getDataType() == DT_CUSTOMIZE);
        auto p = (uint8_t *)m_data.c_str();
        auto end = p + m_data.size();

        // 4 bytes type, 4 bytes locale
        p += 8;

        vector<uint16_t> a;

        // 用于测试
        // string s;
        // for (; p < end; p += 4) {
        //     s += itos(uint32FromBE(p)) + ",";
        // }
        // printf("N32: %s\n", s.c_str());
        //
        // s.clear(); p = (uint8_t *)m_data.c_str() + 8;
        // for (; p < end; p += 2) {
        //     s += itos(uint16FromBE(p)) + ",";
        // }
        // printf("N16: %s\n", s.c_str());
        //
        // s.clear(); p = (uint8_t *)m_data.c_str() + 8;
        // for (; p < end; p ++) {
        //     s += itos(*p) + ",";
        // }
        // printf("N8: %s\n", s.c_str());

        for (; p < end; p += 2) {
            a.push_back(uint16FromBE(p));
        }

        return a;
    }

    virtual int convertToText(cstr_t text) {
        if (m_data.size() < 8) {
            m_data.resize(8);
            memset((char *)m_data.c_str(), 0, m_data.size());
            m_data[3] = 1;
        } else {
            m_data.resize(8);
        }
        m_data.append(text);
        return ERR_OK;
    }

private:
    string                      &m_data;

};

//////////////////////////////////////////////////////////////////////////

M4aBox::M4aBox(cstr_t szType, size_t offset, size_t size) {
    m_nOffset = offset;
    m_nSize = size;
    strncpy_safe(m_szType, CountOf(m_szType), szType, M4aBox::TYPE_SIZE);

    assert(sizeof(m_szType) >= TYPE_SIZE);
    m_nSizeNew = 0;
}

M4aBox::~M4aBox() {
    clearChildren();
}

void M4aBox::addChild(M4aBox *box) {
    m_vChildren.push_back(box);
}

bool M4aBox::isType(cstr_t szType) {
    return strncmp(m_szType, szType, TYPE_SIZE) == 0;
}

void M4aBox::clearChildren() {
    for (uint32_t i = 0; i < m_vChildren.size(); i++) {
        delete m_vChildren[i];
    }
    m_vChildren.clear();
}

M4aBox *M4aBox::findBox(cstr_t szBox) {
    for (uint32_t i = 0; i < m_vChildren.size(); i++) {
        M4aBox *p = m_vChildren[i];
        if (p->isType(szBox)) {
            return p;
        }
    }

    return nullptr;
}

M4aBox *M4aBox::nextBox(M4aBox *brother) {
    for (uint32_t i = 0; i < m_vChildren.size(); i++) {
        M4aBox *p = m_vChildren[i];
        if (p == brother) {
            if (i + 1 < m_vChildren.size()) {
                return m_vChildren[i + 1];
            }
        }
    }

    return nullptr;
}

bool M4aBox::deleteChild(cstr_t szType) {
    for (uint32_t i = 0; i < m_vChildren.size(); i++) {
        M4aBox *p = m_vChildren[i];
        if (p->isType(szType)) {
            delete p;
            m_vChildren.erase(m_vChildren.begin() + i);
            return true;
        }
    }

    return false;
}

void M4aBox::toBuf(string &buf) {
    // size
    if (m_nSizeNew == 0) {
        calculateNewSize();
    }

    uint8_t size[4];
    uint32ToBE((uint32_t)m_nSizeNew, size);
    buf.append((char *)size, M4aBox::TYPE_SIZE);

    // Type
    buf.append(m_szType, M4aBox::TYPE_SIZE);

    if (m_vChildren.size() == 0) {
        buf.append(m_data);
        return;
    }

    // Data
    if (m_data.size() > 0) {
        assert(isType(SZ_TYPE_META));
        buf.append(m_data);
    }

    // Children
    for (uint32_t i = 0; i < m_vChildren.size(); i++) {
        M4aBox *p = m_vChildren[i];
        p->toBuf(buf);
    }
}

void M4aBox::calculateNewSize() {
    m_nSizeNew = HEADER_SIZE + m_data.size();

    for (uint32_t i = 0; i < m_vChildren.size(); i++) {
        M4aBox *p = m_vChildren[i];
        p->calculateNewSize();
        m_nSizeNew += p->m_nSizeNew;
    }
}

//////////////////////////////////////////////////////////////////////////

CM4aTag::CM4aTag() : m_root("", 0, 0) {
}

CM4aTag::~CM4aTag() {
    close();
}

int CM4aTag::open(cstr_t szFile, bool bModify, bool bCreate) {
    if (bModify) {
        m_fp = fopen(szFile, "r+b");
    } else {
        m_fp = fopen(szFile, "rb");
    }

    if (!m_fp) {
        setCustomErrorDesc(stringPrintf("%s: %s", (cstr_t)OSError(), szFile).c_str());
        return ERR_CUSTOM_ERROR;
    }

    return open(m_fp, bCreate);
}

int CM4aTag::open(FILE *fp, bool bCreate) {
    assert(fp);
    m_fp = fp;

    fseek(fp, 0, SEEK_END);
    size_t nFileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    m_root.m_nOffset = 0;
    m_root.m_nSize = nFileSize;
    int nRet = parse(&m_root, 0);
    if (nRet != ERR_OK) {
        return nRet;
    }

    // read moov box immediately
    M4aBox *boxMoov = m_root.findBox(SZ_TYPE_MOOV);
    if (boxMoov == nullptr) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    return read(boxMoov);
}

void CM4aTag::close() {
    if (m_fp) {
        fclose(m_fp);
        m_fp = nullptr;
    }
}

int CM4aTag::saveClose() {
    M4aBox *boxMoov = m_root.findBox(SZ_TYPE_MOOV);
    if (boxMoov == nullptr) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    if (getBox("moov.trak.mdia.minf.stbl.co64") != nullptr) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    boxMoov->calculateNewSize();

    long nAdjustedOffset = 0;
    size_t nSizeNew = boxMoov->m_nSizeNew;
    if (boxMoov->m_nSize != nSizeNew) {
        // Need to resize boxes
        M4aBox *free = m_root.nextBox(boxMoov);
        if (free == nullptr) {
            // resize
            int nRet = fileMoveEndData(m_fp, boxMoov->m_nOffset + boxMoov->m_nSize, (uint32_t)(boxMoov->m_nOffset + nSizeNew));
            if (nRet != ERR_OK) {
                return nRet;
            }
            nAdjustedOffset = nSizeNew - boxMoov->m_nSize;
        } else {
            if (boxMoov->m_nSize + free->m_nSize + M4aBox::HEADER_SIZE < nSizeNew) {
                // resize
                int nRet = fileMoveEndData(m_fp, boxMoov->m_nOffset + boxMoov->m_nSize, boxMoov->m_nOffset + nSizeNew);
                if (nRet != ERR_OK) {
                    return nRet;
                }
                nAdjustedOffset = nSizeNew - boxMoov->m_nSize;
            } else {
                // resize free box
                free->m_nOffset = boxMoov->m_nOffset + nSizeNew;
                free->m_nSize -= nSizeNew - boxMoov->m_nSize;
                free->m_data.resize(free->getDataSize(), 0);

                string bufFree;
                free->toBuf(bufFree);
                int nRet = write(bufFree, free->m_nOffset);
                if (nRet != ERR_OK) {
                    return nRet;
                }
            }
        }
    }

    if (nAdjustedOffset != 0) {
        adjustStco((int)nAdjustedOffset);
    }

    string buf;
    boxMoov->toBuf(buf);

    int nRet = write(buf, boxMoov->m_nOffset);
    if (nRet != ERR_OK) {
        return nRet;
    }

    close();

    return ERR_OK;
}

int CM4aTag::parse(M4aBox *parent, int nExtraHeaderOffset) {
    size_t totalSize = nExtraHeaderOffset;
    size_t offset = parent->m_nOffset + nExtraHeaderOffset;

    while (true) {
        fseek(m_fp, offset, SEEK_SET);

        // read size
        uint8_t buf[M4aBox::TYPE_SIZE];
        auto read = fread(buf, 1, M4aBox::TYPE_SIZE, m_fp);
        if (read != M4aBox::TYPE_SIZE) {
            return ERR_BAD_FILE_FORMAT;
        }

        size_t size = uint32FromBE(buf);
        if (size < M4aBox::HEADER_SIZE) {
            return ERR_BAD_FILE_FORMAT;
        }

        // read type
        char type[M4aBox::TYPE_SIZE];
        read = fread(type, 1, M4aBox::TYPE_SIZE, m_fp);
        if (read != M4aBox::TYPE_SIZE) {
            return ERR_BAD_FILE_FORMAT;
        }

        totalSize += size;
        if (totalSize > parent->m_nSize) {
            return ERR_BAD_FILE_FORMAT;
        }

        M4aBox *pChild = new M4aBox(type, offset, size);

        offset += size;
        parent->addChild(pChild);

        DBG_LOG3("Box: %S, offset: %d, size: %d\n", type, offset, size);

        if (totalSize == parent->m_nSize) {
            break;
        }
    }

    for (uint32_t i = 0; i < parent->m_vChildren.size(); i++) {
        M4aBox *pChild = parent->m_vChildren[i];

        // if (canBoxHasChild(pChild->m_szType))
        {
            int nExtraHeaderOffset = M4aBox::HEADER_SIZE;
            if (pChild->isType(SZ_TYPE_META)) {
                nExtraHeaderOffset += 4;
            }
            int nRet = parse(pChild, nExtraHeaderOffset);
            if (nRet != ERR_OK) {
                pChild->clearChildren();
            }
        }
    }

    return ERR_OK;
}

void getBoxText(M4aBox *parent, string &value) {
    if (value.empty()) {
        auto box = parent->findBox("data");
        if (box) {
            CM4aBoxParser parser(box);
            value = parser.parseAsUtf8();
        }
    }
}

void getBoxTrackNumber(M4aBox *parent, string &value) {
    if (value.empty()) {
        auto box = parent->findBox("data");
        if (box) {
            CM4aBoxParser parser(box);
            if (parser.getDataType() == CM4aBoxParser::DT_CUSTOMIZE) {
                auto arr = parser.parseAsUint16Arr();
                if (arr.size() == 4) {
                    value = itos(arr[1]) + "," + itos(arr[2]);
                }
            }
        }
    }
}

void getBoxGenre(M4aBox *parent, string &value) {
    if (value.empty()) {
        auto box = parent->findBox("data");
        if (box) {
            CM4aBoxParser parser(box);
            if (parser.getDataType() == CM4aBoxParser::DT_CUSTOMIZE) {
                auto arr = parser.parseAsUint16Arr();
                if (arr.size() == 1) {
                    value = CID3v1::getGenreDescription(arr[0]);
                }
            } else if (parser.getDataType() == CM4aBoxParser::DT_UTF8_STRING) {
                value = parser.parseAsUtf8();
            }
        }
    }
}

int CM4aTag::getTags(BasicMediaTags &tags) {

    M4aBox *parent = getBox("moov.udta.meta.ilst");
    if (!parent) {
        return ERR_NOT_FOUND;
    }

    for (auto child : parent->m_vChildren) {
        printf("%s\n", child->m_szType);

        if (child->isType("\xA9""art")) { getBoxText(child, tags.artist); }
        else if (child->isType("\xA9""ART")) { getBoxText(child, tags.artist); }
        else if (child->isType("aART")) { getBoxText(child, tags.artist); }
        else if (child->isType("\xA9""nam")) { getBoxText(child, tags.title); }
        else if (child->isType("\xA9""alb")) { getBoxText(child, tags.album); }
        else if (child->isType("\xA9""day")) { getBoxText(child, tags.year); }
        else if (child->isType("\xA9""gen")) { getBoxText(child, tags.genre); }
        else if (child->isType("gnre")) { getBoxGenre(child, tags.genre); }
        else if (child->isType("trkn")) { getBoxTrackNumber(child, tags.trackNo); }
    }

    return ERR_OK;
}

/**
stco = sample table chunk offset

QT Atom Preamble
1 uint8_t    version
3 bytes   flags
4 bytes   total entries in offset table (n)
4 bytes   chunk offset 0
4 bytes   chunk offset 1
..
..
4 bytes   chunk offset n-1
*/
int CM4aTag::adjustStco(int nAdjustedOffset) {
    M4aBox *pBoxStco = getBox("moov.trak.mdia.minf.stbl.stco");
    if (pBoxStco != nullptr) {
        const int offsetTrunk = 8;
        int count = (int)(pBoxStco->m_data.size() - offsetTrunk) / 4;
        int countShould = uint32FromBE((uint8_t *)(pBoxStco->m_data.c_str() + 4));
        assert(count == countShould);
        if (count > countShould) {
            count = countShould;
        }

        int end = offsetTrunk + count * 4;
        for (int i = offsetTrunk; i < end; i += 4) {
            uint32_t n = uint32FromBE((uint8_t *)(pBoxStco->m_data.c_str() + i));
            uint32ToBE(n + nAdjustedOffset, (uint8_t *)(pBoxStco->m_data.c_str() + i));
        }
    }

    return ERR_OK;
}

void read(FILE *fp, string &buf, size_t offset, size_t size) {
    fseek(fp, offset, SEEK_SET);
    buf.resize(size);
    auto n = fread((char *)buf.c_str(), 1, size, fp);
    if (n != size) {
        buf.resize(0);
    }
}

#ifdef _DEBUG
// This is used to verify the data read can composed are OK.
void verifyData(FILE *fp, M4aBox *box) {
    string buf;
    read(fp, buf, box->m_nOffset, box->m_nSize);
    assert(buf.size() == box->m_nSize);

    string buf2;
    box->toBuf(buf2);
    assert(buf.size() == buf2.size());
    assert(memcmp(buf.c_str(), buf2.c_str(), buf.size()) == 0);
}
#endif // #ifdef _DEBUG

int CM4aTag::read(M4aBox *parent) {
    // Assure the box is correct, we don't verify again.
    if (parent->m_vChildren.size() == 0) {
        parent->m_data.resize(parent->getDataSize());
        fseek(m_fp, parent->m_nOffset + M4aBox::HEADER_SIZE, SEEK_SET);
        auto n = fread((char *)parent->m_data.c_str(), 1, parent->m_data.size(), m_fp);
        if (n != parent->m_data.size()) {
            return ERR_READ_FILE;
        }

        // verifyData(m_fp, parent);

        return ERR_OK;
    }

    if (parent->isType(SZ_TYPE_META)) {
        parent->m_data.resize(4);
        fseek(m_fp, parent->m_nOffset + M4aBox::HEADER_SIZE, SEEK_SET);
        auto n = fread((char *)parent->m_data.c_str(), 1, 4, m_fp);
        if (n != 4) {
            return ERR_READ_FILE;
        }
    }

    for (uint32_t i = 0; i < parent->m_vChildren.size(); i++) {
        M4aBox *pChild = parent->m_vChildren[i];
        int nRet = read(pChild);
        if (nRet != ERR_OK) {
            return nRet;
        }
    }

    // verifyData(m_fp, parent);

    return ERR_OK;
}

M4aBox *CM4aTag::getBox(cstr_t szPath) {
    return ::getBox(&m_root, szPath);
}

int CM4aTag::listLyrics(VecStrings &vLyrNames) {
    if (hasLyrics()) {
        vLyrNames.push_back(SZ_SONG_M4A_LYRICS);
    }

    return ERR_OK;
}

int CM4aTag::getLyrics(string &strLyrics) {
    M4aBox *box = getBox("moov.udta.meta.ilst.\xA9lyr.data");
    if (!box) {
        return ERR_NOT_FOUND;
    }

    CM4aBoxParser parser(box);
    if (parser.getDataType() == CM4aBoxParser::DT_UTF8_STRING) {
        strLyrics = parser.parseAsUtf8();
        return ERR_OK;
    }

    return ERR_NOT_SUPPORT_FILE_FORMAT;
}

bool CM4aTag::hasLyrics() {
    return getBox("moov.udta.meta.ilst.\xA9lyr.data") != nullptr;
}

int CM4aTag::setLyrics(cstr_t szLyrics) {
    M4aBox *pIList = getBox("moov.udta.meta.ilst");
    if (pIList == nullptr) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    getBox("moov")->m_nSizeNew = 0;

    M4aBox *pLyrics = pIList->findBox(SZ_TYPE_LYRICS);
    if (pLyrics == nullptr) {
        pLyrics = new M4aBox(SZ_TYPE_LYRICS, 0, 0);
        pIList->addChild(pLyrics);

        pLyrics->addChild(new M4aBox(SZ_TYPE_DATA, 0, 0));
    }

    M4aBox *box = pLyrics->findBox(SZ_TYPE_DATA);
    if (!box) {
        return ERR_BAD_FILE_FORMAT;
    }

    CM4aBoxParser parser(box);
    parser.convertToText(szLyrics);

    return ERR_OK;
}

int CM4aTag::removeLyrics() {
    M4aBox *pIList = getBox("moov.udta.meta.ilst");
    if (pIList == nullptr) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    getBox("moov")->m_nSizeNew = 0;

    M4aBox *pLyrics = pIList->findBox(SZ_TYPE_LYRICS);
    if (pLyrics != nullptr) {
        pIList->deleteChild(SZ_TYPE_LYRICS);
    }

    return ERR_OK;
}

int CM4aTag::write(string &buf, size_t offset) {
    fseek(m_fp, offset, SEEK_SET);
    if (fwrite(buf.c_str(), 1, buf.size(), m_fp) != buf.size()) {
        return ERR_WRITE_FILE;
    }
    return ERR_OK;
}
