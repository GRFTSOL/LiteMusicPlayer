#pragma once

#include "SkinLinearContainer.h"


class CSkinResizableLinearContainer : public CSkinLinearContainer {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
    CSkinResizableLinearContainer();
    virtual ~CSkinResizableLinearContainer();

    virtual bool onMouseDrag(CPoint point) override;
    virtual bool onMouseMove(CPoint point) override;
    virtual bool onLButtonDown(uint32_t nFlags, CPoint point) override;
    virtual bool onLButtonUp(uint32_t nFlags, CPoint point) override;

    virtual void onCreate() override;
    virtual void onDestroy() override;

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    virtual JsValue getJsObject(VMContext *ctx) override;

    void setMode(cstr_t modeName);
    cstr_t getMode() const;

protected:
    // Return -1, if NOT in resizer area
    int hitTestResizer(CPoint pt);

    void setResizerCursor();

    void dragResizer(int &offset);

    static void zoomFromRight(VecItems &vItems, int nSize);
    static void zoomFromLeft(VecItems &vItems, int nSize);
    static void zoomAll(VecItems &vItems, int nSize);

protected:
    Cursor                      m_cursorWE, m_cursorNS;

    bool                        m_bDragWnd;
    CPoint                      m_ptDragOld;
    int                         m_nCurrentResizerArea; // Current drag resizer area, start from 0.

    int                         m_nThicknessOfResizer;

};
