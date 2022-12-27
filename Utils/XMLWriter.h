#pragma once

class CXMLWriter {
public:
    CXMLWriter(bool bWriteStartDoc = true);
    virtual ~CXMLWriter();

    // Writes: <?xml version=\"1.0\" encoding='"szEncoding"?>
    virtual void writeStartDocument(cstr_t szEncoding);

    virtual void writeEndDocument();

    // Begin xml element
    virtual void writeStartElement(cstr_t szElement);

    // Writes: szAttr="szValue"
    virtual void writeAttribute(cstr_t szAttr, cstr_t szValue);

    // write binary data attribute
    virtual void writeAttribute(cstr_t szAttr, const void *data, size_t lenData);

    // Writes: szAttr="value"
    virtual void writeAttribute(cstr_t szAttr, int value);

    // close xml element
    virtual void writeEndElement();

    // Writes the given text content: <node>szString</node>
    virtual void writeString(cstr_t szString);

    // Writes: <szElement>szString</szElement>
    virtual void writeElementString(cstr_t szElement, cstr_t szString);

    // Writes: <szElement>value</szElement>
    virtual void writeElementInt(cstr_t szElement, int value);

    // Writes: <!-- szComment -->
    virtual void writeComment(cstr_t szComment);

    // write Data directly, it's useful to joint/insert several xml node data.
    virtual void writeData(const char *szData, int n);

    // get xml data
    virtual string &getBuffer() { return m_strBuff; }

    virtual bool saveAsFile(cstr_t szFile);

    void setIndentStr(char *szIndentStr) { m_indent = szIndentStr; }

protected:
    void writeDirect(cstr_t szBuff);
    void writeConvert(cstr_t szBuff);

    void pushElementStack(cstr_t szElement);
    void popElementStack() { assert(!m_vElementStack.empty()); if (!m_vElementStack.empty()) m_vElementStack.pop_back(); }
    cstr_t getCurElementName();

protected:
    string                      m_indent;
    vector<string>              m_vElementStack;
    string                      m_strBuff;
    string                      m_strTemp;          // temp buffer to save new/delete time.

    enum WritePos {
        WP_READY,
        WP_ELEMENT_HEAD,
        WP_ELEMENT_BODY,
    };

    WritePos                    m_wp;

};
