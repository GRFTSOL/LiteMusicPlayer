//
//  SkinJsAPI.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2022/12/21.
//

#include "SkinJsAPI.hpp"
#include "strings/CommonString.hpp"
#include "../SkinFactory.h"
#include "../SkinWnd.h"


void registerDocument(VMRuntimeCommon *rt);
void registerUIObject(VMRuntimeCommon *rt);
void registerProfile(VMRuntimeCommon *rt);

void initSkinJsAPIs() {
    auto rt = VMRuntimeCommon::getInstance();

    registerDocument(rt);
    registerUIObject(rt);
    registerProfile(rt);
}
