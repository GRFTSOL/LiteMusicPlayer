// SkinWndDrag.h: interface for the CSkinWndDrag class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINWNDDRAG_H__A02E79B7_9596_4BAB_AAAE_6173A4201D75__INCLUDED_)
#define AFX_SKINWNDDRAG_H__A02E79B7_9596_4BAB_AAAE_6173A4201D75__INCLUDED_

#pragma once

#include "../Window/WindowLib.h"

class ISkinWndDragHost
{
public:
    virtual void getWndDragAutoCloseTo(vector<Window *> &vWnd) = 0;
    virtual void getWndDragTrackMove(vector<Window *> &vWnd) = 0;

};

class CSkinWndDrag : public WndDrag
{
public:
    CSkinWndDrag();
    virtual ~CSkinWndDrag();

    void init(Window *pWnd, ISkinWndDragHost *pSkinWndDragHost);

    void onDrag(uint32_t fwKeys, CPoint pt);

    virtual bool autoCloseToWindows(int &nOffx, int &nOffy, bool bMoveWindow);

protected:
    void setWindowPosSafely(int xOld, int yOld, int nOffsetx, int nOffsety);

protected:
    ISkinWndDragHost    *m_pSkinWndDragHost;
    vector<Window *>    m_vStickedWnds;

};

class CSkinWndResizer : public WndResizer
{
public:
    CSkinWndResizer();
    virtual ~CSkinWndResizer();

    void init(Window *pWnd, ISkinWndDragHost *pSkinWndDragHost);

    virtual void autoCloseToWindows(int &nOffx, int &nOffy);

protected:
    ISkinWndDragHost    *m_pSkinWndDragHost;

};

#endif // !defined(AFX_SKINWNDDRAG_H__A02E79B7_9596_4BAB_AAAE_6173A4201D75__INCLUDED_)
