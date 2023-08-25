//
//  JsUIObject.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2022/12/21.
//

#pragma once

#ifndef JsUIObject_hpp
#define JsUIObject_hpp

#include "interpreter/VirtualMachineTypes.hpp"
#include "objects/JsObjectX.hpp"


class CUIObject;

class JsUIObject : public JsObjectX {
public:
    JsUIObject(CUIObject *obj);

    virtual bool onSetValue(VMContext *ctx, const StringView &name, const JsValue &value) override;
    virtual JsValue onGetValue(VMContext *ctx, const StringView &name) override;
    virtual void onEnumAllProperties(VMContext *ctx, VecStringViews &names, VecJsValues &values) override;

    CUIObject *uiobject() const { return _obj; }

protected:
    CUIObject                   *_obj;

};

#endif /* JsUIObject_hpp */
