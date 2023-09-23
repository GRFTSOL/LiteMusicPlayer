#include "Utils.h"
#include "MLBinXMLParser.h"
#include "LogAlias.h"


/************************************************************************

MiniLyrics binary XML format 1.0

string encoding: UTF8.

XML = "MBXML" version[uint8_t] flags[uint32_t] total_length[uint32_t] [data block] [data block] ...

[data block] = block_type[2 bytes] length[uint32_t] [data...]

[string table data block] = "ST"[2 bytes] length[uint32_t] count[uint32_t] [string\0] [string\0] ...
[bin data block] = "BN"[2 bytes] length[uint32_t] count[uint32_t] item_length[uint32_t] item_data[item_length uint8_t] item_length[uint32_t] item_data[item_length uint8_t] ...
[XML content data block] = "CT" [2 bytes] length[uint32_t]

"<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table "</" element_name_index_str_table ">"

"<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table 
  "<" element2_name_index_str_table prop_name_index_str_table prop_value_index_str_table "/>"
"/>"

\************************************************************************/

#define SZ_BIN_XML_HEADER   "MBXML"
#define LEN_BIN_XML_HEADER        (sizeof(SZ_BIN_XML_HEADER) - 1)

#define LEN_BLOCK_NAME      2
#define LEN_BLOCK_HEADER        (LEN_BLOCK_NAME + sizeof(uint32_t)  )
#define LEN_BIN_XML_MIN            (LEN_BIN_XML_HEADER + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + (LEN_BLOCK_HEADER + 2) * 2)

#define BX_BLOCK_BIN        "BN"
#define BX_BLOCK_STING_TABLE "ST"
#define BX_BLOCK_CONTENT    "CT"


enum BinXmlVersion {
    BXV_1                       = '1'
};

enum BinXmlFlag {
    // BXF_UCS2            = 1,        // UCS2 encoding, or it's UTF-8 encoding
    BXF_BYTE_INDEX              = 1 << 1,
    BXF_WORD_INDEX              = 1 << 2,
    BXF_DWORD_INDEX             = 1 << 3,
};

enum StringTableId {
    BX_ID_INVALID               = 0,
    BX_ID_BINDATA               = 1,
    BX_ID_L_BRACKET             = 2,
    BX_ID_R_BRACKET             = 3,
    BX_ID_ELEMENT_END           = 4,
    BX_ID_STR_START             = 10,
};

class CBinXmlStrTable {
public:
    bool setTable(char *pFirst, uint32_t nLength, uint32_t nCount) {
        m_vStr.resize(nCount);

        const char *str = (const char *)pFirst;
        const char *end = str + nLength / sizeof(char);
        uint32_t n;
        for (n = 0; n < nCount && str < end; n++) {
            size_t len = strlen(str);
            m_vStr[n].assign(str, len);
            str += len + 1;
        }
        assert(str == end);
        assert(n == nCount);

        return n == nCount && str == end;
    }

    string &getStr(int nIndex) {
        nIndex -= BX_ID_STR_START;
        assert(nIndex >= 0 && nIndex < (int)m_vStr.size());
        if (nIndex < 0 || nIndex >= (int)m_vStr.size()) {
            return m_emptyStr;
        }

        return m_vStr[nIndex];
    }

    VecStrings                  m_vStr;
    string                      m_emptyStr;

};

//////////////////////////////////////////////////////////////////////////

class CBinXmlBinDataTable {
public:
    bool setTable(uint8_t *data, uint32_t nLength, uint32_t nCount) {
        m_vBinData.resize(nCount);

        uint8_t *dataEnd = data + nLength;
        for (uint32_t i = 0; i < nCount && data + sizeof(uint32_t) + 1 < dataEnd; i++) {
            uint32_t lenItem = uint32FromLE(data);
            data += sizeof(uint32_t);
            if (data + lenItem > dataEnd) {
                return false;
            }
            m_vBinData[i].assign((cstr_t)data, lenItem);
            data += lenItem;
        }

        assert(data == dataEnd);

        return m_vBinData.size() == nCount && data == dataEnd;
    }

