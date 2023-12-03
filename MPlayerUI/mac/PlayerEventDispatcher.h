#pragma once


class CPlayerEventDispatcher {
public:
    CPlayerEventDispatcher();
    ~CPlayerEventDispatcher() { }

    void init() {
    }

    void quit() {
    }

    void setLyrDrawUpdateFast(bool bFast) {
        if (bFast) {
            m_nTimeOutUpdateLyr = 20;
        } else {
            m_nTimeOutUpdateLyr = 40;
        }

        startLyrDrawUpdate();
    }

    void startLyrDrawUpdate();

    void stopLyrDrawUpdate();

protected:
    int                         m_nTimeOutUpdateLyr;
    void                        *m_pInternal;

};

extern CPlayerEventDispatcher g_playerEventDispatcher;
