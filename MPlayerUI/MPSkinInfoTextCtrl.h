// MPSkinInfoCtrl.h: interface for the CMPSkinInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_)
#define AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinScrollText.h"

class CMPSkinInfoTextCtrl : public CSkinScrollText, public IEventHandler
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinScrollText)
public:
    CMPSkinInfoTextCtrl();
    virtual ~CMPSkinInfoTextCtrl();

    void onCreate();

    virtual void onEvent(const IEvent *pEvent);

    bool onMouseMove(CPoint point);

    virtual bool onLButtonUp(uint32_t nFlags, CPoint point);
    virtual bool onLButtonDown(uint32_t nFlags, CPoint point);

    void onTimer(int nId);

    void draw(CRawGraph *canvas);

protected:
    void updateShowDefaultInfo();

protected:
    int            m_nTimerIDHideInfo;
    string        m_strCmd;
    Cursor        m_Cursor;

};

#endif // !defined(AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_)
