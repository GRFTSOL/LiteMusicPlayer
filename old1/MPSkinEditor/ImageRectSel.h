#if !defined(AFX_IMAGERECTSEL_H__488BFFD0_6FE6_4A24_A590_746D2237B79C__INCLUDED_)
#define AFX_IMAGERECTSEL_H__488BFFD0_6FE6_4A24_A590_746D2237B79C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageRectSel.h : header file
//

enum ImageRectSelType
{
    IRST_NONE,
    IRST_IMG,                    // SkinImage
    IRST_HORZ_STRETCH_IMG,        // HStretchImage
    IRST_VERT_STRETCH_IMG,        // VStretchImage
    IRST_CAPTION_IMG,            // CaptionImage
};

class CImageSelRectEx
{
public:
    struct Vertex
    {
        Vertex()
        {
            bCanMoveHorz = true;
            bCanMoveVert = true;
            x = y = 0;
        }
        bool    bCanMoveHorz;
        bool    bCanMoveVert;
        int        x, y;
    };
    typedef    vector<Vertex *>    V_VERTEX;

    struct Group
    {
        bool            bVert;        // true = vert, false = horz
        V_VERTEX        vVertex;
    };
    typedef    vector<Group>        V_GROUP;

    struct Line
    {
        Vertex    *pV1, *pV2;
    };
    typedef    vector<Line>        V_LINE;

public:
    CImageSelRectEx();
    virtual ~CImageSelRectEx();

    void close();

    void addVertexOrderByX(Vertex *pVer);
    void addVertexOrderByY(Vertex *pVer);
    void addLine(Line &line);
    void addLine(Vertex *pV1, Vertex *pV2);
    void addGroup(Group &group);

    void setSelAllRect(Vertex *pV1, Vertex *pV2);

    void onLButtonDown(uint32_t nFlags, CPoint point);
    void onLButtonUp(uint32_t nFlags, CPoint point);
    void onMouseMove(uint32_t nFlags, CPoint point);
    void onKeyDown(uint32_t nChar, uint32_t nRepCnt, uint32_t nFlags);

    void draw(HDC hdc, int nScale);

    void drawXor(HDC hdc);

    void offsetAllVertex(int nXOffset, int nYOffset);

    void setVertex(Vertex *pV, int x, int y);

protected:
    Group *findGroup(Vertex *pV, bool bVert);
    void offsetVertexInGroup(Group *gr, bool bVert, int nOffset);

    void offsetVertex(int nXOffset, int nYOffset);

public:
    V_VERTEX        m_vVertexOBX, m_vVertexOBY;    // OBX = order by X
    V_LINE            m_vLine;
    V_GROUP            m_vGroup;

    Group            m_SelAllRect;
    bool            m_bDragAll;

    bool            m_bMoveKeepVertexOrder;            // order by x, y

    bool            m_bLBtDown;
    CPoint            m_ptDragOld;            // drag begin pos

    // int                m_nSelGroup;
    int                m_nSelVertex;
    int                m_nScale;

public:
    HWND            m_hWnd;

};

/////////////////////////////////////////////////////////////////////////////
// CImageRectSelCtrl window

/*

  // SS = StretchStart
  // SE = StretchEnd
  v1 ---vSS----vSE-----vSS2----vSE2-- v2
  |                                   |
  |                                   |
  |                                   |
  |                                   |
  v4 ---vSSb---vSEb----vSS2b---vSE2b- v3

*/

class CImageRectSelCtrl : public CStatic
{
// Construction
public:
    CImageRectSelCtrl();

// Attributes
protected:
    // CSFImage        m_image;
    CMLImage        m_image;
    CImageSelRectEx    m_imageSelGrid;

    ImageRectSelType    m_imgRectSelType;

    int                m_nScale;

    CImageSelRectEx::Vertex        *m_pV1;
    CImageSelRectEx::Vertex        *m_pV2;
    CImageSelRectEx::Vertex        *m_pV3;
    CImageSelRectEx::Vertex        *m_pV4;
    CImageSelRectEx::Vertex        *m_pVSS;
    CImageSelRectEx::Vertex        *m_pVSE;
    CImageSelRectEx::Vertex        *m_pVSS2;
    CImageSelRectEx::Vertex        *m_pVSE2;
    CImageSelRectEx::Vertex        *m_pVSSb;
    CImageSelRectEx::Vertex        *m_pVSEb;
    CImageSelRectEx::Vertex        *m_pVSS2b;
    CImageSelRectEx::Vertex        *m_pVSE2b;


// Operations
public:
    int getScale() { return m_nScale; }
    void setScale(int nScale);
    bool setImageFile(cstr_t szImage);
    void initAsRectImg(CRect &rcObj);
    void initAsHorzStretchImg(CRect &rcObj, int nStretchStartX, int nStretchEndX);
    void initAsVertStretchImg(CRect &rcObj, int nStretchStartY, int nStretchEndY);
    void initAsCaptionImg(CRect &rcObj, int nStretchStartX, int nStretchEndX, int nStretchStartX2, int nStretchEndX2);

    void getRect(CRect &rcObj);
    int getStretchStart();
    int getStretchEnd();
    int getStretchStart2();
    int getStretchEnd2();

    int getImageWidth() { return m_image.m_cx; }
    int getImageHeight() { return m_image.m_cy; }

    void setRectX(int x);
    void setRectY(int y);
    void setRectCx(int cx);
    void setRectCy(int cy);
    void setStretchStart(int nStretchStart);
    void setStretchEnd(int nStretchEnd);
    void setStretchStart2(int nStretchStart2);
    void setStretchEnd2(int nStretchEnd2);

    void getLatestCursorPos(CPoint &pt);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CImageRectSelCtrl)
    protected:
    virtual LRESULT windowProc(uint32_t message, WPARAM wParam, LPARAM lParam);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CImageRectSelCtrl();

    // Generated message map functions
protected:
    //{{AFX_MSG(CImageRectSelCtrl)
    afx_msg void onPaint();
    afx_msg int onCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void onDestroy();
    afx_msg void onLButtonDown(uint32_t nFlags, CPoint point);
    afx_msg void onLButtonUp(uint32_t nFlags, CPoint point);
    afx_msg void onMouseMove(uint32_t nFlags, CPoint point);
    afx_msg void onKeyDown(uint32_t nChar, uint32_t nRepCnt, uint32_t nFlags);
    afx_msg bool OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGERECTSEL_H__488BFFD0_6FE6_4A24_A590_746D2237B79C__INCLUDED_)
