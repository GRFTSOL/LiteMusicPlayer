// UIEditObject.h: interface for the CUIEditObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_)
#define AFX_UIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _OUR_MFC_LIB

// typedef vector<string>    CProperties;

class CUIObject;
interface ISESkinNotify;

class CUIEditObject : public CUIObject  
{
public:
    CUIEditObject();
    virtual ~CUIEditObject();

    virtual void onCreate();

    virtual bool onLButtonUp(uint32_t nFlags, CPoint point);
    virtual bool onLButtonDown(uint32_t nFlags, CPoint point);
    virtual bool onMouseMove(uint32_t nFlags, CPoint point);

    virtual void draw(CRawGraph *canvas);

    void setContainedUIObject(CUIObject *pObj) { m_pUIObject = pObj; }

    virtual bool reCalculatePos(FORMULA_VAR vars[]);

    virtual void enumProperties(CUIObjProperties &listProperties);

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue);

    virtual int fromXML(CSkinContainer *pContainer, SXNode *pXmlNode);

    virtual void toXML(CXMLWriter &xmlStream);

    void setNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }

public:
    //
    // CSkinContainer API
    //
    virtual bool isContainer() const { return m_pUIObject->isContainer(); }
    virtual CSkinContainer *getContainerIf() { return m_pUIObject->getContainerIf(); }

public:
    cstr_t getClassName() { return m_pUIObject->getClassName(); }
    bool isKindOf(cstr_t szClassName)
    {
        if (CUIObject::isKindOf(szClassName))
            return true;

        if (m_pUIObject->isKindOf(szClassName))
            return true;

        return strcasecmp(szClassName, ms_szClassName) == 0;
    }
    static cstr_t className() { return ms_szClassName; }
protected:
    static cstr_t        ms_szClassName;

protected:
    virtual void onKillFocus();
    virtual void onSetFocus();

protected:
    CUIObject        *m_pUIObject;
    bool            m_bLBtDown;
    CPoint            m_ptDragOld;            // drag begin pos

    HBRUSH            m_hbrBk;

    ISESkinNotify    *m_pNotify;

};

#endif // !defined(AFX_UIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_)
