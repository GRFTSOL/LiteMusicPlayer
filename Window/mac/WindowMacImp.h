//
//  WindowMacImp.h
//
//  Created by Hongyong Xiao on 11/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#pragma once

#import <AppKit/AppKit.h>

typedef map<int, NSTimer *> MapNSTimer;

class Window;

@interface WindowMacImp : NSWindow <NSWindowDelegate> {
    MapNSTimer mMapTimer;
    Window *mBaseWnd;
    float mYScroll, mXScroll;
}

- (void)setTimer:(int)idTimer duration:(int)duration;
- (void)killTimer:(int)idTimer;

- (void)onTimer:(NSTimer*)theTimer;

- (void)setOwnerBaseWnd:(Window*)baseWnd;

- (void)onUserMsg:(NSArray*)msg ;

@end
