#pragma once

#ifndef MPlayerUI_MPSkinTimeCtrl_h
#define MPlayerUI_MPSkinTimeCtrl_h


#include "../Skin/SkinPicText.h"


string formatDuration(int nTimeSec);

template<class _CSkinText>
class CMPSkinTimeCtrl : public _CSkinText {
    UIOBJECT_CLASS_NAME_DECLARE(_CSkinText)
public:
    CMPSkinTimeCtrl() {
        _CSkinText::m_msgNeed = UO_MSG_WANT_LBUTTON;
        m_nTimeOld = -1;
        m_bShowElapsedTime = true;
    }

    virtual ~CMPSkinTimeCtrl() {
        _CSkinText::m_pSkin->unregisterTimerObject(this);
    }

    void onCreate() override {
        _CSkinText::onCreate();

        _CSkinText::m_pSkin->registerTimerObject(this, 200);
    }

    bool onLButtonUp(uint32_t nFlags, CPoint point) override {
        m_bShowElapsedTime = !m_bShowElapsedTime;

        m_nTimeOld = -1;
        resetTimeText();

        return true;
    }

    void onTimer(int nId) override {
        resetTimeText();
    }

protected:

    void resetTimeText() {
        int nTime = g_player.getPlayPos() + 500;
        nTime /= 1000;
        if (!m_bShowElapsedTime) {
            nTime = g_player.getMediaLength() / 1000 - nTime;
        }

        if (nTime != m_nTimeOld) {
            m_nTimeOld = nTime;

            string strText;

            if (!m_bShowElapsedTime) {
                strText = "-";
            }

            strText += formatDuration(nTime);
            _CSkinText::setText(strText.c_str());

            _CSkinText::invalidate();
        }
    }

protected:
    bool                        m_bShowElapsedTime;
    int                         m_nTimeOld;

};

#endif // !defined(MPlayerUI_MPSkinTimeCtrl_h)
