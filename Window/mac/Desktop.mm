#import <Cocoa/Cocoa.h>
#include "WindowTypes.h"
#include "Window.h"
#include "Desktop.h"


void openUrl(Window *pWnd, cstr_t szUrl) {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:szUrl]]];
}

bool isModifierKeyPressed(int nKey, uint32_t nFlags) {
    assert(nKey == MK_SHIFT || nKey == MK_CONTROL || nKey == MK_COMMAND || nKey == MK_ALT);
    return isFlagSet(nFlags, nKey);
}

CPoint getCursorPos() {
    NSPoint pt = [NSEvent mouseLocation];
    return CPoint(pt.x, pt.y);
}

// set new cursor, and return the old cursor.
bool setCursor(Cursor &Cursor) {
#pragma warning("Unplememented method: ");

    return true;
}

Window *findWindow(cstr_t szClassName, cstr_t szWindowName) {
#pragma warning("Unplememented method: ");

    return nullptr;
}

bool getMonitorRestrictRect(const CRect &rcIn, CRect &rcRestrict) {
    NSArray *screens = [NSScreen screens];
    int nMaxSpace = -1;
    for (int i = 0; i < [screens count]; i++) {
        NSRect screenVisibleFrame = [[screens objectAtIndex:i] visibleFrame];
        CRect rcScreen;
        rcScreen.top = screenVisibleFrame.origin.y;
        rcScreen.left = screenVisibleFrame.origin.x;
        rcScreen.bottom = rcScreen.top + screenVisibleFrame.size.height;
        rcScreen.right = rcScreen.left + screenVisibleFrame.size.width;
        CRect rc;
        rc.intersect(rcIn, rcScreen);
        int nSpace = rc.width() * rc.height();
        if (nMaxSpace < nSpace) {
            nMaxSpace = nSpace;
            rcRestrict = rcScreen;
        }
    }

    return nMaxSpace != -1;
}

bool copyTextToClipboard(cstr_t szText) {
    NSPasteboard *board = [NSPasteboard generalPasteboard];
    [board clearContents];
    [board setString:[NSString stringWithUTF8String:szText] forType:NSPasteboardTypeString];
    return true;
}

bool getClipBoardText(string &str) {
    NSPasteboard *board = [NSPasteboard generalPasteboard];
    NSString *ns = [board stringForType:NSPasteboardTypeString];
    if (ns == nil) {
        return false;
    }
    str = [ns UTF8String];
    return true;
}
