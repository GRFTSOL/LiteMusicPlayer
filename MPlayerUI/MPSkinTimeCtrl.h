// MPSkinTimeCtrl.h: interface for the CMPSkinTimeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINTIMECTRL_H__962BB037_AA83_4BD0_B78E_41AE6BA87DE6__INCLUDED_)
#define AFX_MPSKINTIMECTRL_H__962BB037_AA83_4BD0_B78E_41AE6BA87DE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinPicText.h"

string formatTime(int nTimeSec);

template<class _CSkinText>
class CMPSkinTimeCtrl : public _CSkinText  
{
    UIOBJECT_CLASS_NAME_DECLARE(_CSkinText)
public:
    CMPSkinTimeCtrl()
    {
        _CSkinText::m_msgNeed = UO_MSG_WANT_LBUTTON;
        m_nTimeOld = -1;
        m_bShowElapsedTime = true;
    }

    virtual ~CMPSkinTimeCtrl()
    {
        _CSkinText::m_pSkin->unregisterTimerObject(this);
    }

    void onCreate()
    {
        _CSkinText::onCreate();

        _CSkinText::m_pSkin->registerTimerObject(this, 200);
    }

    bool onLButtonUp(uint32_t nFlags, CPoint point)
    {
        m_bShowElapsedTime = !m_bShowElapsedTime;

        m_nTimeOld = -1;
        resetTimeText();

        return true;
    }

    void onTimer(int nId)
    {
        resetTimeText();
    }

protected:

    void resetTimeText()
    {
        int        nTime = g_Player.getPlayPos() + 500;
        nTime /= 1000;
        if (!m_bShowElapsedTime)
            nTime = g_Player.getMediaLength() / 1000 - nTime;

        if (nTime != m_nTimeOld)
        {
            m_nTimeOld = nTime;

            string        strText;

            if (!m_bShowElapsedTime)
                strText = "-";

            strText += formatTime(nTime);
            _CSkinText::setText(strText.c_str());

            _CSkinText::invalidate();
        }
    }

protected:
    bool            m_bShowElapsedTime;
    int                m_nTimeOld;

};

#endif // !defined(AFX_MPSKINTIMECTRL_H__962BB037_AA83_4BD0_B78E_41AE6BA87DE6__INCLUDED_)