    string &getData(int nIndex) {
        assert(nIndex >= 0 && nIndex < (int)m_vBinData.size());
        if (nIndex < 0 || nIndex >= (int)m_vBinData.size()) {
            return m_emptyData;
        }
        return m_vBinData[nIndex];
    }

    VecStrings                  m_vBinData;
    string                      m_emptyData;

};

//////////////////////////////////////////////////////////////////////////
// "<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table "</" element_name_index_str_table ">"
// "<" element_name_index_str_table prop_name_index_str_table prop_value_index_str_table ">" value_name_index_str_table
//   "<" element2_name_index_str_table prop_name_index_str_table prop_value_index_str_table "/>"
// "/>"
//     BX_ID_INVALID        = 0,
//     BX_ID_BINDATA        = 1,
//     BX_ID_L_BRACKET        = 2,
//     BX_ID_R_BRACKET        = 3,
//     BX_ID_ELEMENT_END    = 4,
//     BX_ID_STR_START        = 10

template<class index_t>
class CMLBXContentParserObj {
public:
    CMLBXContentParserObj(CSimpleXML &xml, index_t *data, int nLen,
        CBinXmlBinDataTable    &binDataTable, CBinXmlStrTable &strTable)
    : m_xml(xml), m_data(data), m_nLen(nLen), m_binDataTable(binDataTable), m_strTable(strTable) {
    }

    CMLBinXMLParser::MLBinXmlParseError ParseContent() {
        assert(m_xml.m_pRoot == nullptr);
        m_xml.m_pRoot = new SXNode;

        int nPosNext = 0;
        CMLBinXMLParser::MLBinXmlParseError ret = parseEntity(0, nPosNext, m_xml.m_pRoot);
        if (ret != CMLBinXMLParser::E_OK) {
            DBG_LOG2("Failed to parse MLBinary XML at: %d, error: %d", nPosNext, ret);
        }

        return ret;
    }

protected:
    CMLBinXMLParser::MLBinXmlParseError parseEntity(int nPos, int &nPosNext, SXNode *pNode) {
        if (m_data[nPos] != BX_ID_L_BRACKET) {
            return CMLBinXMLParser::E_FMT_NEED_LBRACKET;
        }
        nPos++;

        // Name
        pNode->name = m_strTable.getStr(m_data[nPos]);
        nPos++;

        // properties
        while (m_data[nPos] != BX_ID_R_BRACKET && m_data[nPos] != BX_ID_ELEMENT_END) {
            index_t iPropName = m_data[nPos];
            index_t iPropValue = m_data[nPos + 1];
            nPos += 2;

            SXNode::Property prop;
            prop.name = m_strTable.getStr(iPropName);
            if (iPropValue == BX_ID_BINDATA) {
                prop.strValue = m_binDataTable.getData(m_data[nPos]);
                nPos++;
            } else {
                prop.strValue = m_strTable.getStr(iPropValue);
            }
            pNode->listProperties.push_back(prop);
        }

        // Property end
        if (m_data[nPos] == BX_ID_R_BRACKET) {
            // read value str
            nPos++;
            if (m_data[nPos] != BX_ID_L_BRACKET
                && m_data[nPos] != BX_ID_ELEMENT_END) {
                if (m_data[nPos] == BX_ID_BINDATA) {
                    pNode->strContent = m_binDataTable.getData(m_data[nPos]);
                } else {
                    pNode->strContent = m_strTable.getStr(m_data[nPos]);
                }
                nPos++;
            }
        }

        // Child element
        while (m_data[nPos] == BX_ID_L_BRACKET) {
            SXNode *pNodeChild = new SXNode;
            CMLBinXMLParser::MLBinXmlParseError err = parseEntity(nPos, nPosNext, pNodeChild);
            if (err != CMLBinXMLParser::E_OK) {
                delete pNodeChild;
                return err;
            }
            pNode->listChildren.push_back(pNodeChild);
            nPos = nPosNext;
        }

        // Element End
        nPosNext = nPos + 1;
        if (m_data[nPos] == BX_ID_ELEMENT_END) {
            return CMLBinXMLParser::E_OK;
        } else {
            return CMLBinXMLParser::E_FMT_NEED_ELEMENT_END;
        }
    }


protected:
    CSimpleXML                  &m_xml;
    index_t                     *m_data;
    int                         m_nLen;

