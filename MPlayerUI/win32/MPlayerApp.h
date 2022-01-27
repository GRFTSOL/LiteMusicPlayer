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

    bool isRunning() { return m_hMutexRuning != nullptr; }

#ifndef _MPLAYER
    //
    // Embedded skin mode support.
    //
    bool isSupportEmbedded();
    bool isEmbeddedImmovable();
    bool isEmbeddedMode() { return !m_bMPSkinMode; }

    int onSwitchToEmbeddedSkinMode();
    void endEmbeddedSkin();

    int loadGeneralEmbeddedSkin(bool bLoadSkinColorTheme = true);
    int loadWinamp2Skin(bool bLoadSkinColorTheme = true);

    void restartToAppMode(AppMode appMode);

    void setReloadEmbeddedTheme() { m_bReloadEmbeddedTheme = true; }
protected:
    AppMode                    m_appModeQuitToStart;
    bool                    m_bReloadEmbeddedTheme;

#endif

protected:
    HANDLE                    m_hMutexRuning;

};
