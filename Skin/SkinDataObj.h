#ifndef _SKINDATAOBJ_H_
#define _SKINDATAOBJ_H_

#pragma once

#include "UIObject.h"

//
// CSkinDataObj is a special UIObject, only do data processing, invisible, no drawing and no other UI message processing.
//
class CSkinDataObj : public CUIObject
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinDataObj();
    ~CSkinDataObj();

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

};


#endif // _SKINDATAOBJ_H_
