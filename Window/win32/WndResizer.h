/********************************************************************
Created  :    2002/01/04    21:43
FileName :    WndResizer.h
Author   :    xhy

Purpose  :    调整窗口大小的类
*********************************************************************/

#if !defined(AFX_UIWNDSIZING_H__B7400FE0_78AA_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_UIWNDSIZING_H__B7400FE0_78AA_11D5_9E04_02608CAD9330__INCLUDED_

class WndResizer
{
public:
    enum ResizeDirection
    {
        RD_TOP                = 0x1,
        RD_LEFT                = 0x1 << 1,
        RD_RIGHT            = 0x1 << 2,
        RD_BOTTOM            = 0x1 << 3,
    };

    struct ResizeArea
    {
        int            nID;
        uint32_t        resizeDirection;
        CRect        rcResizeArea;
    };

    typedef list<ResizeArea>        ListResizeArea;

public:
    WndResizer();
    virtual ~WndResizer();

    void init(Window *pWnd);

    void onMouseMessage(uint32_t fwKeys, CPoint pt);

    bool isSizing();

    void fixedHeight(bool bFixedHeight) { m_fixedHeight = bFixedHeight; }
    void fixedWidth(bool bFixedWidth) { m_fixedWidth = bFixedWidth; }

    bool isFixedWidth() const { return m_fixedWidth; }
    bool isFixedHeight() const { return m_fixedHeight; }

    bool isResizeAutoCloseTo() const { return m_bResizingAutoCloseto; }
    void setResizeAutoCloseTo(bool bResizeAutoCloseTo) { m_bResizingAutoCloseto = bResizeAutoCloseTo; }

    int getMinCx() { return m_nMincx; }
    int getMinCy() { return m_nMincy; }

    void setMinCx(int nMincx) { m_nMincx = nMincx; }
    void setMinCy(int nMincy) { m_nMincy = nMincy; }

    // If nID == -1, just add it.
    void setResizeArea(int nIDArea, uint32_t resizeDirection, CRect &rcResizeArea);

    void removeResizeArea(int nIDArea);

    void clearResizeArea() { m_listResizeArea.clear(); }

protected:
    void endSizing(uint32_t fwKeys, CPoint &pt);
    void onSizing(uint32_t fwKeys, CPoint &pt);
    void beginSize(uint32_t fwKeys, CPoint &pt);

    virtual void autoCloseToWindows(int &nOffx, int &nOffy);

    uint32_t hitTestResizeArea(CPoint &pt, const CRect &rc);

    void setCursor();

protected:
    ListResizeArea    m_listResizeArea;

    bool            m_fixedWidth, m_fixedHeight;
    bool            m_bInstanceResizing;
    bool            m_bResizingAutoCloseto;        // 是否在调整大小时, 自动靠近指定窗口

    bool            m_bResizing;        // 是否在拖动窗口
    uint32_t            m_ResizeDirection;
    CRect            m_rcResizing;                    // 调整中的矩形的大小

    CPoint            m_ptOld;
    Window        *m_pWnd;                // 窗口

    int                m_nMincx;        // 窗口最小宽度
    int                m_nMincy;        // 窗口最小高度
    Cursor            m_cursorArrow, m_cursorNWSE, m_cursorNESW, m_cursorWE, m_cursorNS;

};

#endif // !defined(AFX_UIWNDSIZING_H__B7400FE0_78AA_11D5_9E04_02608CAD9330__INCLUDED_)
