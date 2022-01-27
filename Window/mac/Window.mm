//
//  Window.cpp
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/15/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "WindowTypes.h"
#import "Window.h"
#import "WindowHandleHolder.h"


Window::Window()
{
    m_handleHolder = new WindowHandleHolder();
    m_handleHolder->window = nullptr;
    m_handleHolder->view = nullptr;
    m_bMouseCaptured = false;
    
    m_parent = nullptr;
}

Window::~Window()
{
    delete m_handleHolder;
}

bool Window::createForSkin(cstr_t szClassName, cstr_t szCaption, int x, int y, int nWidth, int nHeight, Window *pWndParent, bool bToolWindow, bool bTopmost, bool bVisible)
{
    NSRect frame = NSMakeRect(x, y, nWidth, nHeight);
    WindowMacImp* w = [[WindowMacImp alloc] initWithContentRect:frame
//                                              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                              styleMask:NSResizableWindowMask
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    [w setOwnerBaseWnd:this];
    
    ViewMacImp* view = [[ViewMacImp alloc] initWithFrame:frame];
    [view setOwnerBaseWnd:this];

    m_parent = pWndParent;
    m_handleHolder->window = w;
    m_handleHolder->view = view;

    [w setContentView:view];
    // [w setBackgroundColor:[NSColor blueColor]];
//    [w makeFirstResponder:view];
//    [w setAcceptsMouseMovedEvents:YES];
//    [w setRestorable:NO];

    if (szCaption)
        [w setTitle: [NSString stringWithUTF8String:szCaption]];

    if (pWndParent && pWndParent->m_handleHolder->window)
        [w setParentWindow:pWndParent->m_handleHolder->window];

    //[w setOpaque:NO];
    // [w setAlphaValue:(float)128 / 255];

    onCreate();

    if (bVisible)
        [w makeKeyAndOrderFront:w];
        // [w orderFront:NSApp];

    if (bTopmost)
        [w setLevel:NSFloatingWindowLevel];

    return true;
}

void Window::setHasShadow(bool hasShadow)
{
    [m_handleHolder->window setHasShadow:hasShadow ? YES : NO];
    [m_handleHolder->window invalidateShadow];
}

void Window::destroy()
{
    if (m_handleHolder->window)
        [m_handleHolder->window close];
}

void Window::postDestroy()
{
    destroy();
}

void Window::onPaint(CRawGraph *surface, CRect *rc)
{
}

void Window::activateWindow()
{
}

void Window::showNoActivate() {
    show();
}

void Window::show() {
    if (m_handleHolder->window)
        [m_handleHolder->window orderFront:NSApp];
}

void Window::hide() {
    if (m_handleHolder->window)
        [m_handleHolder->window miniaturize:m_handleHolder->window];
}

void Window::minimize() {
    if (m_handleHolder->window)
        [m_handleHolder->window miniaturize:m_handleHolder->window];
}

void Window::maximize() {
    if (m_handleHolder->window)
        [m_handleHolder->window zoom:m_handleHolder->window];
}

void Window::restore() {
    if (isZoomed())
        maximize();
    else
        minimize();
}

void Window::minimizeNoActivate() {
}
//bool Window::showWindow(int nCmdShow)
//{
//    case SW_SHOWMINIMIZED:
//        [m_handleHolder->window miniaturize:m_handleHolder->window];
//        break;
//    case SW_SHOWMAXIMIZED:
//        [m_handleHolder->window zoom:m_handleHolder->window];
//        break;
//    }
//
//    return true;
//}

void Window::screenToClient(CRect &rc)
{
    if (m_handleHolder->window == nullptr)
        return;

    NSRect nsrc = NSMakeRect(rc);
    NSRect rcRet = [m_handleHolder->window convertRectFromScreen:nsrc];

    NSRectToRect(rcRet, rc);
}

void Window::clientToScreen(CRect &rc)
{
    if (m_handleHolder->window == nullptr)
        return;

    NSRect nsrc = NSMakeRect(rc);
    nsrc.origin.y = [m_handleHolder->window frame].size.height - rc.top - nsrc.size.height;
    NSRect rcRet = [m_handleHolder->window convertRectToScreen:nsrc];

    NSRectToRect(rcRet, rc);
}

void Window::screenToClient(CPoint &pt)
{
    if (m_handleHolder->window == nullptr)
        return;
    
    NSRect nsrc = NSMakeRect(pt.x, pt.y, 0, 0);
    NSRect rcRet = [m_handleHolder->window convertRectFromScreen:nsrc];

    pt.x = rcRet.origin.x;
    pt.y = rcRet.origin.y;
}

void Window::clientToScreen(CPoint &pt)
{
    if (m_handleHolder->window == nullptr)
        return;
    
    NSRect nsrc = NSMakeRect(pt.x, pt.y, 0, 0);
    nsrc.origin.y = [m_handleHolder->window frame].size.height - pt.y;
    NSRect rcRet = [m_handleHolder->window convertRectToScreen:nsrc];
    
    pt.x = rcRet.origin.x;
    pt.y = rcRet.origin.y;
}

bool Window::getWindowRect(CRect* lpRect)
{
    if (m_handleHolder->window == nullptr)
        return false;
    
    NSRect rcRet = [m_handleHolder->window frame];
    NSRectToRect(rcRet, *lpRect);

    return true;
}

bool Window::getClientRect(CRect* lpRect)
{
    if (m_handleHolder->view == nullptr)
        return false;
    
    NSRect rcRet = [m_handleHolder->view frame];
    NSRectToRect(rcRet, *lpRect);

    return true;
}

void Window::setParent(Window *pWndParent)
{
    m_parent = pWndParent;

    if (m_handleHolder->window == nullptr)
        return;

    if (pWndParent && pWndParent->m_handleHolder->window)
        [m_handleHolder->window setParentWindow:pWndParent->m_handleHolder->window];
    else
        [m_handleHolder->window setParentWindow:nullptr];
}

Window *Window::getParent()
{
    return m_parent;
}

bool Window::setTimer(uint32_t nTimerId, uint32_t nElapse)
{
    if (m_handleHolder->window == nullptr)
        return false;

    [m_handleHolder->window setTimer:nTimerId duration:nElapse];

    return true;
}

void Window::killTimer(uint32_t nTimerId)
{
    if (m_handleHolder->window)
        [m_handleHolder->window killTimer:nTimerId];
}

string Window::getTitle()
{
    if (m_handleHolder->window == nullptr)
        return 0;
    
    NSString *s = [m_handleHolder->window title];
    if (s == nil)
        return 0;
    
    string str = [s UTF8String];

    return str;
}

void Window::setTitle(cstr_t szText)
{
    if (m_handleHolder->window != nullptr)
        [m_handleHolder->window setTitle: [NSString stringWithUTF8String:szText]];
}

bool Window::setWndCursor(Cursor *pCursor)
{
    return true;
}

bool Window::setFocus()
{
    [m_handleHolder->window makeKeyAndOrderFront:m_handleHolder->window];
    //[m_handleHolder->window orderFront:NSApp];
    // [m_handleHolder->window makeKeyWindow];
    return true;
}

bool Window::setCapture()
{
    m_bMouseCaptured = true;

    return true;
}

void Window::releaseCapture()
{
    m_bMouseCaptured = false;
}
//
//CGraphics *Window::getGraphics()
//{
//    if (m_handleHolder->window == nullptr)
//        return nullptr;
//
//    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
//
//    CGContextSaveGState(context);
//
//    CGContextTranslateCTM(context, 0, [m_handleHolder->view frame].size.height);
//    CGContextScaleCTM(context, 1.0, -1.0);
//
//    CGraphics        *canvas;
//
//    canvas = new CGraphics;
//    canvas->attach(context);
//
//    return canvas;
//}
//
//void Window::ReleaseGraphics(CGraphics *canvas)
//{
//    if (m_handleHolder->window == nullptr)
//        return;
//
//    CGContextRestoreGState(canvas->getHandle());
//
//    canvas->detach();
//
//    delete canvas;
//}

bool Window::invalidateRect(const CRect *lpRect, bool bErase)
{
    if (m_handleHolder->window != nullptr) {
        if (lpRect == nullptr)
            [m_handleHolder->view setNeedsDisplay:YES];
        else {
            int h = lpRect->bottom - lpRect->top;
            [m_handleHolder->view setNeedsDisplayInRect: NSMakeRect(lpRect->left, [m_handleHolder->view frame].size.height - lpRect->bottom, lpRect->right - lpRect->left, h)];
        }
    }

    return true;
}

bool Window::isChild()
{
    return false;
}

bool Window::isMouseCaptured()
{
    return m_bMouseCaptured;
}

bool Window::isIconic()
{
    if (m_handleHolder->window == nullptr)
        return false;

    return [m_handleHolder->window isMiniaturized] == YES;
}

bool Window::isZoomed()
{
    if (m_handleHolder->window == nullptr)
        return false;
    
    return [m_handleHolder->window isZoomed] == YES;
}

bool Window::isWindow()
{
    return m_handleHolder->window != nullptr;
}

bool Window::isValid()
{
    return m_handleHolder->window != nullptr;
}

bool Window::isVisible()
{
    if (m_handleHolder->window == nullptr)
        return false;
    
    return [m_handleHolder->window isVisible] == YES;
}

bool Window::isTopmost()
{
    if (m_handleHolder->window == nullptr)
        return false;

    return [m_handleHolder->window level] == NSFloatingWindowLevel;
}

bool Window::isSameWnd(Window *pWnd)
{
    return this == pWnd;
}

void Window::setTopmost(bool bTopmost)
{
    if (m_handleHolder->window == nullptr)
        return;

    if (bTopmost)
        [m_handleHolder->window setLevel: NSFloatingWindowLevel];
    else
        [m_handleHolder->window setLevel: NSNormalWindowLevel];
}

bool Window::isToolWindow()
{
    if (m_handleHolder->window == nullptr)
        return false;
    
    return [m_handleHolder->window level] == NSFloatingWindowLevel;
}

void Window::setToolWindow(bool bToolWindow)
{
    if (m_handleHolder->window == nullptr)
        return;
    
    if (bToolWindow)
        [m_handleHolder->window setLevel: NSFloatingWindowLevel];
    else
        [m_handleHolder->window setLevel: NSNormalWindowLevel];
}

bool Window::setForeground()
{
    if (m_handleHolder->window == nullptr)
        return false;
    
    [m_handleHolder->window orderFront:NSApp];

    return true;
}

void Window::setWindowPos(int x, int y)
{
    if (m_handleHolder->window == nullptr)
        return;

    [m_handleHolder->window setFrameOrigin:NSMakePoint(x, y)];
}

void Window::setWindowPosSafely(int x, int y)
{
    setWindowPos(x, y);
}

bool Window::moveWindow(int X, int Y, int nWidth, int nHeight, bool bRepaint)
{
    if (m_handleHolder->window == nullptr)
        return false;
    
    [m_handleHolder->window setFrame:NSMakeRect(X, Y, nWidth, nHeight) display: bRepaint ? YES : NO];
    
    return true;
}

bool Window::moveWindowSafely(int X, int Y, int nWidth, int nHeight, bool bRepaint)
{
    return moveWindow(X, Y, nWidth, nHeight, bRepaint);
}

int messageOut(cstr_t lpText, uint32_t uType, cstr_t lpCaption)
{
    uint32_t btType = uType & 0xF;
    NSString *defBt = nil;
    NSString *alterBt = nil;
    NSString *otherBt = nil;
    if (btType == MB_OKCANCEL) {
        defBt = [NSString stringWithUTF8String:("OK")];
        alterBt = [NSString stringWithUTF8String:("Cancel")];
    } else if (btType == MB_YESNOCANCEL) {
        defBt = [NSString stringWithUTF8String:("Yes")];
        alterBt = [NSString stringWithUTF8String:("Cancel")];
        otherBt = [NSString stringWithUTF8String:("NO")];
    } else if (btType == MB_YESNO) {
        defBt = [NSString stringWithUTF8String:("Yes")];
        alterBt = [NSString stringWithUTF8String:("NO")];
    }
    
    NSAlert *alert = [NSAlert alertWithMessageText:[NSString stringWithUTF8String:lpCaption]
                                     defaultButton:defBt
                                   alternateButton:alterBt
                                       otherButton:otherBt
                         informativeTextWithFormat:@"%@", [NSString stringWithUTF8String:lpText]];
    
    int nRet = (int)[alert runModal];
    if (btType == MB_OKCANCEL) {
        if (nRet == NSAlertFirstButtonReturn)
            return IDOK;
        else
            return IDCANCEL;
    } else if (btType == MB_YESNOCANCEL) {
        if (nRet == NSAlertFirstButtonReturn)
            return IDYES;
        else if (nRet == NSAlertOtherReturn)
            return IDNO;
        else
            return IDCANCEL;
    } else if (btType == MB_YESNO) {
        if (nRet == NSAlertFirstButtonReturn)
            return IDYES;
        else
            return IDNO;
    } else {
        return IDOK;
    }
}

int Window::messageOut(cstr_t lpText, uint32_t uType, cstr_t lpCaption)
{
    uint32_t btType = uType & 0xF;
    NSString *defBt = nil;
    NSString *alterBt = nil;
    NSString *otherBt = nil;
    if (btType == MB_OKCANCEL) {
        defBt = [NSString stringWithUTF8String:("OK")];
        alterBt = [NSString stringWithUTF8String:("Cancel")];
    } else if (btType == MB_YESNOCANCEL) {
        defBt = [NSString stringWithUTF8String:("Yes")];
        alterBt = [NSString stringWithUTF8String:("Cancel")];
        otherBt = [NSString stringWithUTF8String:("NO")];
    } else if (btType == MB_YESNO) {
        defBt = [NSString stringWithUTF8String:("Yes")];
        alterBt = [NSString stringWithUTF8String:("NO")];
    }
    
    NSString *title;
    if (lpCaption == nullptr)
        title = [m_handleHolder->window title];
    else
        title = [NSString stringWithUTF8String:lpCaption];
    NSAlert *alert = [NSAlert alertWithMessageText:title
                                    defaultButton:defBt
                                  alternateButton:alterBt
                                      otherButton:otherBt
                        informativeTextWithFormat:@"%@", [NSString stringWithUTF8String:lpText]];
    
    int nRet = (int)[alert runModal];
    if (btType == MB_OKCANCEL) {
        if (nRet == NSAlertFirstButtonReturn)
            return IDOK;
        else
            return IDCANCEL;
    } else if (btType == MB_YESNOCANCEL) {
        if (nRet == NSAlertFirstButtonReturn)
            return IDYES;
        else if (nRet == NSAlertOtherReturn)
            return IDNO;
        else
            return IDCANCEL;
    } else if (btType == MB_YESNO) {
        if (nRet == NSAlertFirstButtonReturn)
            return IDYES;
        else
            return IDNO;
    } else {
        return IDOK;
    }
}

bool Window::replaceChildPos(int nIDChildSrcPos, Window *pChildNew)
{
    return true;
}

void Window::postUserMessage(int nMessageID, LPARAM param)
{
    if (m_handleHolder->window == nullptr)
        return;
    
    NSArray *msg = [NSArray arrayWithObjects:[NSNumber numberWithInt:nMessageID], [NSNumber numberWithLong:param], nil];
    
    [m_handleHolder->window performSelectorOnMainThread:@selector(onUserMsg:)
                                     withObject:msg
                                  waitUntilDone:false];
}

void Window::setTransparent(uint8_t nAlpha, bool bClickThrough)
{
    m_nAlpha = nAlpha;
    m_bClickThrough = bClickThrough;
    
    if (nAlpha == 255)
    {
        // [m_handleHolder->window setOpaque:YES];
        [m_handleHolder->window setAlphaValue:1.0];
    }
    else
    {
        // [m_handleHolder->window setOpaque:NO];
        [m_handleHolder->window setAlphaValue:(float)nAlpha / 255];
    }

    // NOTE: [window setIgnoresMouseEvents] or NSResizableWindowMask flag will affect the mouse click through
    bool ignoresMouseEvents = isClickThrough() ? YES : NO;
    if ([m_handleHolder->window ignoresMouseEvents] != ignoresMouseEvents)
        [m_handleHolder->window setIgnoresMouseEvents: ignoresMouseEvents];
}
//
//bool Window::UpdateLayeredWindowUsingMemGraph(CRawGraph *canvas)
//{
//    return false;
//}
//
//void *Window::getHandle()
//{
//    return (void *)m_handleHolder->window;
//}
//