    CBinXmlBinDataTable         &m_binDataTable;
    CBinXmlStrTable             &m_strTable;

};

//////////////////////////////////////////////////////////////////////////

CMLBinXMLParser::CMLBinXMLParser(void) {
}

CMLBinXMLParser::~CMLBinXMLParser(void) {
}

CMLBinXMLParser::MLBinXmlParseError CMLBinXMLParser::parseData(const void *lpXmlData, size_t nLen, CSimpleXML &xmlData) {
    assert(strlen(SZ_BIN_XML_HEADER) == LEN_BIN_XML_HEADER);

    if (nLen < LEN_BIN_XML_MIN) {
        return E_INVALID_SIZE;
    }

    xmlData.m_encoding = SZ_UTF8;

    uint8_t *data = (uint8_t*)lpXmlData;
    uint8_t *dataEnd = data;

    // Header
    if (!startsWith((cstr_t)data, SZ_BIN_XML_HEADER)) {
        return E_INVALID_HEADER;
    }
    data += LEN_BIN_XML_HEADER;

    // version
    if (*data != BXV_1) {
        return E_UNKNOWN_VERSION;
    }
    data++;

    // Flag
    uint32_t nXmlFlags = uint32FromLE(data);
    data += sizeof(uint32_t);

    // length
    uint32_t nTotalLength = uint32FromLE(data);
    data += sizeof(uint32_t);
    dataEnd += nTotalLength;

    if (nTotalLength > nLen) {
        return E_INVALID_SIZE;
    }

    CBinXmlBinDataTable binDataTable;
    CBinXmlStrTable strTable;

    while (data + LEN_BLOCK_HEADER + sizeof(uint32_t) < dataEnd) {
        // read block info
        cstr_t blockName = (cstr_t)data;
        data += LEN_BLOCK_NAME;
        uint32_t lenBlock = uint32FromLE(data);
        data += sizeof(uint32_t);

        lenBlock -= LEN_BLOCK_HEADER;
        if (data + lenBlock > dataEnd) {
            return E_INVALID_SIZE;
        }

        if (startsWith(blockName, BX_BLOCK_STING_TABLE)) {
            uint32_t countStr = uint32FromLE(data);
            data += sizeof(uint32_t);
            lenBlock -= sizeof(uint32_t);
            if (!strTable.setTable((char*)data, lenBlock, countStr)) {
                return E_INVALID_STR_TABLE;
            }

            data += lenBlock;
        } else if (startsWith(blockName, BX_BLOCK_BIN)) {
            uint32_t countItem = uint32FromLE(data);
            data += sizeof(uint32_t);
            lenBlock -= sizeof(uint32_t);
            if (!binDataTable.setTable(data, lenBlock, countItem)) {
                return E_INVALID_BIN_TABLE;
            }

            data += lenBlock;
        } else if (startsWith(blockName, BX_BLOCK_CONTENT)) {
            if (isFlagSet(nXmlFlags, BXF_BYTE_INDEX)) {
                CMLBXContentParserObj<uint8_t> parser(xmlData, data, lenBlock, binDataTable, strTable);
                return parser.ParseContent();
            } else if (isFlagSet(nXmlFlags, BXF_WORD_INDEX)) {
                assert(lenBlock % sizeof(uint16_t) == 0);
                uint16_t *wData = (uint16_t *)data;
                uint8_t *dataEnd = data + lenBlock;
                uint16_t *wp = wData;
                for (uint8_t *p = data; p < dataEnd; p += sizeof(uint16_t), wp++) {
                    *wp = uint16FromLE(data);
                }

                CMLBXContentParserObj<uint16_t>    parser(xmlData, wData,
                    lenBlock / sizeof(uint16_t), binDataTable, strTable);
                return parser.ParseContent();
            } else if (isFlagSet(nXmlFlags, BXF_DWORD_INDEX)) {
                assert(lenBlock % sizeof(uint32_t) == 0);
                uint32_t *dwData = (uint32_t *)data;
                uint8_t *dataEnd = data + lenBlock;
                uint32_t *wp = dwData;
                for (uint8_t *p = data; p < dataEnd; p += sizeof(uint32_t), wp++) {
                    *wp = uint32FromLE(data);
                }

                CMLBXContentParserObj<uint32_t>    parser(xmlData, dwData,
                    lenBlock / sizeof(uint32_t), binDataTable, strTable);
                return parser.ParseContent();
            } else {
                assert(0);
            }
        } else {
            assert(0);
            return E_INVALID_BLOCK_TYPE;
        }
    }

    return E_OK;
}

