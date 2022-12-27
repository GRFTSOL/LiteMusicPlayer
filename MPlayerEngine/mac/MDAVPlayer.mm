#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

@interface _MDAVPlayer : NSObject 
{
    AVPlayer *mPlayer;
    class CMDAVPlayer    *mMDAVPlayer;
}

@end

#import "IMPlayer.h"
#import "MDAVPlayer.h"
#import "MPTime.h"
#import "MPlayer.h"


static void *AVSPPlayerItemStatusContext = &AVSPPlayerItemStatusContext;
static void *AVSPPlayerRateContext = &AVSPPlayerRateContext;


@implementation _MDAVPlayer

- (id)init:(CMDAVPlayer *)player {
    self = [super init];
    if (self) {
        mPlayer = [[AVPlayer alloc] init];
        mMDAVPlayer = player;

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

    mMDAVPlayer->getMPlayer()->notifyEod(mMDAVPlayer, ERR_OK);
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
            if (mMDAVPlayer->getMPlayer()) {
                mMDAVPlayer->getMPlayer()->notifyEod(mMDAVPlayer, ERR_OK);
            }
        }
    } else if (context == AVSPPlayerRateContext) {
        float rate = [[change objectForKey:NSKeyValueChangeNewKey] floatValue];
        if (rate != 1.f) {
            CMTime pos = [mPlayer currentTime];
            CMTime length = [[mPlayer currentItem] duration];
            if (pos.value * 1.f / pos.timescale + 1 >= length.value * 1.f / length.timescale) {
                // + 0.5 是因为结束时，pos 比 length 小.
                if (mMDAVPlayer->getMPlayer()) {
                    mMDAVPlayer->getMPlayer()->notifyEod(mMDAVPlayer, ERR_OK);
                }
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

CMDAVPlayer::CMDAVPlayer(void) {
    OBJ_REFERENCE_INIT;

    init();
}

CMDAVPlayer::~CMDAVPlayer(void) {
}

cstr_t CMDAVPlayer::getDescription() {
    return "AVPlayer";
}

cstr_t CMDAVPlayer::getFileExtentions() {
    return ".mp3|MP3 files|.mp4|MP4 files|.wma|WMA files|.mp2|MP2 files";
}

MLRESULT CMDAVPlayer::getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia) {
    if (!isOK()) {
        return ERR_DECODER_INIT_FAILED;
    }

    AVURLAsset *asset = [AVAsset assetWithURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String: pInput->getSource()]]];

    if (![asset isPlayable] || [asset hasProtectedContent]) {
        return ERR_NOT_SUPPORT;
    }

    CMTime duration = [asset duration];
    if (duration.timescale > 0) {
        pMedia->setAttribute(MA_DURATION, (int)(duration.value * 1000 / duration.timescale));
    }

    /*
    m_player = [[AVPlayer alloc] init];

    CMPAutoPtr<IWMPMedia>    media;
    HRESULT hr = m_collection->add((BSTR)pInput->getSource(), &media);
    if (FAILED(hr))
        return HResultToMLResult(hr);

    double duration;
    hr = media->get_duration(&duration);
    if (SUCCEEDED(hr))
        pMedia->setAttribute(MA_DURATION, (int)(duration * 1000));

    bstr_s    artist, album, title;

    BSTR    pbstrItemName = SysAllocString(L"Artist");
    hr = media->getItemInfo(pbstrItemName, &artist);
    SysFreeString(pbstrItemName);
    if (SUCCEEDED(hr))
        pMedia->setAttribute(MA_ARTIST, artist.c_str());

    pbstrItemName = SysAllocString(L"Album");
    hr = media->getItemInfo(pbstrItemName, &album);
    SysFreeString(pbstrItemName);
    if (SUCCEEDED(hr))
        pMedia->setAttribute(MA_ALBUM, album.c_str());

    pbstrItemName = SysAllocString(L"Title");
    hr = media->getItemInfo(pbstrItemName, &title);
    SysFreeString(pbstrItemName);
    if (SUCCEEDED(hr))
        pMedia->setAttribute(MA_TITLE, title.c_str());
*/
    return ERR_OK;
}

//
// decode media file related methods
//

bool CMDAVPlayer::isSeekable() {
    return true;
}

bool CMDAVPlayer::isUseOutputPlug() {
    return false;
}

