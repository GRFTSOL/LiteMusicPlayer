/********************************************************************
    Created  :    2002/01/04    21:43
    FileName :    WndDrag.h
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#pragma once

#ifndef Window_WndDrag_h
#define Window_WndDrag_h

#include <vector>


class Window;

class WndDrag {
public:
    struct WndCloseTo {
        Window                      *pWnd;
        string                      strClass;
        string                      strWndName;
    };

    std::vector<WndCloseTo>     m_vWndCloseTo;

public:
    WndDrag();
    virtual ~WndDrag();

    void init(Window *pWnd);

    bool isDragging() { return m_bDragWnd; }
    void onDrag(uint32_t fwKeys, CPoint pt);

    void enableDrag(bool bEnable) { m_bEnableDrag = bEnable; }
    void enableAutoCloseto(bool bEnable) { m_bDragAutoCloseto = bEnable; }

    void addWndCloseto(Window *pWnd, cstr_t szWndClass, cstr_t szWndName);

    void beforeTrackMoveWith(Window *pWndChain[], int nCount, Window *pWndToTrack);
    void trackMoveWith(Window *pWnd, int x, int y);

protected:
    virtual void setWindowPosSafely(int xOld, int yOld, int nOffsetx, int nOffsety);
    virtual bool autoCloseToWindows(int &nOffx, int &nOffy, bool bMoveWindow = true);

protected:
    Window                      *m_pWnd;

    bool                        m_bDragWnd;         // dragging window?

    CPoint                      m_ptDragOld;        // drag begin pos

    bool                        m_bDragAutoCloseto;

    bool                        m_bSticked;
    Window                      *m_pWndToTrack;
    CPoint                      m_ptWndTrack;

    bool                        m_bEnableDrag;

    // vector<string>        *m_pvWndClassCloseTo;
    // char *          *m_arrszWndClassCloseTo;
};

#endif // !defined(Window_WndDrag_h)