bool CMLBinXMLParser::isHeaderType(cstr_t szHeader) {
    return startsWith(szHeader, SZ_BIN_XML_HEADER);
}

//////////////////////////////////////////////////////////////////////////

inline void appendUint32(string &buf, uint32_t n) {
    uint8_t byLen[sizeof(uint32_t)];
    uint32ToLE(n, byLen);
    buf.append((cstr_t)byLen, sizeof(uint32_t));
}

CMLBinXMLWriter::CMLBinXMLWriter() : CXMLWriter(false) {
    m_nBinItemCount = 0;
    m_nElementDeep = 0;
}

CMLBinXMLWriter::~CMLBinXMLWriter() {
}

// Begin xml element
void CMLBinXMLWriter::writeStartElement(cstr_t szElement) {
    if (m_wp == WP_ELEMENT_HEAD) {
        m_vContentBlock.push_back(BX_ID_R_BRACKET);
    }
    m_wp = WP_ELEMENT_HEAD;

    m_vContentBlock.push_back(BX_ID_L_BRACKET);
    m_vContentBlock.push_back(getStrIndex(szElement));
    m_nElementDeep++;
}

// Writes: szAttr="szValue"
void CMLBinXMLWriter::writeAttribute(cstr_t szAttr, cstr_t szValue) {
    assert(m_wp == WP_ELEMENT_HEAD);

    m_vContentBlock.push_back(getStrIndex(szAttr));
    m_vContentBlock.push_back(getStrIndex(szValue));
}

void CMLBinXMLWriter::writeAttribute(cstr_t szAttr, const void *data, size_t lenData) {
    assert(m_wp == WP_ELEMENT_HEAD);

    // length
    appendUint32(m_bufBinTable, (uint32_t)lenData);

    // data
    m_bufBinTable.append((cstr_t)data, lenData);
    m_nBinItemCount++;

    // attribute
    m_vContentBlock.push_back(getStrIndex(szAttr));
    m_vContentBlock.push_back(BX_ID_BINDATA);
    m_vContentBlock.push_back((uint32_t)m_nBinItemCount - 1);
}

// close xml element
void CMLBinXMLWriter::writeEndElement() {
    assert(m_wp == WP_ELEMENT_HEAD || m_wp == WP_ELEMENT_BODY);

    m_nElementDeep--;
    assert(m_nElementDeep >= 0);

    m_vContentBlock.push_back(BX_ID_ELEMENT_END);

    if (m_nElementDeep > 0) {
        m_wp = WP_ELEMENT_BODY;
    } else {
        m_wp = WP_READY;
    }
}

// Writes the given text content: <node>szString</node>
void CMLBinXMLWriter::writeString(cstr_t szString) {
    assert(m_wp == WP_ELEMENT_HEAD);
    m_wp = WP_ELEMENT_BODY;

    m_vContentBlock.push_back(BX_ID_R_BRACKET);
    m_vContentBlock.push_back(getStrIndex(szString));
}

// Writes: <szElement>szString</szElement>
void CMLBinXMLWriter::writeElementString(cstr_t szElement, cstr_t szString) {
    m_vContentBlock.push_back(BX_ID_L_BRACKET);
    m_vContentBlock.push_back(getStrIndex(szElement));
    m_vContentBlock.push_back(BX_ID_R_BRACKET);
    m_vContentBlock.push_back(getStrIndex(szString));
    m_vContentBlock.push_back(BX_ID_ELEMENT_END);
}

// Writes: <!-- szComment -->
void CMLBinXMLWriter::writeComment(cstr_t szComment) {
}

