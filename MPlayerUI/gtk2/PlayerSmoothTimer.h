// PlayerSmoothTimer.h: interface for the CPlayerSmoothTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERSMOOTHTIMER_H__B89F0D41_2E58_4D51_9610_00ADA16BC4FD__INCLUDED_)
#define AFX_PLAYERSMOOTHTIMER_H__B89F0D41_2E58_4D51_9610_00ADA16BC4FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CPlayerSmoothTimer
{
public:
    CPlayerSmoothTimer();
    ~CPlayerSmoothTimer();

    bool start();
    void stop();
    void adjustTimer(int nTimePos);

    bool isRunning() { return !m_bQuit; }

protected:
    CThread            m_thread;
    uint32_t            m_dwTimeBeg;
    int                m_nTimePosOld;
    bool            m_bQuit;
    int                m_nTimeSpan;

};

#endif // !defined(AFX_PLAYERSMOOTHTIMER_H__B89F0D41_2E58_4D51_9610_00ADA16BC4FD__INCLUDED_)
