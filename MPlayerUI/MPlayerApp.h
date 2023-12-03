#pragma once

#include "MPlayerAppBase.h"


class CMPlayerApp : public CMPlayerAppBase {
public:
    CMPlayerApp();
    virtual ~CMPlayerApp();

    bool isAnotherInstanceRunning();
    bool setRunningFlag();

    virtual bool init();
    virtual void quit();

    bool isRunning() { return m_hMutexRuning != nullptr; }

protected:
#ifdef _WIN32
    HANDLE                      m_hMutexRuning;
#else
    bool                        m_bRunning;
#endif

};
