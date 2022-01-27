/********************************************************************
    Created  :    2002/01/04    21:42
    FileName :    SkinButton.h
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#if !defined(AFX_SKINBUTTON_H__FD258491_71A3_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_SKINBUTTON_H__FD258491_71A3_11D5_9E04_02608CAD9330__INCLUDED_

#include "UIObject.h"
#include "SkinNStatusButton.h"

class CSkinButton : public CSkinNStatusButton  
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinNStatusButton)
public:
    CSkinButton();
    virtual ~CSkinButton();

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    bool isCheck();
    void setCheck(bool bCheck);

protected:
    virtual void buttonUpAction();

    bool            m_bRadioBt;

};

class CSkinActiveButton : public CSkinButton
{
UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)

public:
    void draw(CRawGraph *canvas);
    
    CSkinActiveButton();
    virtual ~CSkinActiveButton();

};

class CSkinImageButton : public CSkinButton
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)
public:
    CSkinImageButton();
    virtual ~CSkinImageButton();

    void draw(CRawGraph *canvas);

    bool setProperty(cstr_t szProperty, cstr_t szValue);

    void setContentImage(RawImageData *image);

    void setColor(const CColor &clr, bool bAutoSelColor);

    CColor getColor() const
        { return m_clrContent; }

    void buttonUpAction();

protected:
    int                    m_nContentMarginX, m_nContentMarginY;
    CSFImage                m_contentImage;
    CColor                m_clrContent;
    bool                m_bContentImage;
    bool                m_bAutoSelColor;

};

class CSkinHoverActionBtEventNotify : public IUIObjNotify
{
public:
    CSkinHoverActionBtEventNotify(CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_CLICK; }
    enum Command
    {
        C_CLICK,
        C_BEGIN_HOVER,
        C_HOVER_ACTION,
        C_END_HOVER,
    };

    Command                cmd;

};

class CSkinHoverActionButton : public CSkinButton
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)

public:
    CSkinHoverActionButton();
    virtual ~CSkinHoverActionButton();

    bool onLButtonUp(uint32_t nFlags, CPoint point);
    bool onMouseMove(CPoint point);

    void buttonUpAction();

    void onTimer(int nId);

protected:
    void dispatchEvent(CSkinHoverActionBtEventNotify::Command cmd);

    void endHover();

protected:
    int                    m_nTimerIdHoverAction;
    bool                m_bHoverActionBegin;
    uint32_t                m_dwBeginHoverTime;

};

#endif // !defined(AFX_SKINBUTTON_H__FD258491_71A3_11D5_9E04_02608CAD9330__INCLUDED_)
