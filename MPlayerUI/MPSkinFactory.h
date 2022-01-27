// MPSkinFactory.h: interface for the CMPSkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINFACTORY_H__0C65D114_F3BF_47A3_A647_37E9AB35B4ED__INCLUDED_)
#define AFX_MPSKINFACTORY_H__0C65D114_F3BF_47A3_A647_37E9AB35B4ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Skin/SkinFactory.h"


#ifdef _MPLAYER
#define SZ_DEFSKIN        "ZikiPlayer"
#else
#define SZ_DEFSKIN        "MiniLyrics"
#endif

class CSkinMenu;

class CMPSkinFactory : public CSkinFactory  
{
public:
    CMPSkinFactory(CSkinApp *pApp, UIObjectIDDefinition uidDefinition[]);
    virtual ~CMPSkinFactory();

    virtual int init();

    virtual CSkinWnd *newSkinWnd(cstr_t szSkinWndName, bool bMainWnd);

    void topmostAll(bool bTopmost);
    void minizeAll(bool bSilently = false);
    void restoreAll();

    virtual int getIDByNameEx(cstr_t szId, string &strToolTip);
    virtual void getTooltip(int nId, string &strToolTip);

    int openSimpliestSkin();

    virtual void adjustHue(float hue, float saturation = 0.5, float luminance = 0.5);

#ifndef _MPLAYER
    //
    // Auto close to player window, and track to move with it.
    //

    void beforeTrackMoveWith(Window *pWndChain[], int nCount, Window *pWndToTrack);
    void trackMoveWith(Window *pWnd, int x, int y);

    void addWndCloseto(Window *pWnd, cstr_t szWndName, cstr_t szClass);

    // ISkinWndDragHost
    virtual void getWndDragAutoCloseTo(vector<Window *> &vWnd);

    typedef vector<WndDrag::WndCloseTo>        V_WNDCLOSETO;

    WndDrag            m_WndCloseToPlayers;
#endif
    void allUpdateTransparent();
    void setClickThrough(bool bClickThrough);
    bool getClickThrough() const { return m_bClickThrough; }

protected:
    bool                m_bClickThrough;

protected:
    virtual CSkinMenu *loadPresetMenu(CSkinWnd *pWnd, cstr_t szMenu);

    SXNode *getMenuOfMenus(SXNode *pNodeMenus, cstr_t szMenuName);

};

#endif // !defined(AFX_MPSKINFACTORY_H__0C65D114_F3BF_47A3_A647_37E9AB35B4ED__INCLUDED_)
