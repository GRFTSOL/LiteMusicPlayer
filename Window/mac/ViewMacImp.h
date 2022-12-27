#pragma once

//
//  ViewMacImp.h
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

class Window;

inline void NSRectToRect(const NSRect &nsrc, CRect &rc) {
    rc.left = nsrc.origin.x;
    rc.top = nsrc.origin.y;
    rc.right = rc.left + nsrc.size.width;
    rc.bottom = rc.top + nsrc.size.height;
}

inline NSRect NSMakeRect(const CRect &rc) {
    NSRect nsrc;
    nsrc.origin.x = rc.left;
    nsrc.origin.y = rc.top;
    nsrc.size.width = rc.right - rc.left;
    nsrc.size.height = rc.bottom - rc.top;

    return nsrc;
}

@interface ViewMacImp : NSView {
    Window *mBaseWnd;
}

- (void)setOwnerBaseWnd:(Window*)baseWnd;

@end
