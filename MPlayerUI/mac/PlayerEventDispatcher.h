
#pragma once


///////////////////////////////////////////////////////////////////////////////
// create window to get playback pos of player timely.

class CPlayerEventDispatcher
{
public:
    CPlayerEventDispatcher();
    ~CPlayerEventDispatcher() { }
    
    void init()
    {
    }
    
    void quit()
    {
    }
    
    long getMasterVolume()
    {
        return 0;
    }
    
    void setMasterVolume(long volume)
    {
    }

    void setLyrDrawUpdateFast(bool bFast)
    {
        if (bFast)
            m_nTimeOutUpdateLyr = 20;
        else
            m_nTimeOutUpdateLyr = 40;
        
        startLyrDrawUpdate();
    }
    
    void startLyrDrawUpdate();
    
    void stopLyrDrawUpdate();

protected:
    int            m_nTimeOutUpdateLyr;
    void *        m_pInternal;
    
};

extern CPlayerEventDispatcher        g_playerEventDispatcher;
