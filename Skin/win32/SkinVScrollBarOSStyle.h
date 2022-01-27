// SkinVScrollBarOSStyle.h: interface for the CSkinVScrollBarOSStyle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINVSCROLLBAROSSTYLE_H__122A87BD_1DC8_4EA7_A35E_F85FC32AB87B__INCLUDED_)
#define AFX_SKINVSCROLLBAROSSTYLE_H__122A87BD_1DC8_4EA7_A35E_F85FC32AB87B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIObject.h"
#include "../widget/win32/CommonWidget.h"

class CSkinScrollBarOSStyleBase : public CUIObject, public IScrollBar, public IScrollNotify
{
public:
    CSkinScrollBarOSStyleBase();
    virtual ~CSkinScrollBarOSStyleBase();

    void setScrollInfo(int nMin, int nMax, int nPage, int nPos = 0, int nLine = 1, bool bRedraw = true)
    {
        m_scrollbar.setScrollInfo(nMin, nMax, nPage, nPos, nLine, bRedraw);
    }
    int setScrollPos(int nPos, bool bRedraw = true)
    {
        return m_scrollbar.setScrollPos(nPos, bRedraw);
    }

    int getScrollPos() const { return m_scrollbar.getScrollPos(); }
    int getPage() const { return m_scrollbar.getPage(); }
    int getMin() const { return m_scrollbar.getMin(); }
    int getMax() const { return m_scrollbar.getMax(); }
    virtual int getID() const { return m_id; }

    void setScrollNotify(IScrollNotify *pNofity) { m_pScrollNotify = pNofity; }

    virtual bool isEnabled() const { return m_scrollbar.isEnabled(); }
    virtual void disableScrollBar() { m_scrollbar.disableScrollBar(); }
    virtual bool handleScrollCode(uint32_t nSBCode, int nPos) { return m_scrollbar.handleScrollCode(nSBCode, nPos); }

    // IScrollNotify
    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar);
    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar);

    virtual void onSize();
    virtual void onSetFocus();

    void onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt);

protected:
    // 基本设置
    CWidgetScrollBar    m_scrollbar;
    IScrollNotify        *m_pScrollNotify;

};

class CSkinVScrollBarOSStyle : public CSkinScrollBarOSStyleBase
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinVScrollBarOSStyle();

    virtual void onCreate();

};

class CSkinHScrollBarOSStyle : public CSkinScrollBarOSStyleBase
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinHScrollBarOSStyle();

    virtual void onCreate();

};

#endif // !defined(AFX_SKINVSCROLLBAROSSTYLE_H__122A87BD_1DC8_4EA7_A35E_F85FC32AB87B__INCLUDED_)
