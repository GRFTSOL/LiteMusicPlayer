#include "MPlayerApp.h"
#include "PlayerSmoothTimer.h"


#define COMPARE_PRECISION   300
CPlayerSmoothTimer::CPlayerSmoothTimer() {
    m_dwTimeBeg = 0;
    m_nTimePosOld = 0;
    m_bQuit = true;
    m_nTimeSpan = 12;
}

CPlayerSmoothTimer::~CPlayerSmoothTimer() {
}

bool CPlayerSmoothTimer::start() {
    // m_nTimeSpan = nTimerSpan;
    m_nTimeSpan = 12; // 1000 ms / 85 fps
    assert(m_bQuit);
    m_bQuit = false;
    m_thread.create(threadTimer, this);

    return true;
}

void CPlayerSmoothTimer::stop() {
    m_bQuit = true;
}

void CPlayerSmoothTimer::adjustTimer(int nTimePos) {
    int n = getTickCount() - m_dwTimeBeg - nTimePos;

    if (n >= COMPARE_PRECISION || n <= -COMPARE_PRECISION) {
        if (m_nTimePosOld != nTimePos) {
            m_dwTimeBeg = getTickCount() - nTimePos;

            g_LyricData.SetPlayElapsedTime(nTimePos);

            CMPlayerAppBase::getEventsDispatcher()->dispatchUnsyncEvent(ET_LYRICS_DRAW_UPDATE);
        }
    }
    m_nTimePosOld = nTimePos;
}

void CPlayerSmoothTimer::threadTimer(void *lpParam) {
    CPlayerSmoothTimer *pThis;
    uint32_t dwTimeNow;
    int n;

    pThis = (CPlayerSmoothTimer*)lpParam;

    while (!pThis->m_bQuit) {
        sleep(pThis->m_nTimeSpan);

        dwTimeNow = getTickCount();
        // g_wndLyricShow.OnPlayTimeChanged(dwTimeNow - dwTimeBeg);
        n = dwTimeNow - pThis->m_dwTimeBeg;
        if ((int)(n - pThis->m_nTimePosOld) >= COMPARE_PRECISION) {
            continue;
        }

        g_LyricData.SetPlayElapsedTime(n);

        CMPlayerAppBase::getEventsDispatcher()->dispatchUnsyncEvent(ET_LYRICS_DRAW_UPDATE);
    }
}
