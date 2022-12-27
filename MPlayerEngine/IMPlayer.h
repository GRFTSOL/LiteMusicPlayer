#pragma once

#ifndef _IMPLAYER_H_
#define _IMPLAYER_H_

#include "../Utils/Utils.h"


#ifndef _WIN32
#define interface           struct
#endif

interface IMediaDecode;
interface IMediaOutput;
interface IMediaInput;
interface IMediaLibrary;
interface IVis;
interface IDSP;

typedef int MLRESULT;

enum MPLAYER_ERROR_CODE {
    ERR_AUDIO_DECODE_INITFAILED = ERR_PLAYER_ERROR_BASE + 1,
    ERR_END_OF_STREAM,
    ERR_DECODE_FAILED,
    ERR_BUFFER_FULL,                 // 缓冲区已经写满了，需要等待
    ERR_NOT_INITED,                  // 未初始化
    ERR_NOT_SUPPORT,
    ERR_READ_ONLY,

    ERR_MI_OPEN_SRC,
    ERR_MI_NOT_FOUND,                // the media source doesn't exist.
    ERR_MI_SEEK,
    ERR_MI_READ,
    ERR_MI_WRITE,

    ERR_PLAYER_INVALID_STATE,        // Invalid state of current object.
    ERR_PLAYER_INVALID_FILE,
    ERR_SOUND_DEVICE_OPEN,
    ERR_SOUND_DEVICE_WRITE,
    ERR_DECODER_INNER_ERROR,
    ERR_DECODER_UNSUPPORTED_FEATURE,
    ERR_DECODER_INIT_FAILED,
    ERR_DISK_FULL,
    ERR_CREATE_THREAD,
    ERR_INVALID_HANDLE,
    ERR_END_OF_PLAYLIST,             // end of playlist, prev, next will return this value
    ERR_EMPTY_PLAYLIST,
    ERR_NO_DEVICE,

    ERR_ML_NOT_FOUND,                // Not found in media library.
};

//
// IMPlayer constant
//
enum {
    MP_VOLUME_MAX               = 100,
    MP_VOL_BALANCE_MIN          = -100,
    MP_VOL_BALANCE_MAX          = 100,

};

enum MP_LOOP_MODE {
    MP_LOOP_OFF,
    MP_LOOP_ALL,
    MP_LOOP_TRACK,
};

#ifndef _PLAYER_STATE_DEFINED
#define _PLAYER_STATE_DEFINED
enum PLAYER_STATE {
    PS_STOPED                   = 1,
    PS_PLAYING                  = 2,
    PS_PAUSED                   = 3
};
#endif // _PLAYER_STATE_DEFINED

enum MPInterfaceType {
    MPIT_INVALID                = 0,
    MPIT_INPUT                  = 1,
    MPIT_OUTPUT                 = 2,
    MPIT_DECODE                 = 3,
    MPIT_VIS                    = 4,
    MPIT_DSP                    = 5,
    MPIT_MEMALLOCATOR           = 6,
    MPIT_MEDIA_LIB              = 7,
    MPIT_GENERAL_PLUGIN         = 8,
    MPIT_INPUT_DECTOR           = 9,

};

enum {
    EQ_BANDS_COUNT              = 18,
    EQ_dB_MIN                   = -20,
    EQ_dB_DEF                   = 0,
    EQ_dB_MAX                   = 20,
};

struct EQualizer {
    bool                        bEnable;
    int                         nPreamp;            // set value EQ_dB_MAX to EQ_dB_MIN (default EQ_dB_DEF)
    int                         vData[EQ_BANDS_COUNT]; // set value EQ_dB_MAX to EQ_dB_MIN (default EQ_dB_DEF)
};


interface IString {
    virtual ~IString() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual char * data() = 0;
    virtual cstr_t c_str() = 0;
    virtual size_t size() = 0;
    virtual uint32_t capacity() = 0;
    virtual void resize(uint32_t nSize) = 0;
    virtual MLRESULT reserve(uint32_t nCapacity) = 0;
    virtual void copy(IString *pSrcStr) = 0;
    virtual void copy(cstr_t str) = 0;
    virtual void erase(int nOffset, int n) = 0;
    virtual void clear() = 0;
    virtual void insert(int nOffset, cstr_t str, int n) = 0;
    virtual void append(cstr_t str, int n) = 0;

};

