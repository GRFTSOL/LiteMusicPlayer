// MOSoundCard.h: interface for the CMOSoundCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_)
#define AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmsystem.h>

class CMOSoundCard : public IMediaOutput  
{
OBJ_REFERENCE_DECL
public:
    enum FADE_MODE
    {
        FADE_NONE,
        FADE_IN,
        FADE_OUT,
    };

    CMOSoundCard();
    virtual ~CMOSoundCard();

    cstr_t getDescription();

    virtual MLRESULT init(IMPlayer *pPlayer) { return ERR_OK; }
    virtual MLRESULT quit() { return ERR_OK; }

    virtual MLRESULT open(int nSampleRate, int nNumChannels, int nBitsPerSamp);

    virtual MLRESULT waitForWrite();

    virtual MLRESULT write(IFBuffer *pBuf);
    virtual MLRESULT flush();

    virtual MLRESULT pause(bool bPause);
    virtual bool isPlaying();
    virtual MLRESULT stop();

    virtual bool isOpened();

    // volume
    MLRESULT setVolume(int nVolume, int nBanlance);

    virtual uint32_t getPos();

protected:
    MLRESULT doOpen();
    MLRESULT doStop();

    void fadeInSoundData(char *buf, int nLen);
    void fadeOutSoundData(char *buf, int nLen);

    WAVEHDR* NewWaveHdr();

public:
    static void CALLBACK MCICallBack(HWAVEOUT hwo, uint32_t msg, uint32_t dwInstance,
                uint32_t dwParam1, uint32_t dwParam2);

private:
    Event            m_eventCanWrite;
    uint32_t            m_dwTotolBytesOffset;        // 输入到MOSoundCard中的字节偏移总量

    WAVEFORMATEX    m_wfex;
    HWAVEOUT        m_hwo;

    int                m_iBytesPerSample;
    uint32_t            m_dwDataSize;

    typedef    list<WAVEHDR *>        LIST_WAVHDR;

    std::mutex            m_mutex;
    list<WAVEHDR *>    m_listFree, m_listPrepared, m_listPausedHdr;
    int                m_nBuffered;
    int                m_nBufferedMax;

    FADE_MODE        m_fadeMode;
    long            m_nFadeDoneSamples;
    long            m_nFadeMaxSamples;

    bool            m_bWrittingPausedData;
    
    WAVEHDR            *m_pFadeOutEndHdr;
    
};

#endif // !defined(AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_)