string &CMLBinXMLWriter::getBuffer() {
    if (m_strBuff.size() > 0) {
        return m_strBuff;
    }

    uint32_t flagXml = 0;
    int indexSize = 0;
    if (m_mapStrTable.size() + BX_ID_STR_START <= 255) {
        indexSize = 1;
        flagXml |= BXF_BYTE_INDEX;
    } else if (m_mapStrTable.size() + BX_ID_STR_START <= (uint16_t)-1) {
        indexSize = sizeof(uint16_t);
        flagXml |= BXF_WORD_INDEX;
    } else {
        indexSize = sizeof(uint32_t);
        flagXml |= BXF_DWORD_INDEX;
    }

    m_strBuff.append(SZ_BIN_XML_HEADER);
    m_strBuff.append(1, BXV_1);
    appendUint32(m_strBuff, flagXml);
    appendUint32(m_strBuff, 0);

    // write string table
    writeStrTable();

    // write binary data block
    if (m_bufBinTable.size()) {
        m_strBuff.append(BX_BLOCK_BIN, LEN_BLOCK_NAME);
        appendUint32(m_strBuff, LEN_BLOCK_HEADER + sizeof(uint32_t) + (uint32_t)m_bufBinTable.size());
        appendUint32(m_strBuff, m_nBinItemCount);
        m_strBuff.append(m_bufBinTable.c_str(), m_bufBinTable.size());
    }

    // write content block
    writeContentBlock(indexSize);

    // Rewrite total length
    uint32ToLE((uint32_t)m_strBuff.size(), (uint8_t *)m_strBuff.data() + LEN_BIN_XML_HEADER + 1 + sizeof(uint32_t));

    return m_strBuff;
}

bool CMLBinXMLWriter::saveAsFile(cstr_t szFile) {
    getBuffer();

    return CXMLWriter::saveAsFile(szFile);
}

int CMLBinXMLWriter::getStrIndex(cstr_t str) {
    MapStrIndex::iterator it = m_mapStrTable.find(str);
    if (it == m_mapStrTable.end()) {
        int n = (uint32_t)m_mapStrTable.size() + BX_ID_STR_START;
        m_mapStrTable[str] = n;
        assert(n + 1 - BX_ID_STR_START == m_mapStrTable.size());
        return n;
    } else {
        return (*it).second;
    }
}

void CMLBinXMLWriter::writeStrTable() {
    assert(strlen(BX_BLOCK_CONTENT) == LEN_BLOCK_NAME);
    typedef vector<const string *> VPStr;
    VPStr vPStr;

    // sort map str as an array.
    vPStr.resize(m_mapStrTable.size());
    for (MapStrIndex::iterator it = m_mapStrTable.begin(); it != m_mapStrTable.end(); ++it) {
        assert((*it).second >= BX_ID_STR_START && (*it).second < BX_ID_STR_START + (int)m_mapStrTable.size());
        vPStr[(*it).second - BX_ID_STR_START] = &((*it).first);
    }

    // write header
    uint32_t nStrTableStartPos = (uint32_t)m_strBuff.size();
    m_strBuff.append(BX_BLOCK_STING_TABLE, LEN_BLOCK_NAME);
    appendUint32(m_strBuff, 0);
    appendUint32(m_strBuff, (uint32_t)m_mapStrTable.size());

    // write string
    for (uint32_t i = 0; i < vPStr.size(); i++) {
        const string *pStr = vPStr[i];
        if (m_strBuff.capacity() < m_strBuff.size() + pStr->size() * 3) {
            m_strBuff.reserve(m_strBuff.size() + 1024 * 4);
        }

        m_strBuff.append(pStr->c_str(), pStr->size() + 1);
    }

    // Rewrite length
    uint32ToLE((uint32_t)m_strBuff.size() - nStrTableStartPos, (uint8_t*)m_strBuff.data() + nStrTableStartPos + LEN_BLOCK_NAME);
}

