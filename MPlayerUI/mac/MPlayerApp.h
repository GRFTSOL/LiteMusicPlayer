#pragma once

#include "../MPlayerAppBase.h"

class CMPlayerApp : public CMPlayerAppBase
{
public:
    CMPlayerApp();
    virtual ~CMPlayerApp();

    bool isAnotherInstanceRunning();
    bool setRunningFlag();

    virtual bool init();
    virtual void quit();

    bool isRunning() { return m_bRunning; }
    
    void restartToAppMode(AppMode appMode);

protected:
    AppMode                    m_appModeQuitToStart;
    bool                    m_bRunning;

};