interface IVString {
    virtual ~IVString() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual size_t size() = 0;
    virtual void push_back(cstr_t szStr) = 0;
    virtual void clear() = 0;
    virtual cstr_t at(int index) = 0;
    virtual void set(int index, cstr_t szStr) = 0;
    virtual void insert(int index, cstr_t szStr) = 0;

};

interface IVector {
    virtual ~IVector() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual size_t size() = 0;
    virtual void push_back(void *p) = 0;
    virtual void clear() = 0;
    virtual void *at(int index) = 0;
    virtual void set(int index, void *p) = 0;
    virtual void insert(int index, void *p) = 0;

};

interface IVInt {
    virtual ~IVInt() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual size_t size() = 0;
    virtual void push_back(int nData) = 0;
    virtual void clear() = 0;
    virtual int at(int index) = 0;
    virtual void set(int index, int nData) = 0;
    virtual void insert(int index, int nData) = 0;

};

interface IFBuffer {
    virtual ~IFBuffer() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual char *data() = 0;
    virtual uint32_t size() = 0;
    virtual uint32_t capacity() = 0;
    virtual void resize(uint32_t nSize) = 0;
    virtual MLRESULT reserve(uint32_t nCapacity) = 0;

};

interface IMemAllocator {
    virtual ~IMemAllocator() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual IFBuffer *allocFBuffer(uint32_t nCapacity) = 0;
    virtual IString *allocStr() = 0;

};
/*

interface IMediaTag
{
    virtual void addRef() = 0;
    virtual void release() = 0;

};
*/

#define MEDIA_LENGTH_INVALID 0
#define MEDIA_ID_INVALID    0

enum MediaAttribute {
    MA_ARTIST                   = 0,
    MA_ALBUM                    = 1,
    MA_TITLE                    = 2,
    MA_TRACK_NUMB               = 3,
    MA_YEAR                     = 4,
    MA_GENRE                    = 5,
    MA_COMMENT                  = 6,
    MA_DURATION                 = 7,
    MA_FILESIZE                 = 8,
    MA_TIME_ADDED               = 9,
    MA_TIME_PLAYED              = 10,
    MA_IS_USER_RATING           = 12,
    MA_RATING                   = 11,
    MA_TIMES_PLAYED             = 13,
    MA_TIMES_PLAY_SKIPPED       = 14,
    MA_LYRICS_FILE              = 15,

    // the following info will not be stored in media library
    MA_FORMAT                   = 16,
    MA_ISVBR                    = 17,
    MA_BITRATE                  = 18,
    MA_BPS                      = 19,
    MA_CHANNELS                 = 20,
    MA_SAMPLE_RATE              = 21,
    MA_EXTRA_INFO               = 22,
};

interface IMedia {
    virtual ~IMedia() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    // the ID in media library, 0 for not existing in media library
    virtual int getID() = 0;

    virtual MLRESULT getSourceUrl(IString *strUrl) = 0;
    virtual MLRESULT getArtist(IString *strArtist) = 0;
    virtual MLRESULT getTitle(IString *strTitle) = 0;
    virtual MLRESULT getAlbum(IString *strAlbum) = 0;

    virtual int getDuration() = 0;

    virtual MLRESULT setSourceUrl(cstr_t strUrl) = 0;

    virtual bool isInfoUpdatedToMediaLib() = 0;
    virtual MLRESULT setInfoUpdatedToMediaLib(bool bUpdated) = 0;

    //
    // attribute methods
    //
    virtual MLRESULT getAttribute(MediaAttribute mediaAttr, IString *strValue) = 0;
    virtual MLRESULT setAttribute(MediaAttribute mediaAttr, cstr_t szValue) = 0;

    virtual MLRESULT getAttribute(MediaAttribute mediaAttr, int *pnValue) = 0;
    virtual MLRESULT setAttribute(MediaAttribute mediaAttr, int value) = 0;

};

