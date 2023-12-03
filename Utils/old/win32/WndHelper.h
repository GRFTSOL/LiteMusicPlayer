#pragma once

#ifndef Utils_old_win32_WndHelper_h
#define Utils_old_win32_WndHelper_h

// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <vector>


using namespace std;

class CWndHelper {
    struct ChildWndProp {
        HWND                        hWnd;
        HWND                        hWndRelation;
        bool                        bAlignRight;
        bool                        bAlignBottom;
        int                         nMarginX, nMarginY;

        bool                        bSizeXFix, bSizeYFix;
        int                         nX, nY;
        int                         nCx, nCy;
    };
    struct ChildAlignCenter {
        HWND                        hWnd;
        bool                        bVert;
        int                         nOffsetToCenter;
    };

public:
    CWndHelper();
    virtual ~CWndHelper();

    void init(HWND    hWndParent) { m_hWndParent = hWndParent; }

    void add(HWND hWnd, bool bAlignRight, bool bAlignBottom, bool bSizeXFix, bool bSizeYFix, HWND hWndRelation = nullptr);
    void addAlignCenter(HWND hWnd, bool bAlignVert);

    void add(int nChildID, bool bAlignRight, bool bAlignBottom, bool bSizeXFix, bool bSizeYFix, HWND hWndRelation = nullptr);
    void addAlignCenter(int nChildID, bool bAlignVert);
    void addAlignRight(int nChildID, bool bSizeXFixed);
    void addAlignBottom(int nChildID, bool bSizeYFixed);
    void addAlignRightBottom(int nChildID, bool bSizeXFixed, bool bSizeYFixed);

    void onResize();

protected:
    HWND                        m_hWndParent;

    vector<ChildWndProp>        m_vChildWnd;
    vector<ChildAlignCenter>    m_vAlignCenterWnds;

};

#endif // !defined(Utils_old_win32_WndHelper_h)
