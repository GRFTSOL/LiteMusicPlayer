//
//  JsResizableLinearContainer.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2022/12/23.
//

#include "JsResizableLinearContainer.hpp"
#include "Skin/SkinResizableLinearContainer.h"


const SizedString SS_MODE = makeCommonString("mode");

JsResizableLinearContainer::JsResizableLinearContainer(CSkinResizableLinearContainer *obj) : JsUIObject(obj) {
}

bool JsResizableLinearContainer::onSetValue(VMContext *ctx, const SizedString &name, const JsValue &value) {
    if (name.equal(SS_MODE)) {
        auto mode = ctx->runtime->toSizedString(ctx, value);
        uiobject()->setMode(mode.toString().c_str());
    }

    return false;
}

JsValue JsResizableLinearContainer::onGetValue(VMContext *ctx, const SizedString &name) {
    if (name.equal(SS_MODE)) {
        return ctx->runtime->pushString(uiobject()->getMode());
    }

    return jsValueEmpty;
}

void JsResizableLinearContainer::onEnumAllProperties(VMContext *ctx, VecSizedStrings &names, VecJsValues &values) {
    names.push_back(SS_MODE);
    values.push_back(ctx->runtime->pushString(uiobject()->getMode()));
}
