#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

@interface _MDAVPlayer : NSObject 
{
    AVPlayer *mPlayer;
    class CoreAVPlayer    *mCoreAVPlayer;
}

@end

#import "CoreAVPlayer.h"
#import "../IPlayerCore.hpp"
#import "../../MediaTags/MediaTags.h"


static void *AVSPPlayerItemStatusContext = &AVSPPlayerItemStatusContext;
static void *AVSPPlayerRateContext = &AVSPPlayerRateContext;


@implementation _MDAVPlayer

- (id)init:(CoreAVPlayer *)player {
    self = [super init];
    if (self) {
        mPlayer = [[AVPlayer alloc] init];
        mCoreAVPlayer = player;

        [self addObserver:self forKeyPath:@"player.rate" options:NSKeyValueObservingOptionNew context:AVSPPlayerRateContext];
        [self addObserver:self forKeyPath:@"player.currentItem.status" options:NSKeyValueObservingOptionNew context:AVSPPlayerItemStatusContext];
    }

    return self;
}

- (bool)play:(NSString *)file {
    // create an asset with our URL, asychronously load its tracks, its duration, and whether it's playable or protected.
    // When that loading is complete, configure a player to play the asset.
    AVURLAsset *asset = [AVURLAsset assetWithURL:[NSURL fileURLWithPath:file]];

    if (![asset isPlayable] || [asset hasProtectedContent]) {
        return false;
    }

    // create a new AVPlayerItem and make it our player's current item.
    AVPlayerItem *playerItem = [AVPlayerItem playerItemWithAsset:asset];
    [mPlayer replaceCurrentItemWithPlayerItem:playerItem];

    [mPlayer play];

    return true;
}

- (void)pause {
    [mPlayer pause];
}

- (void)unpause {
    [mPlayer play];
}

- (void)stop {
    [mPlayer pause];
    [mPlayer seekToTime:CMTimeMake(0, 1)];

    mCoreAVPlayer->notifyEndOfPlaying();
}

