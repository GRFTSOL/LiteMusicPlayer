#include "M4aTag.h"
#include "ID3/ID3v2.h"
#include "ID3/ID3Helper.h"
#include "MediaTags.h"


#define SZ_TYPE_MOOV        "moov"
#define SZ_TYPE_TRACK        "trak"
#define SZ_TYPE_UDTA        "udta"
#define SZ_TYPE_META        "meta"
#define SZ_TYPE_ILST        "ilst"
#define SZ_TYPE_LYRICS        "\xA9lyr"
#define SZ_TYPE_DATA        "data"


bool canBoxHasChild(char szType[])
{
    cstr_t    vList[] = { SZ_TYPE_MOOV, SZ_TYPE_TRACK, SZ_TYPE_UDTA, SZ_TYPE_META, SZ_TYPE_ILST, };

    for (int i = 0; i < CountOf(vList); i++)
    {
        if (strncmp(szType, vList[i], M4aBox::TYPE_SIZE) == 0)
            return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
class CM4aBoxParser
{
public:
    CM4aBoxParser() { }
    virtual ~CM4aBoxParser() { }

    virtual int parse(M4aBox *box) = 0;
    virtual int convertTo(M4aBox *box) = 0;

};

class CM4aBoxTextParser : public CM4aBoxParser
{
public:
    virtual int parse(M4aBox *box)
    {
        char buf[8];
        memset(buf, 0, 8);
        buf[3] = 1;
        if (memcmp(box->m_data.c_str(), buf, 8) != 0)
            return ERR_BAD_FILE_FORMAT;

        m_strLyrics.assign(box->m_data.c_str() + 8, box->m_nSize - 8 - M4aBox::HEADER_SIZE);
        return ERR_OK;
    }

    virtual int convertTo(M4aBox *box)
    {
        if (box->m_data.size() < 8)
        {
            box->m_data.resize(8);
            memset((char *)box->m_data.c_str(), 0, box->m_data.size());
            box->m_data[3] = 1;
        }
        else
            box->m_data.resize(8);
        box->m_data.append(m_strLyrics);
        return ERR_OK;
    }

    string        m_strLyrics;

};

#ifdef _DEBUG
void debugParseLyrics(M4aBox *box)
{
    CM4aBoxTextParser parser;
    parser.parse(box);
    _tprintf("Lyrics: %s\r\n", parser.m_strLyrics.c_str());
}
#endif // #ifdef _DEBUG

//////////////////////////////////////////////////////////////////////////

M4aBox::M4aBox(cstr_t szType, size_t offset, size_t size)
{
    m_nOffset = offset;
    m_nSize = size;
    strncpy_safe(m_szType, CountOf(m_szType), szType, M4aBox::TYPE_SIZE);

    assert(sizeof(m_szType) >= TYPE_SIZE);
    m_nSizeNew = 0;
}

M4aBox::~M4aBox()
{
    clearChildren();
}

void M4aBox::addChild(M4aBox *box)
{
    m_vChildren.push_back(box);
}

bool M4aBox::isType(cstr_t szType)
{
    return strncmp(m_szType, szType, TYPE_SIZE) == 0;
}

void M4aBox::clearChildren()
{
    for (uint32_t i = 0; i < m_vChildren.size(); i++)
        delete m_vChildren[i];
    m_vChildren.clear();
}

M4aBox *M4aBox::findBox(cstr_t szBox)
{
    for (uint32_t i = 0; i < m_vChildren.size(); i++)
    {
        M4aBox *p = m_vChildren[i];
        if (p->isType(szBox))
            return p;
    }

    return nullptr;
}

M4aBox *M4aBox::nextBox(M4aBox *brother)
{
    for (uint32_t i = 0; i < m_vChildren.size(); i++)
    {
        M4aBox *p = m_vChildren[i];
        if (p == brother)
        {
            if (i + 1 < m_vChildren.size())
                return m_vChildren[i + 1];
        }
    }

    return nullptr;
}

bool M4aBox::deleteChild(cstr_t szType)
{
    for (uint32_t i = 0; i < m_vChildren.size(); i++)
    {
        M4aBox *p = m_vChildren[i];
        if (p->isType(szType))
        {
            delete p;
            m_vChildren.erase(m_vChildren.begin() + i);
            return true;
        }
    }

    return false;
}

void M4aBox::toBuf(string &buf)
{
    // size
    if (m_nSizeNew == 0)
        calculateNewSize();

    uint8_t size[4];
    uint32ToBE(m_nSizeNew, size);
    buf.append((char *)size, M4aBox::TYPE_SIZE);

    // Type
    buf.append(m_szType, M4aBox::TYPE_SIZE);

    if (m_vChildren.size() == 0)
    {
        buf.append(m_data);
        return;
    }

    // Data
    if (m_data.size() > 0)
    {
        assert(isType(SZ_TYPE_META));
        buf.append(m_data);
    }

    // Children
    for (uint32_t i = 0; i < m_vChildren.size(); i++)
    {
        M4aBox *p = m_vChildren[i];
        p->toBuf(buf);
    }
}

void M4aBox::calculateNewSize()
{
    m_nSizeNew = HEADER_SIZE + m_data.size();

    for (uint32_t i = 0; i < m_vChildren.size(); i++)
    {
        M4aBox *p = m_vChildren[i];
        p->calculateNewSize();
        m_nSizeNew += p->m_nSizeNew;
    }
}

//////////////////////////////////////////////////////////////////////////

class M4aLyricsBox : public M4aBox
{
public:
    M4aLyricsBox();

    string                m_strLyrics;

};

//////////////////////////////////////////////////////////////////////////

CM4aTag::CM4aTag() : m_root("", 0, 0)
{
}

CM4aTag::~CM4aTag()
{
    close();
}

int CM4aTag::open(cstr_t szFile, bool bModify, bool bCreate)
{
    if (bModify)
        m_fp = fopen(szFile, "r+b");
    else
        m_fp = fopen(szFile, "rb");

    if (!m_fp)
    {
        setCustomErrorDesc(stringPrintf("%s: %s", (cstr_t)OSError(), szFile).c_str());
        return ERR_CUSTOM_ERROR;
    }

    return open(m_fp, bCreate);
}

int CM4aTag::open(FILE *fp, bool bCreate)
{
    assert(fp);
    m_fp = fp;

    fseek(fp, 0, SEEK_END);
    size_t nFileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    m_root.m_nOffset = 0;
    m_root.m_nSize = nFileSize;
    int nRet = parse(&m_root, 0);
    if (nRet != ERR_OK)
        return nRet;

    // read moov box immediately
    M4aBox *boxMoov = m_root.findBox(SZ_TYPE_MOOV);
    if (boxMoov == nullptr)
        return ERR_NOT_SUPPORT_FILE_FORMAT;

    return read(boxMoov);
}

void CM4aTag::close()
{
    if (m_fp)
    {
        fclose(m_fp);
        m_fp = nullptr;
    }
}

int CM4aTag::saveClose()
{
    M4aBox *boxMoov = m_root.findBox(SZ_TYPE_MOOV);
    if (boxMoov == nullptr)
        return ERR_NOT_SUPPORT_FILE_FORMAT;

    if (getBox("moov.trak.mdia.minf.stbl.co64") != nullptr)
        return ERR_NOT_SUPPORT_FILE_FORMAT;

    boxMoov->calculateNewSize();

    int nAdjustedOffset = 0;
    size_t nSizeNew = boxMoov->m_nSizeNew;
    if (boxMoov->m_nSize != nSizeNew)
    {
        // Need to resize boxes
        M4aBox *free = m_root.nextBox(boxMoov);
        if (free == nullptr)
        {
            // resize
            int nRet = fileMoveEndData(m_fp, boxMoov->m_nOffset + boxMoov->m_nSize, boxMoov->m_nOffset + nSizeNew);
            if (nRet != ERR_OK)
                return nRet;
            nAdjustedOffset = nSizeNew - boxMoov->m_nSize;
        }
        else
        {
            if (boxMoov->m_nSize + free->m_nSize + M4aBox::HEADER_SIZE < nSizeNew)
            {
                // resize
                int nRet = fileMoveEndData(m_fp, boxMoov->m_nOffset + boxMoov->m_nSize, boxMoov->m_nOffset + nSizeNew);
                if (nRet != ERR_OK)
                    return nRet;
                nAdjustedOffset = nSizeNew - boxMoov->m_nSize;
            }
            else
            {
                // resize free box
                free->m_nOffset = boxMoov->m_nOffset + nSizeNew;
                free->m_nSize -= nSizeNew - boxMoov->m_nSize;
                free->m_data.resize(free->getDataSize(), 0);

                string bufFree;
                free->toBuf(bufFree);
                int nRet = write(bufFree, free->m_nOffset);
                if (nRet != ERR_OK)
                    return nRet;
            }
        }
    }

    if (nAdjustedOffset != 0)
        adjustStco(nAdjustedOffset);

    string buf;
    boxMoov->toBuf(buf);

    int nRet = write(buf, boxMoov->m_nOffset);
    if (nRet != ERR_OK)
        return nRet;

    close();

    return ERR_OK;
}

int CM4aTag::parse(M4aBox *parent, int nExtraHeaderOffset)
{
    size_t    totalSize = nExtraHeaderOffset;
    size_t    offset = parent->m_nOffset + nExtraHeaderOffset;

    while (true)
    {
        fseek(m_fp, offset, SEEK_SET);

        // read size
        uint8_t    buf[M4aBox::TYPE_SIZE];
        int read = fread(buf, 1, M4aBox::TYPE_SIZE, m_fp);
        if (read != M4aBox::TYPE_SIZE)
            return ERR_BAD_FILE_FORMAT;

        size_t size = uint32FromBE(buf);
        if (size < M4aBox::HEADER_SIZE)
            return ERR_BAD_FILE_FORMAT;

        // read type
        char    type[M4aBox::TYPE_SIZE];
        read = fread(type, 1, M4aBox::TYPE_SIZE, m_fp);
        if (read != M4aBox::TYPE_SIZE)
            return ERR_BAD_FILE_FORMAT;

        totalSize += size;
        if (totalSize > parent->m_nSize)
            return ERR_BAD_FILE_FORMAT;

        M4aBox    *pChild = new M4aBox(type, offset, size);

        offset += size;
        parent->addChild(pChild);

        DBG_LOG3("Box: %S, offset: %d, size: %d\n", type, offset, size);

        if (totalSize == parent->m_nSize)
            break;
    }

    for (uint32_t i = 0; i < parent->m_vChildren.size(); i++)
    {
        M4aBox    *pChild = parent->m_vChildren[i];

        // if (canBoxHasChild(pChild->m_szType))
        {
            int nExtraHeaderOffset = M4aBox::HEADER_SIZE;
            if (pChild->isType(SZ_TYPE_META))
                nExtraHeaderOffset += 4;
            int nRet = parse(pChild, nExtraHeaderOffset);
            if (nRet != ERR_OK)
                pChild->clearChildren();
        }
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
int CM4aTag::adjustStco(int nAdjustedOffset)
{
    M4aBox *pBoxStco = getBox("moov.trak.mdia.minf.stbl.stco");
    if (pBoxStco != nullptr)
    {
        const int offsetTrunk = 8;
        int count = (pBoxStco->m_data.size() - offsetTrunk) / 4;
        int countShould = uint32FromBE((uint8_t *)(pBoxStco->m_data.c_str() + 4));
        assert(count == countShould);
        if (count > countShould)
            count = countShould;

        int end = offsetTrunk + count * 4;
        for (int i = offsetTrunk; i < end; i += 4)
        {
            uint32_t n = uint32FromBE((uint8_t *)(pBoxStco->m_data.c_str() + i));
            uint32ToBE(n + nAdjustedOffset, (uint8_t *)(pBoxStco->m_data.c_str() + i));
        }
    }

    return ERR_OK;
}

void read(FILE *fp, string &buf, size_t offset, size_t size)
{
    fseek(fp, offset, SEEK_SET);
    buf.resize(size);
    int n = fread((char *)buf.c_str(), 1, size, fp);
    if (n != size)
        buf.resize(0);
}

#ifdef _DEBUG
// This is used to verify the data read can composed are OK.
void verifyData(FILE *fp, M4aBox *box)
{
    string buf;
    read(fp, buf, box->m_nOffset, box->m_nSize);
    assert(buf.size() == box->m_nSize);

    string buf2;
    box->toBuf(buf2);
    assert(buf.size() == buf2.size());
    assert(memcmp(buf.c_str(), buf2.c_str(), buf.size()) == 0);
}
#endif // #ifdef _DEBUG

int CM4aTag::read(M4aBox *parent)
{
    // Assure the box is correct, we don't verify again.
    if (parent->m_vChildren.size() == 0)
    {
        parent->m_data.resize(parent->getDataSize());
        fseek(m_fp, parent->m_nOffset + M4aBox::HEADER_SIZE, SEEK_SET);
        int n = fread((char *)parent->m_data.c_str(), 1, parent->m_data.size(), m_fp);
        if (n != parent->m_data.size())
            return ERR_READ_FILE;

        // verifyData(m_fp, parent);

        return ERR_OK;
    }

    if (parent->isType(SZ_TYPE_META))
    {
        parent->m_data.resize(4);
        fseek(m_fp, parent->m_nOffset + M4aBox::HEADER_SIZE, SEEK_SET);
        int n = fread((char *)parent->m_data.c_str(), 1, 4, m_fp);
        if (n != 4)
            return ERR_READ_FILE;
    }

    for (uint32_t i = 0; i < parent->m_vChildren.size(); i++)
    {
        M4aBox    *pChild = parent->m_vChildren[i];
        int nRet = read(pChild);
        if (nRet != ERR_OK)
            return nRet;
    }

    // verifyData(m_fp, parent);

    return ERR_OK;
}

M4aBox *CM4aTag::getBox(cstr_t szPath)
{
    VecStrings    vPath;
    strSplit(szPath, '.', vPath);

    M4aBox *parent = &m_root;
    for (uint32_t i = 0; i < vPath.size(); i++)
    {
        parent = parent->findBox(vPath[i].c_str());
        if (!parent)
            return nullptr;
    }

    return parent;
}

int CM4aTag::listLyrics(VecStrings &vLyrNames)
{
    if (hasLyrics())
        vLyrNames.push_back(SZ_SONG_M4A_LYRICS);

    return ERR_OK;
}

int CM4aTag::getLyrics(string &strLyrics)
{
    M4aBox *box = getBox("moov.udta.meta.ilst.\xA9lyr.data");
    if (!box)
        return ERR_NOT_FOUND;

    CM4aBoxTextParser parser;
    int nRet = parser.parse(box);
    if (nRet == ERR_OK)
        strLyrics = parser.m_strLyrics;

    return nRet;
}

bool CM4aTag::hasLyrics()
{
    return getBox("moov.udta.meta.ilst.\xA9lyr.data") != nullptr;
}

int CM4aTag::setLyrics(cstr_t szLyrics)
{
    M4aBox *pIList = getBox("moov.udta.meta.ilst");
    if (pIList == nullptr)
        return ERR_NOT_SUPPORT_FILE_FORMAT;

    getBox("moov")->m_nSizeNew = 0;

    M4aBox *pLyrics = pIList->findBox(SZ_TYPE_LYRICS);
    if (pLyrics == nullptr)
    {
        pLyrics = new M4aBox(SZ_TYPE_LYRICS, 0, 0);
        pIList->addChild(pLyrics);

        pLyrics->addChild(new M4aBox(SZ_TYPE_DATA, 0, 0));
    }

    M4aBox *pData = pLyrics->findBox(SZ_TYPE_DATA);
    if (!pData)
        return ERR_BAD_FILE_FORMAT;

    CM4aBoxTextParser parser;
    parser.m_strLyrics = szLyrics;
    parser.convertTo(pData);

    return ERR_OK;
}

int CM4aTag::removeLyrics()
{
    M4aBox *pIList = getBox("moov.udta.meta.ilst");
    if (pIList == nullptr)
        return ERR_NOT_SUPPORT_FILE_FORMAT;

    getBox("moov")->m_nSizeNew = 0;

    M4aBox *pLyrics = pIList->findBox(SZ_TYPE_LYRICS);
    if (pLyrics != nullptr)
    {
        pIList->deleteChild(SZ_TYPE_LYRICS);
    }

    return ERR_OK;
}

int CM4aTag::write(string &buf, size_t offset)
{
    fseek(m_fp, offset, SEEK_SET);
    if (fwrite(buf.c_str(), 1, buf.size(), m_fp) != buf.size())
        return ERR_WRITE_FILE;
    return ERR_OK;
}
