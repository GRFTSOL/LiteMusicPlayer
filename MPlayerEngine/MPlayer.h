#pragma once

#include <thread>
#include "IMPlayer.h"
#include "MPTools.h"
#include "MDAgent.h"
#include "PluginManager.h"


interface IMediaInput;
interface IMediaDecoder;
interface IMediaOutput;

class CMPlayer;

class CFBuffer : public IFBuffer {
public:
    CFBuffer(uint32_t nCapacity);
    virtual ~CFBuffer();

    virtual void addRef();
    virtual void release();

    virtual char *data();
    virtual uint32_t size();
    virtual uint32_t capacity();
    virtual void resize(uint32_t nSize);
    virtual ResultCode reserve(uint32_t nCapacity);

protected:
    char                        *m_buf;
    uint32_t                    m_nSize, m_nCapacity;

    std::atomic<long>           m_nReference;

};

using ListFBuffers = list<CFBuffer *>;

class CMOAgent : public IMediaOutput {
OBJ_REFERENCE_DECL

public:
    CMOAgent();
    ~CMOAgent();

    virtual cstr_t getDescription();

    virtual ResultCode init(IMPlayer *pPlayer);
    virtual ResultCode quit();

    virtual ResultCode open(int nSampleRate, int nNumChannels, int nBitsPerSamp);

    virtual ResultCode waitForWrite();

    virtual ResultCode write(IFBuffer *pBuf);
    virtual ResultCode flush();

    virtual ResultCode pause(bool bPause);
    virtual bool isPlaying();
    virtual ResultCode stop();

    virtual bool isOpened();

    virtual uint32_t getPos();

    // volume
    virtual ResultCode setVolume(int volume, int nBanlance);

public:
    CMPAutoPtr<IMediaOutput>    m_pMediaOutput;

};

class CMPlayer : public IMPlayer {
    OBJ_REFERENCE_DECL
public:
    CMPlayer();
    virtual ~CMPlayer();
    // IMPlayer

    virtual ResultCode queryInterface(MPInterfaceType interfaceType, void **lpInterface);

    //
    // Player control
    //
    virtual ResultCode play();
    virtual ResultCode pause();
    virtual ResultCode unpause();
    virtual ResultCode stop();
    virtual ResultCode seek(uint32_t dwPos);

    //
    // Current playing Media state
    //
    virtual uint32_t getPos();

    virtual PlayerState getState();

    //
    // plugin interface
    //
    virtual void outputWrite(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

    // 0 ~ 100
    virtual ResultCode setVolume(int volume);
    virtual int getVolume();

    // -100 ~ 100
    virtual ResultCode setBalance(int balance);
    virtual int getBalance();

    virtual ResultCode setEQ(const EQualizer *eq);
    virtual ResultCode getEQ(EQualizer *eq);

    // decoder notify to player that the decoding is ended.
    // nError:    ERR_OK, the process ended ok
    virtual void notifyEod(IMediaDecoder *pDecoder, ResultCode nError);

    // Vis:
    virtual ResultCode registerVis(IVisualizer *pVis);
    virtual ResultCode unregisterVis(IVisualizer *pVis);

    // Dsp:
    virtual ResultCode registerDsp(IDSP *pVis);
    virtual ResultCode unregisterDsp(IDSP *pVis);

protected:
    virtual ResultCode makeOutputReadyForDecode();

protected:
    // dsp plugin interface

public:
    static ResultCode getInstance(IMPlayer **ppPlayer);
    static ResultCode quitInstance(IMPlayer **ppPlayer);

protected:
    friend class CMDAgent;
    friend class CMDWmpCore;
    friend class CoreAVPlayer;

    PlayerState                 m_state;
    static CMPlayer             *m_spPlayer;

    PluginManager               m_pluginMgr;

    std::recursive_mutex        m_mutexDSP;
    CMPAutoPtr<IDSP>            m_pDsp;
    CMPAutoPtr<CMDAgent>        m_pMDAgent;
    CMPAutoPtr<CMOAgent>        m_pMOAgent;

    ListFBuffers                m_fbuffers;

    EQualizer                   m_Equalizer;

    std::recursive_mutex        m_mutexDataAccess;

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
    typedef vector<IVisualizer*>            V_VIS;
    V_VIS                       m_vVis;

    ResultCode startVis();
    ResultCode stopVis();
    void calcVisParam(VisParam &param);
    static void threadVisProc(void *lpData);
    void threadVis();

    virtual bool isVisActive();
    virtual void addVisData(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate);

};
