// WndHelper.h: interface for the CWndHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WNDHELPER_H__45A76D39_017B_41E8_A838_312A5F92D1AF__INCLUDED_)
#define AFX_WNDHELPER_H__45A76D39_017B_41E8_A838_312A5F92D1AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// using STL
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#include <vector>
using namespace std;

class CWndHelper  
{
    struct ChildWndProp
    {
        HWND        hWnd;
        HWND        hWndRelation;
        bool        bAlignRight;
        bool        bAlignBottom;
        int            nMarginX, nMarginY;

        bool        bSizeXFix, bSizeYFix;
        int            nX, nY;
        int            nCx, nCy;
    };
    struct ChildAlignCenter
    {
        HWND        hWnd;
        bool        bVert;
        int            nOffsetToCenter;
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
    HWND        m_hWndParent;

    vector<ChildWndProp>        m_vChildWnd;
    vector<ChildAlignCenter>    m_vAlignCenterWnds;

};

#endif // !defined(AFX_WNDHELPER_H__45A76D39_017B_41E8_A838_312A5F92D1AF__INCLUDED_)
