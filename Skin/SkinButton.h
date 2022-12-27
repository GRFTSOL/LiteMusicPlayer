#pragma once

/********************************************************************
    Created  :    2002/01/04    21:42
    FileName :    SkinButton.h
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#ifndef Skin_SkinButton_h
#define Skin_SkinButton_h

#include "UIObject.h"
#include "SkinNStatusButton.h"


class CSkinButton : public CSkinNStatusButton {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinNStatusButton)
public:
    CSkinButton();
    virtual ~CSkinButton();

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    bool isCheck();
    void setCheck(bool bCheck);

protected:
    virtual void buttonUpAction() override;

    bool                        m_bRadioBt;

};

class CSkinActiveButton : public CSkinButton {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)

public:
    void draw(CRawGraph *canvas) override;

    CSkinActiveButton();
    virtual ~CSkinActiveButton();

};

class CSkinImageButton : public CSkinButton {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)
public:
    CSkinImageButton();
    virtual ~CSkinImageButton();

    void draw(CRawGraph *canvas) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    void setContentImage(RawImageData *image);

    void setColor(const CColor &clr, bool bAutoSelColor);

    CColor getColor() const
        { return m_clrContent; }

    void buttonUpAction() override;

protected:
    int                         m_nContentMarginX, m_nContentMarginY;
    CSFImage                    m_contentImage;
    CColor                      m_clrContent;
    bool                        m_bContentImage;
    bool                        m_bAutoSelColor;

};

class CSkinHoverActionBtEventNotify : public IUIObjNotify {
public:
    CSkinHoverActionBtEventNotify(CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_CLICK; }
    enum Command {
        C_CLICK,
        C_BEGIN_HOVER,
        C_HOVER_ACTION,
        C_END_HOVER,
    };

    Command                     cmd;

};

class CSkinHoverActionButton : public CSkinButton {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)

public:
    CSkinHoverActionButton();
    virtual ~CSkinHoverActionButton();

    bool onLButtonUp(uint32_t nFlags, CPoint point) override;
    bool onMouseMove(CPoint point) override;

    void buttonUpAction() override;

    void onTimer(int nId) override;

protected:
    void dispatchEvent(CSkinHoverActionBtEventNotify::Command cmd);

    void endHover();

protected:
    int                         m_nTimerIdHoverAction;
    bool                        m_bHoverActionBegin;
    int64_t                     m_timeBeginHover;

};

#endif // !defined(Skin_SkinButton_h)