MLRESULT CMDAVPlayer::play(IMPlayer *pPlayer, IMediaInput *pInput) {
    if (!isOK()) {
        return ERR_DECODER_INIT_FAILED;
    }

    [(_MDAVPlayer*)m_player play: [NSString stringWithUTF8String:pInput->getSource()]];

    return ERR_OK;
}

MLRESULT CMDAVPlayer::pause() {
    if (!isOK()) {
        return ERR_DECODER_INIT_FAILED;
    }

    [(_MDAVPlayer*)m_player pause];

    return ERR_OK;
}

MLRESULT CMDAVPlayer::unpause() {
    if (!isOK()) {
        return ERR_DECODER_INIT_FAILED;
    }

    [(_MDAVPlayer*)m_player unpause];

    return ERR_OK;
}

MLRESULT CMDAVPlayer::stop() {
    if (!isOK()) {
        return ERR_DECODER_INIT_FAILED;
    }

    [(_MDAVPlayer*)m_player stop];

    return ERR_OK;
}

uint32_t CMDAVPlayer::getLength() {
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

MLRESULT CMDAVPlayer::seek(uint32_t nPos) {
    if (!isOK()) {
        return ERR_DECODER_INIT_FAILED;
    }

    [[(_MDAVPlayer*)m_player player] seekToTime:CMTimeMake(nPos, 1000)];

    return ERR_OK;
}

uint32_t CMDAVPlayer::getPos() {
    if (!isOK()) {
        return 0;
    }

    CMTime time = [[(_MDAVPlayer*)m_player player] currentTime];

    if (time.timescale == 0) {
        return 0;
    }
    return (uint32_t)(time.value * 1000 / time.timescale);
}

MLRESULT CMDAVPlayer::setVolume(int volume, int nBanlance) {
    if (!isOK()) {
        return ERR_DECODER_INIT_FAILED;
    }

    [[(_MDAVPlayer*)m_player player] setVolume:volume / 100.0f];

    return ERR_OK;
}

bool CMDAVPlayer::init() {
    m_player = [[_MDAVPlayer alloc] init:this];

    return true;
}

MLRESULT CMDAVPlayer::doDecode(IMedia *pMedia) {
    if (!isOK()) {
        return ERR_DECODER_INIT_FAILED;
    }

    MLRESULT nRet;
    CXStr strMedia;
    CMPAutoPtr<IMediaInput> pInput;
    CMPTime timePlayed;

    nRet = pMedia->getSourceUrl(&strMedia);
    if (nRet != ERR_OK) {
        return nRet;
    }

    nRet = m_pPlayer->m_pluginMgrAgent.newInput(strMedia.c_str(), &pInput);
    if (nRet != ERR_OK) {
        return nRet;
    }

    getMediaInfo(m_pPlayer, pInput, pMedia);

    nRet = play(m_pPlayer, pInput);
    if (nRet != ERR_OK) {
        if (nRet == ERR_MI_NOT_FOUND && pMedia->getID() != MEDIA_ID_INVALID) {
            // the source can't be opened, set it as deleted.
            m_pPlayer->m_pMediaLib->setDeleted(&pMedia);
        }
        goto R_FAILED;
    }

    if (m_pPlayer->m_volume != -1) {
        if (m_pPlayer->m_bMute) {
            setVolume(0, 0);
        } else {
            setVolume(m_pPlayer->m_volume, m_pPlayer->m_balance);
        }
    }

    // set the play time
    timePlayed.getCurrentTime();
    pMedia->setAttribute(MA_TIME_PLAYED, (int)timePlayed.m_time);

    if (pMedia->getID() == MEDIA_ID_INVALID && m_pPlayer->isAutoAddToMediaLib()) {
        m_pPlayer->m_pMediaLib->add(pMedia);
    }

    // If media info in media library isn't up to date,
    // update to media library.
    if (pMedia->getID() != MEDIA_ID_INVALID && !pMedia->isInfoUpdatedToMediaLib()) {
        m_pPlayer->m_pMediaLib->updateMediaInfo(pMedia);
    }

    return ERR_OK;

R_FAILED:
    return nRet;
}

bool CMDAVPlayer::isOK() {
    return m_player != nullptr && [(_MDAVPlayer*)m_player player] != nil;
}
