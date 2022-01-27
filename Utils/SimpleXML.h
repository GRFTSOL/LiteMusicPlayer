// SimpleXML.h: interface for the CSimpleXML class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLEXML_H__C6DFE0B7_DFB0_4DF5_937A_406F7FB35C5D__INCLUDED_)
#define AFX_SIMPLEXML_H__C6DFE0B7_DFB0_4DF5_937A_406F7FB35C5D__INCLUDED_


class CSimpleXML;
class CXMLWriter;

class SXNode;

SXNode *dupSXNode(SXNode *pOrgNode);

#define SURETEXT(p1)    (p1 ? p1 : "")

//////////////////////////////////////////////////////////////////////////
//
// Only keep basic XML data structure, discard other tags (exp: comments...)
//
// <name property="value">
//   <name>Content</name>
// </StrName>
class SXNode
{
public:
    typedef list<SXNode*>    LIST_CHILDREN;
    typedef list<string>    LIST_STRING;

    struct Property
    {
        string            name;
        string            strValue;
    };
    typedef list<Property>    ListProperties;
    typedef ListProperties::iterator    iterProperties;
    typedef LIST_CHILDREN::iterator    iterator;

    string                name;
    string                strContent;        // the content <nodeName>content</nodeName>
    ListProperties        listProperties;
    LIST_CHILDREN        listChildren;

public:
    // Can't be inherited.
    ~SXNode();

public:
    // If not exists, nullptr will be returned
    cstr_t getProperty(cstr_t szPropName);

    // If not exists, "" will be returned.
    cstr_t getPropertySafe(cstr_t szPropName);

    int getPropertyInt(cstr_t szPropName);
    int getPropertyInt(cstr_t szPropName, int nValueIfInexist);

    bool getPropertyBinData(cstr_t szPropName, string &buf);
    bool getPropertyBase64Data(cstr_t szPropName, string &buf);

    void addProperty(cstr_t szPropName, cstr_t szPropValue);
    void setProperty(cstr_t szPropName, cstr_t szPropValue);

    void eraseProperty(cstr_t szPropName);

    // get first child same with the node name
    SXNode *getChild(cstr_t szName);

    void toXML(CXMLWriter &xmlStream);

protected:
    ListProperties::iterator FindProperty(cstr_t szPropName);

};

class CSimpleXML  
{
public:
    CSimpleXML();

    // Can't be inherited.
    ~CSimpleXML();

    void New(cstr_t szRootName);

    bool parseFile(cstr_t szXmlFile, bool bNoProlog = false);

    bool parseData(const void *lpXmlData, size_t nLen, bool bNoProlog = false);

    bool saveFile(cstr_t szXmlFile);

    void free();

public:
    SXNode        *m_pRoot;
    string        m_encoding;

};

#endif // !defined(AFX_SIMPLEXML_H__C6DFE0B7_DFB0_4DF5_937A_406F7FB35C5D__INCLUDED_)
