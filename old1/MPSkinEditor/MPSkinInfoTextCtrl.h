// MPSkinInfoCtrl.h: interface for the CMPSkinInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_)
#define AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../MPShared/SkinScrollText.h"

class CMPSkinInfoTextCtrl : public CSkinScrollText
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinScrollText)
public:
    CMPSkinInfoTextCtrl();
    virtual ~CMPSkinInfoTextCtrl();

    void onCreate();

    void onTimer(int nId);

    void draw(CRawGraph *canvas);

protected:

protected:
    int            m_nTimerIDHideInfo;

};

#endif // !defined(AFX_MPSKININFOCTRL_H__D590EF82_5CAE_4116_A45C_370C6A8FF824__INCLUDED_)
