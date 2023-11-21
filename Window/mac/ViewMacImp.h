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
class Cursor;

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

inline CPoint NSPointToCPoint(const NSPoint &nsPoint, int wndHeight) {
    return CPoint((int)nsPoint.x, wndHeight - (int)nsPoint.y);
}

@interface ViewMacImp : NSView<NSTextInputClient> {
    Window *mBaseWnd;

    Cursor *mCursorToSet;
    NSCursor *mCursorDefault;

    // 输入法输入需要的成员
    bool mIsTextInputMode;
    bool mIsMarkedText;
    CPoint mCaretPosition;
    CGFloat mMagnification;

    NSPoint mInitPt;
}

- (void)setOwnerBaseWnd:(Window*)baseWnd;
- (void)setCursor:(Cursor *)cursor;

// 在编辑框开始/结束编辑时需要调用
- (void)startTextInput;
- (void)endTextInput;

// 编辑控件的光标位置改变后需要调用此函数
- (void)caretPositionChanged:(CPoint)point;

@end
