// MOSoundCard.h: interface for the CMOSoundCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_)
#define AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_

#pragma once

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

private:
    Event            m_eventCanWrite;
    uint32_t            m_dwTotolBytesOffset;        // 输入到MOSoundCard中的字节偏移总量

    int                m_iBytesPerSample;
    uint32_t            m_dwDataSize;

    std::mutex            m_mutex;
    int                m_nBuffered;
    int                m_nBufferedMax;

    FADE_MODE        m_fadeMode;
    long            m_nFadeDoneSamples;
    long            m_nFadeMaxSamples;

    bool            m_bWrittingPausedData;
    
};

#endif // !defined(AFX_MOSOUNDCARD_H__2A3AF0D3_981C_41D0_8BBF_A6C7752E98FF__INCLUDED_)

