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

class CSkinImageButton : public CSkinButton {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)
public:
    CSkinImageButton();
    virtual ~CSkinImageButton();

    void draw(CRawGraph *canvas) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    void setContentImage(const RawImageDataPtr &image);

    void setColor(const CColor &clr, bool bAutoSelColor);

    CColor getColor() const
        { return m_clrContent; }

    void buttonUpAction() override;

protected:
    int                         m_nContentMarginX = 5, m_nContentMarginY = 5;
    CSFImage                    m_contentImage;
    CColor                      m_clrContent;
    bool                        m_bContentImage = true;
    bool                        m_isStretchContent = true;
    bool                        m_bAutoSelColor = false;

};

#endif // !defined(Skin_SkinButton_h)