interface IPlaylist {
    virtual ~IPlaylist() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual uint32_t getCount() = 0;

    virtual MLRESULT getItem(int nIndex, IMedia **ppMedia) = 0;

    virtual MLRESULT getName(IString *str) = 0;

    virtual MLRESULT insertItem(int nIndex, IMedia *pMedia) = 0;

    virtual MLRESULT moveItem(int nIndexOld, int nIndexNew) = 0;

    virtual MLRESULT removeItem(int nIndex) = 0;

    virtual MLRESULT clear() = 0;
};

enum MediaLibOrderBy {
    MLOB_NONE,
    MLOB_ARTIST,
    MLOB_ALBUM,
    MLOB_ARTIST_ALBUM,
    MLOB_GENRE,
    MLOB_RATING,

};

interface IMediaLibrary {
    virtual ~IMediaLibrary() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual MLRESULT getAllArtist(IVString **ppvArtist) = 0;

    virtual MLRESULT getAllAlbum(IVString **ppvAlbum) = 0;

    virtual MLRESULT getAllGenre(IVString **ppvAlbum) = 0;

    virtual MLRESULT getAllYear(IVInt **ppvYear) = 0;

    virtual MLRESULT getAlbumOfArtist(cstr_t szArtist, IVString **ppvAlbum) = 0;

    virtual uint32_t getMediaCount() = 0;

    virtual MLRESULT getMediaByUrl(cstr_t szUrl, IMedia **pMedia) = 0;

    virtual MLRESULT add(cstr_t szMediaUrl, IMedia **ppMedia) = 0;

    // add media to media library fast, media info will not be reloaded from media file immediately.
    virtual MLRESULT addFast(cstr_t szMediaUrl, cstr_t szArtist, cstr_t szTitle, IMedia **ppMedia) = 0;

    virtual MLRESULT updateMediaInfo(IMedia *pMedia) = 0;

    // removes the specified item from the media library
    virtual MLRESULT remove(IMedia **pMedia, bool bDeleteFile) = 0;

    // If the media file was removed temporarily, set this flag on.
    virtual MLRESULT setDeleted(IMedia **pMedia) = 0;

