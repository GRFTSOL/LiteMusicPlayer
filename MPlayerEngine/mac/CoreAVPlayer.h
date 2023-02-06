#pragma once

#include "../IPlayerCore.hpp"


typedef void * _MDAVPlayerInternal;

class CoreAVPlayer: public IPlayerCore {
public:
    CoreAVPlayer(void);
    ~CoreAVPlayer(void);

    // 实现的说明
    virtual const char *getDescription() override;

    // 支持的文件扩展名
    virtual const char *getFileExtentions() override;

    // 获取媒体的标签
    virtual bool getMediaInfo(const char *mediaUrl, IMediaInfo *pMedia) override;

    //
    // Player control
    //
    virtual bool play(const char *mediaUrl, IMediaInfo *mediaTagsOut) override;
    virtual bool pause() override;
    virtual bool unpause() override;
    virtual bool stop() override;

    virtual bool isSeekable() override;
    virtual bool seek(int pos) override;

    //
    // Current playing Media state
    //
    virtual uint32_t getDuration() override;
    virtual uint32_t getPos() override;
    virtual PlayerState getState() override;

    //
    // Player settings
    //
    // 0 ~ 100
    virtual bool setVolume(int volume) override;
    virtual int getVolume() override;

    // -100 ~ 100
    virtual bool setBalance(int balance) override;
    virtual int getBalance() override;

    virtual bool setEQ(const EQualizer *eq) override;
    virtual bool getEQ(EQualizer *eq) override;

    void notifyEndOfPlaying();

protected:
    bool isOK();

    PlayerState                 m_state;
    _MDAVPlayerInternal         m_player;

};
