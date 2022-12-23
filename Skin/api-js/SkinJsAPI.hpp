//
//  SkinJsAPI.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2022/12/21.
//

#ifndef SkinJsAPI_hpp
#define SkinJsAPI_hpp

#include "JsSkinDocument.hpp"


enum SkinJsDataTypes {
    SJT_DOCUMENT,
    SJT_UIOBJECT,
    SJT_MOUSE_EVENT,
};

void initSkinJsAPIs();

#endif /* SkinJsAPI_hpp */
