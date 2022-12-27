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

    void onCreate() override;

    virtual void onEvent(const IEvent *pEvent) override;

    bool onMouseMove(CPoint point) override;

    virtual bool onLButtonUp(uint32_t nFlags, CPoint point) override;
    virtual bool onLButtonDown(uint32_t nFlags, CPoint point) override;

    void onTimer(int nId) override;

    void draw(CRawGraph *canvas) override;

protected:
    void updateShowDefaultInfo();

protected:
    int            m_nTimerIDHideInfo;
    string        m_strCmd;
    Cursor        m_Cursor;

};

#endif // !defined(AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_)
