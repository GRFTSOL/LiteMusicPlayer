#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>


@interface FontChooserDelegate : NSObject <NSWindowDelegate>

- (void)windowWillClose:(NSNotification *)notification;

@end

#import "../WindowLib.h"


CDlgChooseFont::CDlgChooseFont() {
}

CDlgChooseFont::~CDlgChooseFont() {

}

@implementation FontChooserDelegate

- (void)windowWillClose:(NSNotification *)notification {
    printf("windowWillClose: %s", [[notification name] UTF8String]);
    [NSApp stopModalWithCode:IDOK];
}

@end

int CDlgChooseFont::doModal(Window *pWndParent, cstr_t szFontFaceName, int nFontSize, int nWeight, int nItalic) {
    m_strFontFaceName = szFontFaceName;
    m_nFontSize = nFontSize;
    m_weight = nWeight;
    m_nItalic = nItalic;

    NSFont *oldFont = [NSFont fontWithName:[NSString stringWithUTF8String:szFontFaceName]
        size:nFontSize];
    if (oldFont == nil) {
        oldFont = [NSFont systemFontOfSize:nFontSize];
    }

    NSFontPanel *panel = [NSFontPanel sharedFontPanel];
    if (pWndParent != nullptr) {
        [panel setParentWindow:(NSWindow*)pWndParent->getHandleHolder()->window];
    }
    // [panel setWorksWhenModal:YES];
    FontChooserDelegate *delegate = [FontChooserDelegate alloc];
    [panel setDelegate:delegate];
    [panel setPanelFont:oldFont isMultiple:NO];

    auto nRet = [NSApp runModalForWindow:panel];
    [delegate release];
    if (nRet != IDOK) {
        return (int)nRet;
    }
    //[panel makeKeyAndOrderFront:nil];

    NSFont *newFont = [panel panelConvertFont:oldFont];
    m_strFontFaceName = [[newFont fontName] UTF8String];
    m_nFontSize = [newFont pointSize];

    return IDOK;
}

cstr_t CDlgChooseFont::getFaceName() {
    return m_strFontFaceName.c_str();
}

int CDlgChooseFont::getSize() {
    return m_nFontSize;
}

int CDlgChooseFont::getWeight() {
    return m_weight;
}

int CDlgChooseFont::getItalic() {
    return m_nItalic;
}
