//
//  WindowHandleHolder.h
//  Mp3Player
//
//  Created by HongyongXiao on 2021/12/23.
//

#pragma once

#ifndef WindowHandleHolder_h
#define WindowHandleHolder_h

#include "ViewMacImp.h"
#include "WindowMacImp.h"


struct WindowHandleHolder {
    WindowMacImp                *window;
    ViewMacImp                  *view;
};

#endif /* WindowHandleHolder_h */
