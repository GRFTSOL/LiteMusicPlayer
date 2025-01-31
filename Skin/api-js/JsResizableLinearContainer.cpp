//
//  JsResizableLinearContainer.cpp
//  MusicPlayer
//
//  Created by henry_xiao on 2022/12/23.
//

#include "JsResizableLinearContainer.hpp"
#include "Skin/SkinResizableLinearContainer.h"


const StringView SS_MODE = makeCommonString("mode");

JsResizableLinearContainer::JsResizableLinearContainer(CSkinResizableLinearContainer *obj) : JsUIObject(obj) {
}

bool JsResizableLinearContainer::onSetValue(VMContext *ctx, const StringView &name, const JsValue &value) {
    if (name.equal(SS_MODE)) {
        auto mode = ctx->runtime->toStringView(ctx, value);
        uiobject()->setMode(mode.toString().c_str());
    }

    return false;
}

JsValue JsResizableLinearContainer::onGetValue(VMContext *ctx, const StringView &name) {
    if (name.equal(SS_MODE)) {
        return ctx->runtime->pushString(uiobject()->getMode());
    }

    return jsValueEmpty;
}

void JsResizableLinearContainer::onEnumAllProperties(VMContext *ctx, VecStringViews &names, VecJsValues &values) {
    names.push_back(SS_MODE);
    values.push_back(ctx->runtime->pushString(uiobject()->getMode()));
}
