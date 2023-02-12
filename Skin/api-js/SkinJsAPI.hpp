//
//  SkinJsAPI.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2022/12/21.
//

#pragma once

#ifndef SkinJsAPI_hpp
#define SkinJsAPI_hpp

#include "JsSkinDocument.hpp"


const SizedString SS_ON_COMMAND = makeCommonString("oncommand");
const SizedString SS_ON_MOUSE_MOVE = makeCommonString("onmousemove");
const SizedString SS_ON_MOUSE_ENTER = makeCommonString("onmouseenter");
const SizedString SS_ON_MOUSE_LEAVE = makeCommonString("onmouseleave");
const SizedString SS_ON_SIZE = makeCommonString("onsize");
const SizedString SS_ON_ACTIVATE = makeCommonString("onactivate");
const SizedString SS_ON_DESTORY = makeCommonString("ondestory");
const SizedString SS_ON_MOUSE_ACTIVATE = makeCommonString("onmouseactivate");
const SizedString SS_HEIGHT = makeCommonString("height");

enum SkinJsDataTypes {
    SJT_DOCUMENT,
    SJT_UIOBJECT,
    SJT_MOUSE_EVENT,
};

void initSkinJsAPIs();

#endif /* SkinJsAPI_hpp */
