#pragma once

#ifndef Skin_SkinFadeArea_h
#define Skin_SkinFadeArea_h

#include "UIObject.h"


//
// This control is used to fade in/out the area that it covered.
//
class CSkinFadeArea : public CUIObject {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinFadeArea();
    virtual ~CSkinFadeArea();

    void draw(CRawGraph *canvas) override;

};

#endif // !defined(Skin_SkinFadeArea_h)
