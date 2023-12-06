#import <Cocoa/Cocoa.h>


@interface ColorChooserDelegate : NSObject <NSWindowDelegate>

- (void)windowWillClose:(NSNotification *)notification;

@end

#include "../WindowLib.h"
#import "WindowHandleHolder.h"


@implementation ColorChooserDelegate

- (void)windowWillClose:(NSNotification *)notification {
    printf("windowWillClose: %s", [[notification name] UTF8String]);
    [NSApp stopModalWithCode:IDOK];
}

@end

CDlgChooseColor::CDlgChooseColor() {
    m_clr = RGB(255, 255, 255);
}

CDlgChooseColor::~CDlgChooseColor() {

}

int CDlgChooseColor::doModal(Window *pWndParent, const CColor &clr) {
    m_clr = clr;

    NSColor *nsclr = [NSColor colorWithSRGBRed:(float)clr.r() / 255
        green:(float)clr.g() / 255
        blue:(float)clr.b() / 255
        alpha:1.0];

    NSColorPanel *panel = [NSColorPanel sharedColorPanel];
    if (pWndParent != nullptr) {
        [panel setParentWindow: pWndParent->getWindowHandleHolder()->window];
    }
    // [panel setWorksWhenModal:YES];
    ColorChooserDelegate *delegate = [ColorChooserDelegate alloc];
    [panel setDelegate:delegate];
    [panel setColor:nsclr];

    int nRet = (int)[NSApp runModalForWindow:panel];
    [delegate release];
    if (nRet != IDOK) {
        return nRet;
    }

    nsclr = [panel color];
    if (nsclr != nil) {
        CGFloat r, g, b, a;
        nsclr = [nsclr colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
        [nsclr getRed:&r green:&g blue:&b alpha:&a];

        m_clr.set(int(r * 255), int(g * 255), int(b * 255));
    }

    return IDOK;
}
