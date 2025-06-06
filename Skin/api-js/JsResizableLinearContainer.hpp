﻿//
//  JsResizableLinearContainer.hpp
//  MusicPlayer
//
//  Created by henry_xiao on 2022/12/23.
//

#pragma once

#ifndef JsResizableLinearContainer_hpp
#define JsResizableLinearContainer_hpp

#include "JsUIObject.hpp"


class CSkinResizableLinearContainer;

class JsResizableLinearContainer : public JsUIObject {
public:
    JsResizableLinearContainer(CSkinResizableLinearContainer *obj);

    virtual bool onSetValue(VMContext *ctx, const StringView &name, const JsValue &value) override;
    virtual JsValue onGetValue(VMContext *ctx, const StringView &name) override;
    virtual void onEnumAllProperties(VMContext *ctx, VecStringViews &names, VecJsValues &values) override;

    CSkinResizableLinearContainer *uiobject() const { return (CSkinResizableLinearContainer *)_obj; }

};

#endif /* JsResizableLinearContainer_hpp */
