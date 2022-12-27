#include "Utils.h"
#include "XMLWriter.h"


CXMLWriter::CXMLWriter(bool bWriteStartDoc) {
    if (bWriteStartDoc) {
        writeStartDocument("utf-8");
    }
    m_indent = "  ";
}

CXMLWriter::~CXMLWriter() {
    assert(m_vElementStack.empty());
}

void CXMLWriter::writeStartDocument(cstr_t szEncoding) {
    m_strBuff += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";

    m_wp = WP_READY;
}

void CXMLWriter::writeEndDocument() {

}

void CXMLWriter::writeStartElement(cstr_t szElement) {
    if (m_wp == WP_ELEMENT_HEAD) {
        m_strBuff += ">\r\n";
    }

    for (int i = 0; i < (int)m_vElementStack.size(); i++) {
        m_strBuff += m_indent.c_str();
    }

    m_wp = WP_ELEMENT_HEAD;

    m_strBuff += "<";
    writeDirect(szElement);

    pushElementStack(szElement);
}

void CXMLWriter::writeAttribute(cstr_t szAttr, cstr_t szValue) {
    assert(m_wp == WP_ELEMENT_HEAD);

    m_strBuff += " ";
    writeDirect(szAttr);
    m_strBuff += "=\"";

    writeConvert(szValue);

    m_strBuff += "\"";
}

void CXMLWriter::writeAttribute(cstr_t szAttr, int value) {
    char szBuff[64];
    itoa(value, szBuff);
    writeAttribute(szAttr, szBuff);
}

void CXMLWriter::writeAttribute(cstr_t szAttr, const void *data, size_t lenData) {
    // Convert: Bin >> Base64
    string strB64 = base64Encode((uint8_t *)data, lenData);
    writeAttribute(szAttr, strB64.c_str());
}

void CXMLWriter::writeEndElement() {
    assert(m_wp == WP_ELEMENT_HEAD ||
        m_wp == WP_ELEMENT_BODY);

    if (m_wp == WP_ELEMENT_HEAD) {
        m_strBuff += "/>\r\n";
    } else if (m_wp == WP_ELEMENT_BODY) {
        if (m_strBuff[m_strBuff.size() - 1] == '\n') {
            for (int i = 1; i < (int)m_vElementStack.size(); i++) {
                m_strBuff += m_indent.c_str();
            }
        }

        m_strBuff += "</";
        m_strBuff += getCurElementName();
        m_strBuff += ">\r\n";
    }

    popElementStack();
    if (!m_vElementStack.empty()) {
        m_wp = WP_ELEMENT_BODY;
    } else {
        m_wp = WP_READY;
    }
}

void CXMLWriter::writeString(cstr_t szString) {
    assert(m_wp == WP_ELEMENT_HEAD);
    m_wp = WP_ELEMENT_BODY;

    m_strBuff += ">";

    writeConvert(szString);
}

void CXMLWriter::writeElementString(cstr_t szElement, cstr_t szString) {
    for (int i = 0; i < (int)m_vElementStack.size(); i++) {
        m_strBuff += m_indent.c_str();
    }

    m_strBuff += "<";
    writeDirect(szElement);
    m_strBuff += ">";

    writeConvert(szString);

    m_strBuff += "</";
    writeDirect(szElement);
    m_strBuff += ">\r\n";
}

void CXMLWriter::writeElementInt(cstr_t szElement, int value) {
    char buf[16];
    itoa(value, buf);
    writeElementString(szElement, buf);
}

void CXMLWriter::writeComment(cstr_t szComment) {
    for (int i = 0; i < (int)m_vElementStack.size() + 1; i++) {
        m_strBuff += m_indent.c_str();
    }

    m_strBuff += "<!-- ";
    writeConvert(szComment);
    m_strBuff += " -->\r\n";
}

void CXMLWriter::writeData(const char *szData, int n) {
    m_strBuff.append(szData, n);
}

bool CXMLWriter::saveAsFile(cstr_t szFile) {
    return writeFile(szFile, (void *)m_strBuff.c_str(), m_strBuff.size());
}

void CXMLWriter::writeDirect(cstr_t szBuff) {
    m_strBuff += szBuff;
}

void CXMLWriter::writeConvert(cstr_t szBuff) {
    cstr_t szContent = nullptr;
    szContent = szBuff;
    cstr_t szEnd;

    while (*szContent) {
        szEnd = szContent;
        while (*szEnd && *szEnd != '<' && *szEnd != '>'
            && *szEnd != '&' && *szEnd != '\"'
            && *szEnd != '\'') {
            szEnd++;
        }

        if ((int)(szEnd - szContent) > 0) {
            m_strBuff.append(szContent, (int)(szEnd - szContent));
        }
        if (*szEnd == '\0') {
            break;
        }

        switch (*szEnd) {
            case '<': m_strBuff += "&lt;"; break;
            case '>': m_strBuff += "&gt;"; break;
            case '&': m_strBuff += "&amp;"; break;
            case '\"': m_strBuff += "&quot;"; break;
            case '\'': m_strBuff += "&apos;"; break;
            default: assert(0);
        }
        szEnd++;
        szContent = szEnd;
    }
}

void CXMLWriter::pushElementStack(cstr_t szElement) {
    m_vElementStack.push_back(szElement);
}

cstr_t CXMLWriter::getCurElementName() {
    if (m_vElementStack.empty()) {
        return "";
    } else {
        return m_vElementStack.back().c_str();
    }
}
