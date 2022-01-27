// SkinUIObjAny.h: interface for the CSkinUIObjAny class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINUIOBJANY_H__FA241B56_0221_47A3_8F47_11CA84F903DD__INCLUDED_)
#define AFX_SKINUIOBJANY_H__FA241B56_0221_47A3_8F47_11CA84F903DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinUIObjAny : public CUIObject  
{
public:
    CSkinUIObjAny();
    virtual ~CSkinUIObjAny();
public:

    virtual void draw(CRawGraph *canvas);

    bool setProperty(cstr_t szProperty, cstr_t szValue);

    void enumProperties(CUIObjProperties &listProperties);

    void setClassName(cstr_t szClassName) { m_strClassName = szClassName; }

public:
    cstr_t getClassName() { return m_strClassName.c_str(); }
    bool isKindOf(cstr_t szClassName)
    {
        if (CUIObject::isKindOf(szClassName))
            return true;

        if (strcasecmp(szClassName, m_strClassName.c_str()) == 0)
            return true;

        return strcasecmp(szClassName, ms_szClassName) == 0;
    }
    static cstr_t className() { return ms_szClassName; }
protected:
    static cstr_t        ms_szClassName;

protected:
    string        m_strClassName;
    vector<string>        m_vProperties;

};

#endif // !defined(AFX_SKINUIOBJANY_H__FA241B56_0221_47A3_8F47_11CA84F903DD__INCLUDED_)
