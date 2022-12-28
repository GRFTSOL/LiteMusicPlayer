#include "../MPlayerApp.h"
#include "PlayerSmoothTimer.h"


CPlayerSmoothTimer::CPlayerSmoothTimer() {
    m_dwTimeBeg = 0;
    m_nTimePosOld = 0;
    m_bQuit = true;
    m_nTimeSpan = 12;
}

CPlayerSmoothTimer::~CPlayerSmoothTimer() {
}

bool CPlayerSmoothTimer::start() {
    assert(m_bQuit);
    m_bQuit = false;

    return true;
}

void CPlayerSmoothTimer::stop() {
    m_bQuit = true;
}

void CPlayerSmoothTimer::adjustTimer(int nTimePos) {
}
