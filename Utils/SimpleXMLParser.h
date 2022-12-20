// SimpleXMLParser1.h: interface for the CSimpleXMLParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLEXMLPARSER1_H__4A4EFF14_7DF0_4400_A46A_60723348DA5B__INCLUDED_)
#define AFX_SIMPLEXMLPARSER1_H__4A4EFF14_7DF0_4400_A46A_60723348DA5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Utils.h"


enum SimpleXMLParserError
{
    SXPE_OK,
    SXPE_OPEN_FILE_FAILED,
    SXPE_ALLOC_BUFFER,
    SXPE_READ_FILE_FAILED,
    SXPE_NOT_XML_FORMAT,
    SXPE_FORMAT_ERROR,
    SXPE_ONE_ROOT_ENTITY_ALLOWED,
};

enum ElementType
{
    ET_UNKNOWN,            // 
    ET_EOF_FILE,
    ET_ANY,
    ET_L_BRACKET,        // <
    ET_R_BRACKET,        // >
    ET_COMMENTS,        // <!-- -->
    ET_CDATA,            // <![CDATA[
    ET_EL_BRACKET,        // </
    ET_ER_BRACKET,        // />
    ET_L_XML_DECL,        // <?xml
    ET_R_XML_DECL,        // ?>
    ET_TEXT_VALUE,        // <name>text value<??>
    ET_SPACE,
    ET_NAME,            // Property name
    ET_EQ,                // Property assign
    ET_VALUE,            // Property value
};

class CSimpleXMLParser
{
public:
    class CDeepStack
    {
    public:
        CDeepStack(int *pDeep)
        {
            m_pDeep = pDeep;
            (*m_pDeep)++;
        }
        ~CDeepStack()
        {
            (*m_pDeep)--;
        }
        int        *m_pDeep;
    };

public:
    CSimpleXMLParser();
    virtual ~CSimpleXMLParser();

    SimpleXMLParserError parseData(const void *lpXmlData, size_t nLen, CSimpleXML &xmlData);

    int            nPos, nPosNext;
    bool        m_bNoProlog;

protected:
    void ignoreSpace(int nPos, int &nPosNext);

    ElementType readElemnt(ElementType desireType, string &strElemnt, int nPos, int &nPosNext);

    SimpleXMLParserError parseProlog(int nPos, int &nPosNext, CSimpleXML &xmlData);

    SimpleXMLParserError parseEntity(int nPos, int &nPosNext, SXNode *pNode);

    SimpleXMLParserError doParse(const char *szData, int nLen, CSimpleXML &xmlData);

protected:
    CharEncodingType    m_encodingID;

    const char        *m_szData;
    int                m_nLen;

    int                m_nDeepth;

};

#endif // !defined(AFX_SIMPLEXMLPARSER1_H__4A4EFF14_7DF0_4400_A46A_60723348DA5B__INCLUDED_)
