#pragma once

#ifndef MPlayerEngine_VISDemo_h
#define MPlayerEngine_VISDemo_h


#include "IMPlayer.h"


class CVISDemo : public IVisualizer {
OBJ_REFERENCE_DECL

public:
    CVISDemo();
    virtual ~CVISDemo();

    virtual ResultCode init(IMPlayer *pPlayer);
    virtual ResultCode quit();

    virtual int render(VisParam *visParam);
    int render2(VisParam *visParam);
    int render3(VisParam *visParam);
    int render4(VisParam *visParam);

protected:
    bool createWnd();

protected:
    IMPlayer                    *m_pPlayer;

    HWND                        m_hWnd;

};

#endif // !defined(MPlayerEngine_VISDemo_h)
