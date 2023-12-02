//
//  MediaKeyController.m
//

#import <Foundation/Foundation.h>
#import "MediaKeyController.h"
#import "imported/SPMediaKeyTap/SPMediaKeyTap.h"


@interface MediaKeyController()
{
    SPMediaKeyTap               *_mediaKeyTap;
    // AppleRemote                 *_remote;

    BOOL                        _isMediaKeyTapEnabled;
    BOOL                        _isMediakeyJustJumped;

    // true as long as the user holds the left,right,plus or minus on the remote control
    BOOL                        _isRemoteButtonHold;
}
@end

#import "../../MPlayer/Player.h"


@implementation MediaKeyController

- (instancetype)init
{
    self = [super init];
    if (self) {
        _isMediaKeyTapEnabled = true;
        _isMediakeyJustJumped = true;
        _isRemoteButtonHold = true;

        _mediaKeyTap = [[SPMediaKeyTap alloc] initWithDelegate:self];
        [self enableMediaKeySupport];
    }
    return self;
}

- (void)dealloc
{
    _mediaKeyTap = nil;
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

#pragma mark -
#pragma mark Media Key support

- (void)resetMediaKeyJump
{
    _isMediakeyJustJumped = NO;
}

- (void)enableMediaKeySupport
{
    if ([_mediaKeyTap startWatchingMediaKeys]) {
        _isMediaKeyTapEnabled = YES;
    }
}

- (void)disableMediaKeySupport
{
    _isMediaKeyTapEnabled = NO;
    [_mediaKeyTap stopWatchingMediaKeys];
}

- (void)mediaKeyTap:(SPMediaKeyTap*)keyTap receivedMediaKeyEvent:(NSEvent*)event
{
    if (_isMediaKeyTapEnabled) {
        assert([event type] == NSSystemDefined && [event subtype] == SPSystemDefinedEventMediaKeys);

        int keyCode = (([event data1] & 0xFFFF0000) >> 16);
        int keyFlags = ([event data1] & 0x0000FFFF);
        int keyState = (((keyFlags & 0xFF00) >> 8)) == 0xA;
        int keyRepeat = (keyFlags & 0x1);

        if (keyCode == NX_KEYTYPE_PLAY && keyState == 0) {
            g_player.playPause();
        }

        if ((keyCode == NX_KEYTYPE_FAST || keyCode == NX_KEYTYPE_NEXT) && !_isMediakeyJustJumped) {
            if (keyState == 0 && keyRepeat == 0) {
                g_player.next();
            } else if (keyRepeat == 1) {
                _isMediakeyJustJumped = YES;
                [self performSelector:@selector(resetMediaKeyJump)
                           withObject: NULL
                           afterDelay:0.25];
            }
        }

        if ((keyCode == NX_KEYTYPE_REWIND || keyCode == NX_KEYTYPE_PREVIOUS) && !_isMediakeyJustJumped) {
            if (keyState == 0 && keyRepeat == 0) {
                g_player.prev();
            } else if (keyRepeat == 1) {
                _isMediakeyJustJumped = YES;
                [self performSelector:@selector(resetMediaKeyJump)
                           withObject: NULL
                           afterDelay:0.25];
            }
        }
    }
}

/*
#pragma mark -
#pragma mark Apple Remote Control

- (void)startListeningWithAppleRemote
{
    [_remote startListening: self];
}

- (void)stopListeningWithAppleRemote
{
    [_remote stopListening:self];
}

- (void)executeHoldActionForRemoteButton:(NSNumber *)buttonIdentifierNumber
{
    if (_isRemoteButtonHold) {
        switch([buttonIdentifierNumber intValue]) {
            case kRemoteButtonRight_Hold:
                g_player.seekTo(g_player.getPlayPos() + 3000);
                break;
            case kRemoteButtonLeft_Hold:
                g_player.seekTo(g_player.getPlayPos() - 3000);
                break;
            case kRemoteButtonVolume_Plus_Hold:
                g_player.setVolume(g_player.getVolume() + 10);
                break;
            case kRemoteButtonVolume_Minus_Hold:
                g_player.setVolume(g_player.getVolume() - 10);
                break;
        }
        if (_isRemoteButtonHold) {
            [self performSelector:@selector(executeHoldActionForRemoteButton:)
                       withObject:buttonIdentifierNumber
                       afterDelay:0.25];
        }
    }
}

- (void)appleRemoteButton:(AppleRemoteEventIdentifier)buttonIdentifier
              pressedDown:(BOOL)pressedDown
               clickCount:(unsigned int)count
{
    switch(buttonIdentifier) {
        case k2009RemoteButtonFullscreen:
            break;
        case k2009RemoteButtonPlay:
            g_player.playPause();
            break;
        case kRemoteButtonPlay:
            if (count >= 2) {
                // full screen ?
                g_player.playPause();
            } else {
                g_player.playPause();
            }
            break;
        case kRemoteButtonVolume_Plus:
            g_player.setVolume(g_player.getVolume() + 10);
            break;
        case kRemoteButtonVolume_Minus:
            g_player.setVolume(g_player.getVolume() - 10);
            break;
        case kRemoteButtonRight:
            g_player.next();
            break;
        case kRemoteButtonLeft:
            g_player.prev();
            break;
        case kRemoteButtonRight_Hold:
        case kRemoteButtonLeft_Hold:
        case kRemoteButtonVolume_Plus_Hold:
        case kRemoteButtonVolume_Minus_Hold:
            // simulate an event as long as the user holds the button
            _isRemoteButtonHold = pressedDown;
            if (pressedDown) {
                NSNumber* buttonIdentifierNumber = [NSNumber numberWithInt:buttonIdentifier];
                [self performSelector:@selector(executeHoldActionForRemoteButton:)
                           withObject:buttonIdentifierNumber];
            }
            break;
        case kRemoteButtonMenu:
            // TODO:...
            break;
        case kRemoteButtonPlay_Sleep: {
            NSAppleScript * script = [[NSAppleScript alloc] initWithSource:@"tell application \"System Events\" to sleep"];
            [script executeAndReturnError:nil];
            break;
        }
        default:
            break;
    }
}
*/
@end
