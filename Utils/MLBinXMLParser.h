#pragma once

#include "SimpleXML.h"
#include "XMLWriter.h"


class CMLBinXMLParser {
public:
    CMLBinXMLParser(void);
    virtual ~CMLBinXMLParser(void);

    enum MLBinXmlParseError {
        E_OK,
        E_INVALID_HEADER,
        E_UNKNOWN_VERSION,
        E_INVALID_SIZE,
        E_INVALID_STR_TABLE,
        E_INVALID_BIN_TABLE,
        E_INVALID_BLOCK_TYPE,
        E_NO_STR_TABLE,
        E_FMT_NEED_LBRACKET,
        E_FMT_NEED_ELEMENT_END,
    };

    MLBinXmlParseError parseData(const void *lpXmlData, size_t nLen, CSimpleXML &xmlData);

    static bool isHeaderType(cstr_t szHeader);

};

class CMLBinXMLWriter : public CXMLWriter {
public:
    CMLBinXMLWriter();
    virtual ~CMLBinXMLWriter();

    virtual void writeStartDocument(cstr_t szEncoding) { }

    virtual void writeEndDocument() { }

    // Begin xml element
    void writeStartElement(cstr_t szElement);

    // Writes: szAttr="szValue"
    void writeAttribute(cstr_t szAttr, cstr_t szValue);

    // write binary data attribute
    void writeAttribute(cstr_t szAttr, const void *data, size_t lenData);

    // close xml element
    void writeEndElement();

    // Writes the given text content: <node>szString</node>
    void writeString(cstr_t szString);

    // Writes: <szElement>szString</szElement>
    void writeElementString(cstr_t szElement, cstr_t szString);

    // Writes: <!-- szComment -->
    void writeComment(cstr_t szComment);

    string &getBuffer();

    bool isEmpty() const { return m_vContentBlock.empty(); }

    bool saveAsFile(cstr_t szFile);

protected:
    int getStrIndex(cstr_t str);

    void writeStrTable();
    void writeContentBlock(int indexSize);

protected:
    typedef map<string, int>    MapStrIndex;
    MapStrIndex                 m_mapStrTable;

    int                         m_nElementDeep;
    vector<int>                 m_vContentBlock;

    string                      m_bufBinTable;
    uint32_t                    m_nBinItemCount;

};