- (AVPlayer *)player {
    return mPlayer;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    if (context == AVSPPlayerItemStatusContext) {
        AVPlayerItemStatus status = (AVPlayerItemStatus)[[change objectForKey:NSKeyValueChangeNewKey] integerValue];
        bool enable = NO;
        switch (status) {
        case AVPlayerItemStatusUnknown:
            enable = YES;
            break;
        case AVPlayerItemStatusReadyToPlay:
            enable = YES;
            break;
        case AVPlayerItemStatusFailed:
            break;
        }

        if (!enable) {
            mCoreAVPlayer->notifyEndOfPlaying();
        }
    } else if (context == AVSPPlayerRateContext) {
        float rate = [[change objectForKey:NSKeyValueChangeNewKey] floatValue];
        if (rate != 1.f) {
            CMTime pos = [mPlayer currentTime];
            CMTime length = [[mPlayer currentItem] duration];
            if (pos.value * 1.f / pos.timescale + 1 >= length.value * 1.f / length.timescale) {
                // + 0.5 是因为结束时，pos 比 length 小.
                mCoreAVPlayer->notifyEndOfPlaying();
            }
        }
        //        if (rate != 1.f)
        //        {
        //            [[self playPauseButton] setTitle:@"Play"];
        //        }
        //        else
        //        {
        //            [[self playPauseButton] setTitle:@"Pause"];
        //        }
    } else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

- (void)close {
    [[self player] pause];
    [self removeObserver:self forKeyPath:@"player.rate"];
    [self removeObserver:self forKeyPath:@"player.currentItem.status"];
}

@end

//////////////////////////////////////////////////////////////////////////

CoreAVPlayer::CoreAVPlayer(void) {
    m_player = [[_MDAVPlayer alloc] init:this];
    m_state = PS_STOPPED;
}

CoreAVPlayer::~CoreAVPlayer(void) {
}

void CoreAVPlayer::quit() {
    if (m_player) {
        [(_MDAVPlayer*)m_player release];
        m_player = nullptr;
    }
}

// 实现的说明
const char *CoreAVPlayer::getDescription() {
    return "Mac OS System AVPlayer Implementation";
}

// 支持的文件扩展名
const char *CoreAVPlayer::getFileExtentions()  {
    return ".mp3|MP3 files|.mp4|MP4 files|.wma|WMA files|.mp2|MP2 files|.m4a|M4A files|.flac|FLAC files|.aac|AAC files";
}

// 获取媒体的标签
bool CoreAVPlayer::getMediaInfo(const char *mediaUrl, IMediaInfo *pMedia)  {
    // 不获取信息，由外部获取.
    return false;
}

//
// Player control
//
bool CoreAVPlayer::play(const char *mediaUrl, IMediaInfo *mediaTagsOut)  {
    if (!isOK()) {
        return false;
    }

    m_state = PS_PLAYING;

    if (![(_MDAVPlayer*)m_player play: [NSString stringWithUTF8String:mediaUrl]]) {
        return false;
    }

    getMediaInfo(mediaUrl, mediaTagsOut);

    return true;
}

bool CoreAVPlayer::pause()  {
    if (!isOK()) {
        return false;
    }

    m_state = PS_PAUSED;
    [(_MDAVPlayer*)m_player pause];

    return true;
}

bool CoreAVPlayer::unpause()  {
    if (!isOK()) {
        return false;
    }

    m_state = PS_PLAYING;
    [(_MDAVPlayer*)m_player unpause];

    return true;
}

bool CoreAVPlayer::stop()  {
    if (!isOK()) {
        return false;
    }

    [(_MDAVPlayer*)m_player stop];

    return true;
}


bool CoreAVPlayer::isSeekable()  {
    return true;
}

bool CoreAVPlayer::seek(int pos)  {
    if (!isOK()) {
        return false;
    }

    [[(_MDAVPlayer*)m_player player] seekToTime:CMTimeMake(pos, 1000)];

    return true;
}


//
// Current playing Media state
//
uint32_t CoreAVPlayer::getDuration() {
    if (!isOK()) {
        return 0;
    }

    AVPlayerItem *item = [[(_MDAVPlayer*)m_player player] currentItem];
    if (item == nil) {
        return 0;
    }

    CMTime duration = [item duration];
    return (uint32_t)(duration.value * 1000 / duration.timescale);
}

uint32_t CoreAVPlayer::getPos() {
    if (!isOK()) {
        return 0;
    }

    CMTime time = [[(_MDAVPlayer*)m_player player] currentTime];

    if (time.timescale == 0) {
        return 0;
    }
    return (uint32_t)(time.value * 1000 / time.timescale);
}

PlayerState CoreAVPlayer::getState() {
    return m_state;
}

//
// Player settings
//
// 0 ~ 100
bool CoreAVPlayer::setVolume(int volume) {
    if (!isOK()) {
        return false;
    }

    [[(_MDAVPlayer*)m_player player] setVolume:volume / 100.0f];

    return true;
}

int CoreAVPlayer::getVolume() {
    if (!isOK()) {
        return false;
    }

    return (int)[[(_MDAVPlayer*)m_player player] volume];
}

// -100 ~ 100
bool CoreAVPlayer::setBalance(int balance) {
    return false;
}

int CoreAVPlayer::getBalance() {
    return 0;
}

bool CoreAVPlayer::setEQ(const EQualizer *eq) {
    return false;
}

bool CoreAVPlayer::getEQ(EQualizer *eq) {
    return false;
}

bool CoreAVPlayer::isOK() {
    return m_player != nullptr && [(_MDAVPlayer*)m_player player] != nil;
}

void CoreAVPlayer::notifyEndOfPlaying() {
    m_state = PS_STOPPED;

    if (m_callback) {
        m_callback->onEndOfPlaying();
    }
}
