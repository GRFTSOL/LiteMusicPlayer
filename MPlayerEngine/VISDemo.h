// VISDemo.h: interface for the CVISDemo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VISDEMO_H__CF191EB8_1C87_483F_B95A_16E69074F4F5__INCLUDED_)
#define AFX_VISDEMO_H__CF191EB8_1C87_483F_B95A_16E69074F4F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IMPlayer.h"

class CVISDemo : public IVis  
{
OBJ_REFERENCE_DECL

public:
    CVISDemo();
    virtual ~CVISDemo();

    virtual MLRESULT init(IMPlayer *pPlayer);
    virtual MLRESULT quit();

    virtual int render(VisParam *visParam);
    int render2(VisParam *visParam);
    int render3(VisParam *visParam);
    int render4(VisParam *visParam);

protected:
    bool createWnd();

protected:
    IMPlayer        *m_pPlayer;

    HWND            m_hWnd;

};

#endif // !defined(AFX_VISDEMO_H__CF191EB8_1C87_483F_B95A_16E69074F4F5__INCLUDED_)
