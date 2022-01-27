/********************************************************************
    Created  :    2002/01/04    21:43
    FileName :    WndDrag.h
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#if !defined(AFX_WNDDRAG_H__54E078FE_AFB3_4369_8EF6_5E00313640DC__INCLUDED_)
#define AFX_WNDDRAG_H__54E078FE_AFB3_4369_8EF6_5E00313640DC__INCLUDED_

#include <vector>


class Window;

class WndDrag  
{
public:
    typedef struct WndCloseTo
    {
        Window *pWnd;
        string    strClass;
        string    strWndName;
    }WndCloseTo;

    std::vector<WndCloseTo>    m_vWndCloseTo;

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
    Window        *m_pWnd;            // ������

    bool            m_bDragWnd;        // dragging window?

    CPoint            m_ptDragOld;            // drag begin pos

    bool            m_bDragAutoCloseto;

    bool            m_bSticked;        // ճס��Ŀ�괰�ڣ�
    Window        *m_pWndToTrack;    // ճס��Ŀ�괰��
    CPoint            m_ptWndTrack;    // ճס��Ŀ�괰�ڵ����Ͻ�

    bool            m_bEnableDrag;

    // vector<string>        *m_pvWndClassCloseTo;
    // char *          *m_arrszWndClassCloseTo;
};

#endif // !defined(AFX_WNDDRAG_H__54E078FE_AFB3_4369_8EF6_5E00313640DC__INCLUDED_)
