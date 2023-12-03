#pragma once

#ifndef MPlayerUI_gtk2_PlayerSmoothTimer_h
#define MPlayerUI_gtk2_PlayerSmoothTimer_h



class CPlayerSmoothTimer {
public:
    CPlayerSmoothTimer();
    ~CPlayerSmoothTimer();

    bool start();
    void stop();
    void adjustTimer(int nTimePos);

    bool isRunning() { return !m_bQuit; }

protected:
    CThread                     m_thread;
    uint32_t                    m_dwTimeBeg;
    int                         m_nTimePosOld;
    bool                        m_bQuit;
    int                         m_nTimeSpan;

};

#endif // !defined(MPlayerUI_gtk2_PlayerSmoothTimer_h)
