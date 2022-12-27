//
//  ViewMacImp.m
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "WindowTypes.h"
#import "ViewMacImp.h"
#import "Window.h"


@implementation ViewMacImp

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];

    return self;
}

- (void)dealloc {
    [super dealloc];
}

/*
 When it's time to draw, this routine is called. This view is inside the window, the window's opaqueness has been turned off, and the window's styleMask has been set to NSBorderlessWindowMask on creation, so this view draws the all the visible content. The first two lines below fill the view with "clear" color, so that any images drawn also define the custom shape of the window.  Furthermore, if the window's alphaValue is less than 1.0, drawing will use transparency.
 */
- (void)drawRect:(NSRect)rect {
    // Clear the drawing rect.
    // [[NSColor clearColor] set];
    // NSRectFill(rect);



    //    NSRect r = [self frame];
    //    printf("Frame: x: %f, y: %f, width: %f, height: %f", r.origin.x, r.origin.y, r.size.width, r.size.height);
    //    printf("ClipRect: x: %f, y: %f, width: %f, height: %f", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);

    // A boolean tracks the previous shape of the window. If the shape changes, it's necessary for the
    // window to recalculate its shape and shadow.
    /*    bool shouldDisplayWindow = NO;
    // If the window transparency is > 0.7, draw the circle, otherwise, draw the pentagon. 
    if ([[self window] alphaValue] > 0.7) {
        shouldDisplayWindow = (showingPentagon == YES);
        showingPentagon = NO;
        [circleImage compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver];
    } else {
        shouldDisplayWindow = (showingPentagon == NO);
        showingPentagon = YES;
        [pentagonImage compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver];
    }
    // reset the window shape and shadow.
    if (shouldDisplayWindow) {
        [[self window] display];
        [[self window] setHasShadow:NO];
        [[self window] setHasShadow:YES];
    }*/

    CRect rcClip;
    NSRectToRect(rect, rcClip);

    // 需要将 mac 下的坐标体系转换一下.
    int h = rcClip.height();
    rcClip.top = [self frame].size.height - rcClip.bottom;
    rcClip.bottom = rcClip.top + h;

    mBaseWnd->onPaint(mBaseWnd->getMemGraphics(), &rcClip);
}

- (void)setOwnerBaseWnd:(Window*)baseWnd {
    mBaseWnd = baseWnd;
}

- (void)keyDown:(NSEvent *)theEvent {
    //    mBaseWnd->onKeyDown([theEvent keyCode], 0);
}

///////////////////////////////////////////////////////////////////////////
// View messages

/*- (void)viewWillStartLiveResize {
    NSRect rc = [self frame];
    mBaseWnd->onSize(rc.size.width, rc.size.height);
}*/

- (void)viewDidEndLiveResize {
    //    NSRect rc = [self frame];
    //    mBaseWnd->onSize(rc.size.width, rc.size.height);
}

@end
