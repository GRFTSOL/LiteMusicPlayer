//
//  Profile.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2022/12/20.
//

#include "interpreter/VirtualMachineTypes.hpp"
#include "objects/JsLibObject.hpp"
#include "../SkinTypes.h"


void profile_getInt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto key = args.getStringAt(ctx, 0);
    int defVal = args.getIntAt(ctx, 1, 0);

    if (args.count < 1 || key.empty()) {
        ctx->throwException(JE_TYPE_ERROR, "Invalid parameters for profile.getInt");
        return;
    }

    ctx->retValue = makeJsValueInt32(g_profile.getInt("SkinJs", key.toString().c_str(), defVal));
}

void profile_writeInt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto key = args.getStringAt(ctx, 0);
    int val = args.getIntAt(ctx, 1);

    if (args.count < 2 || key.empty()) {
        ctx->throwException(JE_TYPE_ERROR, "Invalid parameters for profile.wrteInt");
        return;
    }

    g_profile.writeInt("SkinJs", key.toString().c_str(), val);
    ctx->retValue = jsValueUndefined;
}

static JsLibProperty profileFunctions[] = {
    { "name", nullptr, "Profile" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "getInt", profile_getInt },
    { "writeInt", profile_writeInt },
};

void registerProfile(VMRuntimeCommon *rt) {
    setGlobalLibObject("profile", rt, profileFunctions, CountOf(profileFunctions));
}
