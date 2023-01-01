#pragma once

#include <thread>
#include "IMPlayer.h"
#include "Playlist.h"
#include "MPTools.h"
#include "MediaLibrary.h"
#include "MDAgent.h"


interface IMediaInput;
interface IMediaDecode;
interface IMediaOutput;

class CMemAllocator;
class CMPlayer;

class CFBuffer : public IFBuffer {
public:
    CFBuffer(CMemAllocator *pMemAllocator, uint32_t nCapacity);
    virtual ~CFBuffer();

    virtual void addRef();
    virtual void release();

    virtual char *data();
    virtual uint32_t size();
    virtual uint32_t capacity();
    virtual void resize(uint32_t nSize);
    virtual MLRESULT reserve(uint32_t nCapacity);

protected:
    char                        *m_buf;
    uint32_t                    m_nSize, m_nCapacity;
    CMemAllocator               *m_pMemAllocator;

    std::atomic<long>           m_nReference;

};

class CMemAllocator : public IMemAllocator {
OBJ_REFERENCE_DECL
public:
    CMemAllocator();
    virtual ~CMemAllocator();

    void quit();

    virtual IFBuffer *allocFBuffer(uint32_t nCapacity);
    virtual IString *allocStr();

    virtual void onRelease(IFBuffer *pBuf);

protected:
    typedef list<IFBuffer *>        LIST_BUF;
    LIST_BUF                    m_listFree;
    std::recursive_mutex        m_mutex;

};

class CMOAgent : public IMediaOutput {
OBJ_REFERENCE_DECL

public:
    CMOAgent();
    ~CMOAgent();

    virtual cstr_t getDescription();

    virtual MLRESULT init(IMPlayer *pPlayer);
    virtual MLRESULT quit();

    virtual MLRESULT open(int nSampleRate, int nNumChannels, int nBitsPerSamp);

    virtual MLRESULT waitForWrite();

    virtual MLRESULT write(IFBuffer *pBuf);
    virtual MLRESULT flush();

    virtual MLRESULT pause(bool bPause);
    virtual bool isPlaying();
    virtual MLRESULT stop();

    virtual bool isOpened();

    virtual uint32_t getPos();

    // volume
    virtual MLRESULT setVolume(int volume, int nBanlance);

public:
    CMPAutoPtr<IMediaOutput>    m_pMediaOutput;

};

class CMPluginManagerAgent : public IMPluginManager {
public:
    CMPluginManagerAgent() { }
    virtual ~CMPluginManagerAgent() { }

    virtual void addRef() { }
    virtual void release() { }

    virtual MLRESULT detectPlugins();

    virtual MLRESULT onInternalDecoderRegister(IMediaDecode *pDecoder) { if (m_pluginMgr) return m_pluginMgr->onInternalDecoderRegister(pDecoder); else return ERR_OK; }

    virtual MLRESULT newInput(cstr_t szMediaUrl, IMediaInput **ppInput);
    virtual MLRESULT newDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder);
    virtual MLRESULT newOutput(IMediaOutput **ppOutput);

    virtual MLRESULT getActiveDSP(IDSP **ppDSP);
    virtual MLRESULT getActiveVis(IVector *pvVis);

public:
    CMPAutoPtr<IMPluginManager> m_pluginMgr;

};


class CMPlayer : public IMPlayer {
    OBJ_REFERENCE_DECL
public:
    CMPlayer();
    virtual ~CMPlayer();
    // IMPlayer

    virtual MLRESULT setPluginManager(IMPluginManager *pPluginMgr);

    virtual MLRESULT queryInterface(MPInterfaceType interfaceType, void **lpInterface);

    //
    // Player control
    //
    virtual MLRESULT play();
    virtual MLRESULT pause();
    virtual MLRESULT unpause();
    virtual MLRESULT stop();
    virtual MLRESULT prev();
    virtual MLRESULT next();
    virtual MLRESULT seek(uint32_t dwPos);

    virtual MLRESULT newMedia(IMedia **ppMedia, cstr_t szUrl);
    virtual MLRESULT newPlaylist(IPlaylist **ppPlaylist);

    virtual MLRESULT getMediaLibrary(IMediaLibrary **ppMediaLib);

    virtual MLRESULT getCurrentPlaylist(IPlaylist **ppPlaylist);
    virtual MLRESULT getCurrentMedia(IMedia **ppMedia);

    virtual int getCurrentMediaInPlaylist();
    virtual MLRESULT setCurrentMediaInPlaylist(int nIndex);

    virtual MLRESULT setCurrentPlaylist(IPlaylist *pPlaylist);
    virtual MLRESULT setCurrentMedia(IMedia *pMedia);

    virtual MLRESULT setCurrentMedia(cstr_t szSourceMedia);

    //
    // Current playing Media state
    //
    virtual uint32_t getLength();
    virtual uint32_t getPos();

    virtual PLAYER_STATE getState();

    //
    // plugin interface
    //
    virtual void outputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

