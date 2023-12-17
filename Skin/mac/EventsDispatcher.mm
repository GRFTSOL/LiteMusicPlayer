#include "../SkinTypes.h"
#import <Foundation/Foundation.h>
#import <AppKit/NSApplication.h>
#include "../EventsDispatcherBase.h"
#include "../Skin.h"
#include "EventsDispatcher.h"


@interface EventsDispatchItem : NSObject {
    IEvent *m_pEvent;
    CEventsDispatcher *m_pDispacher;
}

- (id) initWithEvent:(IEvent *)pEvent dispatcher:(CEventsDispatcher *)pDispacher;

- (void) dispatch;

@end

@implementation EventsDispatchItem

- (id) initWithEvent:(IEvent *)pEvent dispatcher:(CEventsDispatcher *)pDispacher {
    m_pEvent = pEvent;
    m_pDispacher = pDispacher;
    return self;
}

- (void) dispatch {
    m_pDispacher->dispatchSyncEvent(m_pEvent);
}

@end

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CEventsDispatcher::init() {

    return CEventsDispatcherBase::init();
}

void CEventsDispatcher::quit() {
    CEventsDispatcherBase::quit();

}

void CEventsDispatcher::dispatchSyncEventByNoUIThread(IEvent *pEvent) {
    EventsDispatchItem *dispatchItem = [[EventsDispatchItem alloc] initWithEvent:pEvent
        dispatcher:this];

    [dispatchItem performSelectorOnMainThread:@selector(dispatch)
        withObject:nil
        waitUntilDone:true];
    [dispatchItem release];
}

void CEventsDispatcher::dispatchUnsyncEvent(IEvent *pEvent) {
    EventsDispatchItem *dispatchItem = [[EventsDispatchItem alloc] initWithEvent:pEvent
        dispatcher:this];

    [dispatchItem performSelectorOnMainThread:@selector(dispatch)
        withObject:nil
        waitUntilDone:false];
    [dispatchItem release];
}

void CEventsDispatcher::dispatchUnsyncEventDelayed(IEvent *pEvent, int delayInMs) {
    dispatch_async(dispatch_get_main_queue(), ^{
        [NSTimer scheduledTimerWithTimeInterval:((double)delayInMs / 1000)
                                        repeats:NO block:^(NSTimer *timer) {
            dispatchUnsyncEvent(pEvent);
        }];
    });
}

void CEventsDispatcher::postExecInUIThread(std::function<void()> f) {
    dispatch_async(dispatch_get_main_queue(), ^{
        f();
    });
}

void CEventsDispatcher::postExecInUIThreadDelayed(std::function<void()> f, int delayInMs) {
    dispatch_async(dispatch_get_main_queue(), ^{
        [NSTimer scheduledTimerWithTimeInterval:((double)delayInMs / 1000)
                                        repeats:NO block:^(NSTimer *timer) {
            f();
        }];
    });
}

//////////////////////////////////////////////////////////////////////

@interface CustomCmdPoster : NSObject {
    CSkinWnd *mSkinWnd;
    int mCmdId;

}

- (void) onCommand;
- (id) init:(CSkinWnd *)pWnd cmd:(int)cmdId;

@end

@implementation CustomCmdPoster

- (void) onCommand {
    mSkinWnd->onCommand(mCmdId);
}

- (id) init:(CSkinWnd *)pWnd cmd:(int)cmdId {
    mSkinWnd = pWnd;
    mCmdId = cmdId;
    return self;
}

@end

void postCommandMsgMac(CSkinWnd *pSkinWnd, int cmd) {
    CustomCmdPoster *poster = [[CustomCmdPoster alloc] init:pSkinWnd cmd:cmd];
    [poster performSelectorOnMainThread:@selector(onCommand)
        withObject:nil
        waitUntilDone:NO];
    [poster release];
}

void postQuitMessageMac() {
    [NSApp performSelectorOnMainThread:@selector(terminate:) withObject:nil waitUntilDone:NO];
    //    [NSApp terminate:nil];
}
