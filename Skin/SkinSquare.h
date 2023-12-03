#pragma once

#ifndef Skin_SkinSquare_h
#define Skin_SkinSquare_h


#include "UIObject.h"


class CSkinSquare : public CUIObject {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinSquare();
    virtual ~CSkinSquare();

    void draw(CRawGraph *canvas) override;
    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    CColor                      m_clrBg;

};

#endif // !defined(Skin_SkinSquare_h)