    //
    // Player settings
    //
    virtual void setShuffle(bool bShuffle);
    virtual void setLoop(MP_LOOP_MODE loopMode);
    virtual bool getShuffle();
    virtual MP_LOOP_MODE getLoop();

    virtual void setMute(bool bMute);
    virtual bool getMute();

    // 0 ~ 100
    virtual MLRESULT setVolume(int volume);
    virtual int getVolume();

    // -100 ~ 100
    virtual MLRESULT setBalance(int balance);
    virtual int getBalance();

    virtual MLRESULT setEQ(const EQualizer *eq);
    virtual MLRESULT getEQ(EQualizer *eq);

    virtual bool isAutoAddToMediaLib() { return m_bAutoAddToMediaLib; }

    //
    // Event tracer
    //
    virtual void registerEvent(IMPEvent *pEventHandler);
    virtual void unregisterEvent(IMPEvent *pEventHandler);

    // decoder notify to player that the decoding is ended.
    // nError:    ERR_OK, the process ended ok
    virtual void notifyEod(IMediaDecode *pDecoder, MLRESULT nError);

    // Reload Media tag info: artist, title, bitarte, bps, etc.
    virtual MLRESULT loadMediaTagInfo(IMedia *pMedia, bool bForceReload);

    // Vis:
    virtual MLRESULT registerVis(IVis *pVis);
    virtual MLRESULT unregisterVis(IVis *pVis);

    // Dsp:
    virtual MLRESULT registerDsp(IDSP *pVis);
    virtual MLRESULT unregisterDsp(IDSP *pVis);

protected:
    virtual MLRESULT makeOutputReadyForDecode();

    virtual MLRESULT doStop();
    virtual MLRESULT doNext(bool bLoop = false);

    virtual void currentMediaChanged();
    void notifyPlayStateChanged();
    void notifyCurrentPlaylistChanged(IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, int nIndex, int nIndexOld);
    void notifySettingsChanged(IMPEvent::MP_SETTING_TYPE settingType, int value);
    void notifyEQSettingsChanged(const EQualizer *peq);
    void notifySeek();

    void generateShuffleMediaQueue();

protected:
    // dsp plugin interface

public:
    static MLRESULT getInstance(IMPlayer **ppPlayer);
    static MLRESULT quitInstance(IMPlayer **ppPlayer);

    void notifyPlaylistChanged(CPlaylist *playlist, IMPEvent::MP_PLAYLIST_CHANGE_ACTION action, int nIndex, int nIndexOld);

protected:
    friend class CMDAgent;
    friend class CMDWmpCore;
    friend class CMDAVPlayer;

    bool                        m_bAutoPlayNext;
    PLAYER_STATE                m_state;
    static CMPlayer             *m_spPlayer;

    CMPluginManagerAgent        m_pluginMgrAgent;

    std::recursive_mutex        m_mutexDSP;
    CMPAutoPtr<IDSP>            m_pDsp;
    CMPAutoPtr<CMDAgent>        m_pMDAgent;
    CMPAutoPtr<CMOAgent>        m_pMOAgent;

    CMPAutoPtr<CMemAllocator>   m_pMemAllocator;
    CMPAutoPtr<CMediaLibrary>   m_pMediaLib;

    vector<int>                 m_vShuffleMedia;
    int                         m_nCurrentShuffleMedia;
    bool                        m_bShuffle;
    bool                        m_bMute;
    MP_LOOP_MODE                m_loopMode;
    int                         m_volume;
    int                         m_balance;

    EQualizer                   m_Equalizer;

    CPlaylist                   *m_currentPlaylist;
    IMedia                      *m_pCurrentMedia;
    bool                        m_bCurMediaPlayed;
    int                         m_nCurrentMedia;

    //
    // CMDAgent            *m_pMDAgentPreload;
    // IMedia                *m_pPreloadMedia;
    // long                m_nPreloadMedia;

    list<IMPEvent               *>    m_listEventHandler;
    std::recursive_mutex        m_mutexEventHandler;
    std::recursive_mutex        m_mutexDataAccess;

    bool                        m_bAutoAddToMediaLib;

protected:
    //
    // Vis
    //
    struct VisDataPre {
        IFBuffer                    *pBuf;
        int                         nBps;
        int                         nChannels;
        int                         nSampleRate;
        int                         nPlayingPos;
    };
    typedef list<VisDataPre>        QueueVisDataPre;
    QueueVisDataPre             m_queVisDataPre;
    int                         m_nBufferedVisMs;
    int                         m_nMODelay;
    std::recursive_mutex        m_mutexVisdataAccess;
    std::thread                 *m_threadVis;
    bool                        m_bQuitVis;
    typedef vector<IVis*>            V_VIS;
    V_VIS                       m_vVis;

    MLRESULT startVis();
    MLRESULT stopVis();
    void calcVisParam(VisParam &param);
    static void threadVisProc(void *lpData);
    void threadVis();

    virtual bool isVisActive();
    virtual void addVisData(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

};