void CMLBinXMLWriter::writeContentBlock(int indexSize) {
    m_strBuff.reserve(m_strBuff.size() + m_vContentBlock.size() * indexSize
        + LEN_BLOCK_HEADER + sizeof(uint32_t));
    m_strBuff.append(BX_BLOCK_CONTENT, LEN_BLOCK_NAME);
    appendUint32(m_strBuff, (uint32_t)m_vContentBlock.size() * indexSize);

    size_t start = m_strBuff.size();
    m_strBuff.resize(m_strBuff.size() + m_vContentBlock.size() * indexSize);
    uint8_t *data = (uint8_t *)m_strBuff.data() + start;
    if (indexSize == sizeof(uint8_t)) {
        for (int i = 0; i < (int)m_vContentBlock.size(); i++) {
            data[i] = (uint8_t)m_vContentBlock[i];
        }
    } else if (indexSize == sizeof(uint16_t)) {
        for (int i = 0; i < (int)m_vContentBlock.size(); i++) {
            uint16ToLE(m_vContentBlock[i], data);
            data += sizeof(uint16_t);
        }
    } else if (indexSize == sizeof(uint32_t)) {
        for (int i = 0; i < (int)m_vContentBlock.size(); i++) {
            uint32ToLE(m_vContentBlock[i], data);
            data += sizeof(uint32_t);
        }
    }
}

#if UNIT_TEST

#include "utils/unittest.h"


TEST(MLBinXMLParser, ParseBinXml) {
    CMLBinXMLWriter xmlWriter;

    xmlWriter.writeStartElement("Node");
    xmlWriter.writeAttribute("attr", "value");
    xmlWriter.writeAttribute("attr2", "value2");
    xmlWriter.writeStartElement("Node2");
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();

    string &buf = xmlWriter.getBuffer();

    CMLBinXMLParser parser;
    CSimpleXML xmlData;

    CMLBinXMLParser::MLBinXmlParseError ret = parser.parseData(buf.c_str(), buf.size(), xmlData);
    ASSERT_TRUE(ret == CMLBinXMLParser::E_OK);

    CMLBinXMLWriter xmlWriter2;
    xmlData.m_pRoot->toXML(xmlWriter2);
    string &buf2 = xmlWriter2.getBuffer();
    ASSERT_EQ(buf.size(), buf2.size());
    ASSERT_TRUE(memcmp(buf.c_str(), buf2.c_str(), buf.size()) == 0);
}

TEST(MLBinXMLParser, ParseBinXml2) {
    CMLBinXMLWriter xmlWriter;

    xmlWriter.writeStartElement("Node");
    xmlWriter.writeAttribute("attr", "value");
    xmlWriter.writeAttribute("attr2", "value2");
    xmlWriter.writeString("str");
    xmlWriter.writeStartElement("Node2");
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();

    string &buf = xmlWriter.getBuffer();

    CSimpleXML xmlData;

    ASSERT_TRUE(xmlData.parseData(buf.c_str(), buf.size()));

    CMLBinXMLWriter xmlWriter2;
    xmlData.m_pRoot->toXML(xmlWriter2);
    string &buf2 = xmlWriter2.getBuffer();
    ASSERT_TRUE(buf.size() == buf2.size());
    ASSERT_TRUE(memcmp(buf.c_str(), buf2.c_str(), buf.size()) == 0);
}

TEST(MLBinXMLParser, ParseBinXml3) {
    CMLBinXMLWriter xmlWriter;

#define SZ_BIN_DATA         "\0x1\0x2"

    xmlWriter.writeStartElement("Node");
    xmlWriter.writeAttribute("attr", "value");
    xmlWriter.writeAttribute("attr2", SZ_BIN_DATA, CountOf(SZ_BIN_DATA)); // bin data
    xmlWriter.writeEndElement();

    string &buf = xmlWriter.getBuffer();

    CMLBinXMLParser parser;
    CSimpleXML xmlData;

    CMLBinXMLParser::MLBinXmlParseError ret = parser.parseData(buf.c_str(), buf.size(), xmlData);
    ASSERT_TRUE(ret == CMLBinXMLParser::E_OK);

    ASSERT_TRUE(xmlData.m_pRoot->name == "Node");
    ASSERT_TRUE(strcmp(xmlData.m_pRoot->getProperty("attr"), "value") == 0);
    string bufAttr;
    xmlData.m_pRoot->getPropertyBinData("attr2", bufAttr);
    ASSERT_TRUE(bufAttr.size() == CountOf(SZ_BIN_DATA));
    ASSERT_TRUE(memcmp(SZ_BIN_DATA, bufAttr.c_str(), bufAttr.size()) == 0);
}

#endif