    // nTopN = -1 : query all
    virtual MLRESULT getAll(IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

    virtual MLRESULT getByArtist(cstr_t szArtist, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

    virtual MLRESULT getByAlbum(cstr_t szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

    virtual MLRESULT getByAlbum(cstr_t szArtist, cstr_t szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

    virtual MLRESULT getByTitle(cstr_t szTitle, IPlaylist **ppPlaylist) = 0;

    virtual MLRESULT getByGenre(cstr_t szGenre, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

    virtual MLRESULT getByYear(int nYear, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

    virtual MLRESULT getByRating(int nRating, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN) = 0;

    virtual MLRESULT getRecentPlayed(uint32_t nCount, IPlaylist **ppPlaylist) = 0;

    virtual MLRESULT getRecentPlayed(int nDayAgoBegin, int nDayAgoEnd, IPlaylist **ppPlaylist) = 0;

    virtual MLRESULT getTopPlayed(uint32_t nCount, IPlaylist **ppPlaylist) = 0;

    virtual MLRESULT getTopRating(uint32_t nCount, IPlaylist **ppPlaylist) = 0;

    virtual MLRESULT getRecentAdded(uint32_t nCount, IPlaylist **ppPlaylist) = 0;

    virtual MLRESULT getRandom(uint32_t nCount, IPlaylist **ppPlaylist) = 0;

    virtual MLRESULT getRandomByTime(uint32_t nDurationInMin, IPlaylist **ppPlaylist) = 0;

    // 0-5, 0 for unrate.
    virtual MLRESULT rate(IMedia *pMedia, uint32_t nRating) = 0;

    virtual MLRESULT updatePlayedTime(IMedia *pMedia) = 0;
    virtual MLRESULT markPlayFinished(IMedia *pMedia) = 0;
    virtual MLRESULT markPlaySkipped(IMedia *pMedia) = 0;

};

interface IMPEvent {
    virtual ~IMPEvent() { }

    enum MP_SETTING_TYPE {
        MPS_SHUFFLE,                     // bool
        MPS_LOOP,                        // bool
        MPS_MUTE,                        // bool
        MPS_VOLUME,                      // int
        MPS_BALANCE,                     // int
    };

    enum MP_PLAYLIST_CHANGE_ACTION {
        PCA_FULL_UPDATE,
        PCA_INSERT,
        PCA_MOVE,
        PCA_REMOVE,
        PCA_CLEAR,
    };

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual void onStateChanged(PLAYER_STATE state) = 0;
    virtual void onSeek() = 0;
    virtual void onCurrentMediaChanged() = 0;
    // nIndex the new/cur item which was modified. nIndexOld, is used for move.
    virtual void onCurrentPlaylistChanged(MP_PLAYLIST_CHANGE_ACTION action, int nIndex, int nIndexOld) = 0;

    virtual void onSettingChanged(MP_SETTING_TYPE settingType, int value) = 0;
    virtual void onEQSettingChanged(const EQualizer *eq) = 0;

    virtual void onPlayHaltError(MLRESULT nError) = 0;

};

interface IMPluginManager {
    virtual ~IMPluginManager() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual MLRESULT detectPlugins() = 0;

    virtual MLRESULT onInternalDecoderRegister(IMediaDecode *pDecoder) = 0;

    virtual MLRESULT newInput(cstr_t szMediaUrl, IMediaInput **ppInput) = 0;
    virtual MLRESULT newDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder) = 0;
    virtual MLRESULT newOutput(IMediaOutput **ppOutput) = 0;
    virtual MLRESULT getActiveDSP(IDSP **ppDSP) = 0;
    virtual MLRESULT getActiveVis(IVector *pvVis) = 0;

};

interface IMPlayer {
    virtual ~IMPlayer() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual MLRESULT setPluginManager(IMPluginManager *pPluginMgr) = 0;

    virtual MLRESULT queryInterface(MPInterfaceType interfaceType, void **lpInterface) = 0;

    //
    // Player control
    //
    virtual MLRESULT play() = 0;
    virtual MLRESULT pause() = 0;
    virtual MLRESULT unpause() = 0;
    virtual MLRESULT stop() = 0;
    virtual MLRESULT prev() = 0;
    virtual MLRESULT next() = 0;
    virtual MLRESULT seek(uint32_t dwPos) = 0;

    virtual MLRESULT newMedia(IMedia **ppMedia, cstr_t szUrl) = 0;
    virtual MLRESULT newPlaylist(IPlaylist **ppPlaylist) = 0;

    virtual MLRESULT getMediaLibrary(IMediaLibrary **ppMediaLib) = 0;

    virtual MLRESULT getCurrentPlaylist(IPlaylist **ppPlaylist) = 0;
    virtual MLRESULT getCurrentMedia(IMedia **ppMedia) = 0;

    virtual int getCurrentMediaInPlaylist() = 0;
    virtual MLRESULT setCurrentMediaInPlaylist(int nIndex) = 0;

    virtual MLRESULT setCurrentPlaylist(IPlaylist *pPlaylist) = 0;
    virtual MLRESULT setCurrentMedia(IMedia *pMedia) = 0;

    virtual MLRESULT setCurrentMedia(cstr_t szSourceMedia) = 0;

    //
    // Current playing Media state
    //
    virtual uint32_t getLength() = 0;
    virtual uint32_t getPos() = 0;

    virtual PLAYER_STATE getState() = 0;

    //
    // plugin interface
    //
    // write sound data, and do dsp and vis processing.
    virtual void outputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) = 0;

    //     virtual bool IsDspActive() = 0;
    //     virtual void DspProcess(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) = 0;
    //
    //     virtual void addVisData(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) = 0;

    //
    // Player settings
    //
    virtual void setShuffle(bool bShuffle) = 0;
    virtual void setLoop(MP_LOOP_MODE loopMode) = 0;
    virtual bool getShuffle() = 0;
    virtual MP_LOOP_MODE getLoop() = 0;

    virtual void setMute(bool bMute) = 0;
    virtual bool getMute() = 0;

    // 0 ~ 100
    virtual MLRESULT setVolume(int volume) = 0;
    virtual int getVolume() = 0;

    // -100 ~ 100
    virtual MLRESULT setBalance(int balance) = 0;
    virtual int getBalance() = 0;

    virtual MLRESULT setEQ(const EQualizer *eq) = 0;
    virtual MLRESULT getEQ(EQualizer *eq) = 0;

    //
    // Event tracer
    //
    virtual void registerEvent(IMPEvent *pEventHandler) = 0;
    virtual void unregisterEvent(IMPEvent *pEventHandler) = 0;

    // decoder notify to player that the decoding is ended.
    // nError:    ERR_OK, the process ended ok
    virtual void notifyEod(IMediaDecode *pDecoder, MLRESULT nError) = 0;

    // Reload Media tag info: artist, title, bitarte, bps, etc.
    virtual MLRESULT loadMediaTagInfo(IMedia *pMedia, bool bForceReload) = 0;

    // Vis:
    virtual int registerVis(IVis *pVis) = 0;
    virtual int unregisterVis(IVis *pVis) = 0;

    // Dsp:
    virtual int registerDsp(IDSP *pVis) = 0;
    virtual int unregisterDsp(IDSP *pVis) = 0;

};

#define MAXCHANNELS         2

#define VIS_N_WAVE_SAMPLE   512
#define VIS_N_SPTR_SAMPLE   512

struct VisParam {
    int                         nChannels;          // number of channels
    unsigned char               spectrumData[2][VIS_N_WAVE_SAMPLE]; //
    unsigned char               waveformData[2][VIS_N_SPTR_SAMPLE];
};

interface IVis {
    virtual ~IVis() { }
    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual MLRESULT init(IMPlayer *pPlayer) = 0;
    virtual MLRESULT quit() = 0;

    virtual int render(VisParam *visParam) = 0;
};

interface IDSP {
    virtual ~IDSP() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual MLRESULT init(IMPlayer *pPlayer) = 0;
    virtual MLRESULT quit() = 0;

    virtual void process(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) = 0;
};

interface IMediaOutput { // : IUnknown
    IMediaOutput() : m_nChannels(0), m_nSamplerate(0), m_nBitsPerSamp(0) { }
    virtual ~IMediaOutput() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual cstr_t getDescription() = 0;

    virtual MLRESULT init(IMPlayer *pPlayer) = 0;
    virtual MLRESULT quit() = 0;

    virtual MLRESULT open(int nSampleRate, int nNumChannels, int nBitsPerSamp) = 0;

    virtual MLRESULT waitForWrite() = 0;

    virtual MLRESULT write(IFBuffer *pBuf) = 0;
    virtual MLRESULT flush() = 0;

    virtual MLRESULT pause(bool bPause) = 0;
    virtual bool isPlaying() = 0;
    virtual MLRESULT stop() = 0;

    virtual bool isOpened() = 0;

    virtual uint32_t getPos() = 0;

    // volume
    virtual MLRESULT setVolume(int volume, int nBanlance) = 0;

    int getChannels() { return m_nChannels; }
    int getSamplerate() { return m_nSamplerate; }
    int getBPS() { return m_nBitsPerSamp; }
protected:
    int                         m_nChannels;
    int                         m_nSamplerate;
    int                         m_nBitsPerSamp;

};

interface IMediaInputDetector {
    virtual ~IMediaInputDetector() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    virtual MLRESULT newInput(cstr_t szSourceMedia, IMediaInput **ppInput) = 0;
    virtual MLRESULT getDescription(IString *desc);

};

interface IMediaInput {
    virtual ~IMediaInput() { }
    virtual void addRef() = 0;
    virtual void release() = 0;

    // if FAILED, return ERR_MI_OPEN_SRC, ERR_MI_NOT_FOUND
    virtual MLRESULT open(cstr_t szSourceMedia) = 0;
    virtual uint32_t read(void *lpBuffer, uint32_t dwSize) = 0;
    // nOrigin: SEEK_SET, SEEK_CUR, SEEK_END
    virtual MLRESULT seek(uint32_t dwOffset, int nOrigin) = 0;
    virtual MLRESULT getSize(uint32_t &dwSize) = 0;
    virtual uint32_t getPos() = 0;

    virtual bool isEOF() = 0;
    virtual bool isError() = 0;

    virtual void close() = 0;

    virtual cstr_t getSource() = 0;

};
/*
interface IMediaDecodeSimple
{
    virtual void addRef() = 0;
    virtual void release() = 0;

    //
    // individual methods
    //

    virtual cstr_t getDescription() = 0;
    virtual cstr_t getFileExtentions() = 0;
    virtual MLRESULT getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia) = 0;

    //
    // decode media file related methods
    //

    virtual bool isSeekable() = 0;

    //    ERR_PLAYER_INVALID_FILE:    decoder can't decode the file
    //  ERR_DECODER_INNER_ERROR:    inner error occurs at decoder
    //  ERR_DECODER_UNSUPPORTED_FEATURE:
    //  ERR_DECODER_INIT_FAILED:
    virtual MLRESULT play(IMPlayer *pPlayer, IMediaInput *pInput) = 0;

    // media length, pos related functions, unit: ms
    virtual uint32_t getLength() = 0;
    virtual MLRESULT seek(uint32_t nPos) = 0;
    virtual uint32_t getPos() = 0;

};
*/

interface IMediaDecode { // : IUnknown
    virtual ~IMediaDecode() { }

    virtual void addRef() = 0;
    virtual void release() = 0;

    //
    // individual methods
    //

    virtual cstr_t getDescription() = 0;
    // get supported file extensions. Example: .mp3|MP3 files|.mp2|MP2 files
    virtual cstr_t getFileExtentions() = 0;
    virtual MLRESULT getMediaInfo(IMPlayer *pPlayer, IMediaInput *pInput, IMedia *pMedia) = 0;

    //
    // decode media file related methods
    //

    virtual bool isSeekable() = 0;
    virtual bool isUseOutputPlug() = 0;

    //    ERR_PLAYER_INVALID_FILE:    decoder can't decode the file
    //  ERR_DECODER_INNER_ERROR:    inner error occurs at decoder
    //  ERR_DECODER_UNSUPPORTED_FEATURE:
    //  ERR_DECODER_INIT_FAILED:
    virtual MLRESULT play(IMPlayer *pPlayer, IMediaInput *pInput) = 0;
    virtual MLRESULT pause() = 0;
    virtual MLRESULT unpause() = 0;
    virtual MLRESULT stop() = 0;

    // media length, pos related functions, unit: ms
    virtual uint32_t getLength() = 0;
    virtual MLRESULT seek(uint32_t nPos) = 0;
    virtual uint32_t getPos() = 0;

    // volume
    virtual MLRESULT setVolume(int volume, int nBanlance) = 0;

};

// MLRESULT GetPlayerInstance(IMPlayer **ppPlayer);

//
// A plugin DLL may contain serveral plugins, use zikiPlayerQueryPluginIF to query all the plugin interface
// that it supported.
//
// query will start with nIndex = 0, till !ERR_OK returned.
//
// nIndex = 0, return its interface type, and description, and lpInterface.
// strDescription and lpInterface can be nullptr, and only its description or lpInterface is queried.
//
// extern "C" __declspec(dllexport) MLRESULT zikiPlayerQueryPluginIF(
//     int nIndex,
//     MPInterfaceType *pInterfaceType,
//     IString *strDescription,
//     void **lpInterface
// );
typedef MLRESULT (*ZikiPlayerQueryPluginIF_t)(
    int nIndex,
    MPInterfaceType *pInterfaceType,
    IString *strDescription,
    void **lpInterface
    );

#define SZ_FUNC_ZP_QUERY_PLUGIN_IF  "zikiPlayerQueryPluginIF"

#endif // _IMPLAYER_H_
