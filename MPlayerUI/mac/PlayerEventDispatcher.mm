//
//  PlayerEventDispatcher.m
//  MiniLyricsMac
//
//  Created by Xiao Hongyong on 8/1/13.
//
//
#import <Cocoa/Cocoa.h>
#import "PlayerEventDispatcher.h"


@interface _PlayerEventDispatcherInternal : NSObject
{
    NSTimer *timer;
}

@end

#import "Player.h"
#import "MPlayerApp.h"

@implementation _PlayerEventDispatcherInternal

-(void)StartLyrDrawUpdate {
    dispatch_async(dispatch_get_main_queue(), ^{
        timer = [NSTimer scheduledTimerWithTimeInterval:((double)40) / 1000
            target:self
            selector:@selector(onTimer)
            userInfo:nil
            repeats:YES];
    });
}

-(void)StopLyrDrawUpdate {
    if (timer != nil) {
        [timer invalidate];
        timer = nil;
    }
}

-(void)onTimer {
    if (g_Player.isUseSeekTimeAsPlayingTime()) {
        return;
    }

    int nPlayPos = g_Player.getPlayPos();
    g_LyricData.SetPlayElapsedTime(nPlayPos);

    CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);

    dispatchPlayPosEvent(nPlayPos);
}

@end

CPlayerEventDispatcher::CPlayerEventDispatcher() {
    m_nTimeOutUpdateLyr = 40;
    m_pInternal = [[_PlayerEventDispatcherInternal alloc] init];
}

void CPlayerEventDispatcher::startLyrDrawUpdate() {
    [(_PlayerEventDispatcherInternal*)m_pInternal StartLyrDrawUpdate];
}

void CPlayerEventDispatcher::stopLyrDrawUpdate() {
    [(_PlayerEventDispatcherInternal*)m_pInternal StopLyrDrawUpdate];
}
