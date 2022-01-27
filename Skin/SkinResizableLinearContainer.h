#pragma once

class CSkinResizableLinearContainer : public CSkinLinearContainer  
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
    CSkinResizableLinearContainer();
    virtual ~CSkinResizableLinearContainer();

    virtual bool onMouseDrag(CPoint point);
    virtual bool onMouseMove(CPoint point);
    virtual bool onLButtonDown(uint32_t nFlags, CPoint point);
    virtual bool onLButtonUp(uint32_t nFlags, CPoint point);

    virtual void onCreate();
    virtual void onDestroy();
    virtual void onSize();

    bool setProperty(cstr_t szProperty, cstr_t szValue);

protected:
    // Return -1, if NOT in resizer area
    int hitTestResizer(CPoint pt);

    void setResizerCursor();

    void dragResizer(int &offset);

    static void zoomFromRight(VecItems &vItems, int nSize);
    static void zoomFromLeft(VecItems &vItems, int nSize);
    static void zoomAll(VecItems &vItems, int nSize);

protected:
    Cursor                    m_cursorWE, m_cursorNS;

    bool                    m_bDragWnd;
    CPoint                    m_ptDragOld;
    int                        m_nCurrentResizerArea;    // Current drag resizer area, start from 0.

    int                        m_nThicknessOfResizer;

};
